/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2009 Sun Microsystems
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "drizzled/global.h"

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "mysys/my_dir.h"
#include "mysys/hash.h"

#include "drizzled/service/storage_engine.h"
#include "drizzled/plugin/storage_engine.h"
#include "drizzled/gettext.h"
#include "drizzled/xid.h"
#include "drizzled/errmsg_print.h"
#include "drizzled/plugin/registry.h"
#include "drizzled/table_proto.h"
#include "drizzled/session.h"

using namespace std;

namespace drizzled
{
namespace service
{

StorageEngine::StorageEngine() : all_engines() {}
StorageEngine::~StorageEngine() {}

void StorageEngine::add(plugin::StorageEngine *engine)
{
  all_engines.add(engine);
}

void StorageEngine::remove(plugin::StorageEngine *engine)
{
  all_engines.remove(engine);
}

plugin::StorageEngine *StorageEngine::findByName(Session *session,
                                                 string find_str)
{
  
  transform(find_str.begin(), find_str.end(),
            find_str.begin(), ::tolower);
  string default_str("default");
  if (find_str == default_str)
    return ha_default_storage_engine(session);

  plugin::StorageEngine *engine= all_engines.find(find_str);

  if (engine && engine->is_user_selectable())
    return engine;

  return NULL;
}

class StorageEngineCloseConnection
  : public unary_function<plugin::StorageEngine *, void>
{
  Session *session;
public:
  StorageEngineCloseConnection(Session *session_arg) : session(session_arg) {}
  /*
    there's no need to rollback here as all transactions must
    be rolled back already
  */
  inline result_type operator() (argument_type engine)
  {
    if (engine->is_enabled() && 
      session_get_ha_data(session, engine))
    engine->close_connection(session);
  }
};

/**
  @note
    don't bother to rollback here, it's done already
*/
void StorageEngine::closeConnection(Session* session)
{
  for_each(all_engines.begin(), all_engines.end(),
           StorageEngineCloseConnection(session));
}

void StorageEngine::dropDatabase(char* path)
{
  for_each(all_engines.begin(), all_engines.end(),
           bind2nd(mem_fun(&plugin::StorageEngine::drop_database),path));
}

int StorageEngine::commitOrRollbackByXID(XID *xid, bool commit)
{
  vector<int> results;
  
  if (commit)
    transform(all_engines.begin(), all_engines.end(), results.begin(),
              bind2nd(mem_fun(&plugin::StorageEngine::commit_by_xid),xid));
  else
    transform(all_engines.begin(), all_engines.end(), results.begin(),
              bind2nd(mem_fun(&plugin::StorageEngine::rollback_by_xid),xid));

  if (find_if(results.begin(), results.end(), bind2nd(equal_to<int>(),0))
         == results.end())
    return 1;
  return 0;
}

/**
  @details
  This function should be called when MySQL sends rows of a SELECT result set
  or the EOF mark to the client. It releases a possible adaptive hash index
  S-latch held by session in InnoDB and also releases a possible InnoDB query
  FIFO ticket to enter InnoDB. To save CPU time, InnoDB allows a session to
  keep them over several calls of the InnoDB handler interface when a join
  is executed. But when we let the control to pass to the client they have
  to be released because if the application program uses mysql_use_result(),
  it may deadlock on the S-latch if the application on another connection
  performs another SQL query. In MySQL-4.1 this is even more important because
  there a connection can have several SELECT queries open at the same time.

  @param session           the thread handle of the current connection

  @return
    always 0
*/
int StorageEngine::releaseTemporaryLatches(Session *session)
{
  for_each(all_engines.begin(), all_engines.end(),
           bind2nd(mem_fun(&plugin::StorageEngine::release_temporary_latches),session));
  return 0;
}

bool StorageEngine::flushLogs(plugin::StorageEngine *engine)
{
  if (engine == NULL)
  {
    if (find_if(all_engines.begin(), all_engines.end(),
            mem_fun(&plugin::StorageEngine::flush_logs))
          != all_engines.begin())
      return true;
  }
  else
  {
    if ((!engine->is_enabled()) ||
        (engine->flush_logs()))
      return true;
  }
  return false;
}

/**
  recover() step of xa.

  @note
    there are three modes of operation:
    - automatic recover after a crash
    in this case commit_list != 0, tc_heuristic_recover==0
    all xids from commit_list are committed, others are rolled back
    - manual (heuristic) recover
    in this case commit_list==0, tc_heuristic_recover != 0
    DBA has explicitly specified that all prepared transactions should
    be committed (or rolled back).
    - no recovery (MySQL did not detect a crash)
    in this case commit_list==0, tc_heuristic_recover == 0
    there should be no prepared transactions in this case.
*/
class XARecover : unary_function<plugin::StorageEngine *, void>
{
  int trans_len, found_foreign_xids, found_my_xids;
  bool result;
  XID *trans_list;
  HASH *commit_list;
  bool dry_run;
public:
  XARecover(XID *trans_list_arg, int trans_len_arg,
            HASH *commit_list_arg, bool dry_run_arg) 
    : trans_len(trans_len_arg), found_foreign_xids(0), found_my_xids(0),
      result(false),
      trans_list(trans_list_arg), commit_list(commit_list_arg),
      dry_run(dry_run_arg)
  {}
  
  int getForeignXIDs()
  {
    return found_foreign_xids; 
  }

  int getMyXIDs()
  {
    return found_my_xids; 
  }

  result_type operator() (argument_type engine)
  {
  
    int got;
  
    if (engine->is_enabled())
    {
      while ((got= engine->recover(trans_list, trans_len)) > 0 )
      {
        errmsg_printf(ERRMSG_LVL_INFO,
                      _("Found %d prepared transaction(s) in %s"),
                      got, engine->getName().c_str());
        for (int i=0; i < got; i ++)
        {
          my_xid x=trans_list[i].get_my_xid();
          if (!x) // not "mine" - that is generated by external TM
          {
            xid_cache_insert(trans_list+i, XA_PREPARED);
            found_foreign_xids++;
            continue;
          }
          if (dry_run)
          {
            found_my_xids++;
            continue;
          }
          // recovery mode
          if (commit_list ?
              hash_search(commit_list, (unsigned char *)&x, sizeof(x)) != 0 :
              tc_heuristic_recover == TC_HEURISTIC_RECOVER_COMMIT)
          {
            engine->commit_by_xid(trans_list+i);
          }
          else
          {
            engine->rollback_by_xid(trans_list+i);
          }
        }
        if (got < trans_len)
          break;
      }
    }
  }
};

int StorageEngine::recover(HASH *commit_list)
{
  XID *trans_list= NULL;
  int trans_len= 0;

  bool dry_run= (commit_list==0 && tc_heuristic_recover==0);

  /* commit_list and tc_heuristic_recover cannot be set both */
  assert(commit_list==0 || tc_heuristic_recover==0);

  /* if either is set, total_ha_2pc must be set too */
  if (total_ha_2pc <= 1)
    return 0;


#ifndef WILL_BE_DELETED_LATER

  /*
    for now, only InnoDB supports 2pc. It means we can always safely
    rollback all pending transactions, without risking inconsistent data
  */

  assert(total_ha_2pc == 2); // only InnoDB and binlog
  tc_heuristic_recover= TC_HEURISTIC_RECOVER_ROLLBACK; // forcing ROLLBACK
  dry_run=false;
#endif
  for (trans_len= MAX_XID_LIST_SIZE ;
       trans_list==0 && trans_len > MIN_XID_LIST_SIZE; trans_len/=2)
  {
    trans_list=(XID *)malloc(trans_len*sizeof(XID));
  }
  if (!trans_list)
  {
    errmsg_printf(ERRMSG_LVL_ERROR, ER(ER_OUTOFMEMORY), trans_len*sizeof(XID));
    return(1);
  }

  if (commit_list)
    errmsg_printf(ERRMSG_LVL_INFO, _("Starting crash recovery..."));


  XARecover recover_func(trans_list, trans_len, commit_list, dry_run);
  for_each(all_engines.begin(), all_engines.end(), recover_func);
  free(trans_list);
 
  if (recover_func.getForeignXIDs())
    errmsg_printf(ERRMSG_LVL_WARN,
                  _("Found %d prepared XA transactions"),
                  recover_func.getForeignXIDs());
  if (dry_run && recover_func.getMyXIDs())
  {
    errmsg_printf(ERRMSG_LVL_ERROR,
                  _("Found %d prepared transactions! It means that drizzled "
                    "was not shut down properly last time and critical "
                    "recovery information (last binlog or %s file) was "
                    "manually deleted after a crash. You have to start "
                    "drizzled with the --tc-heuristic-recover switch to "
                    "commit or rollback pending transactions."),
                    recover_func.getMyXIDs(), opt_tc_log_file);
    return(1);
  }
  if (commit_list)
    errmsg_printf(ERRMSG_LVL_INFO, _("Crash recovery finished."));
  return(0);
}

int StorageEngine::startConsistentSnapshot(Session *session)
{
  for_each(all_engines.begin(), all_engines.end(),
           bind2nd(mem_fun(&plugin::StorageEngine::start_consistent_snapshot),
                   session));
  return 0;
}

class StorageEngineGetTableProto: public unary_function<plugin::StorageEngine *,bool>
{
  const char* path;
  message::Table *table_proto;
  int *err;
public:
  StorageEngineGetTableProto(const char* path_arg,
                             message::Table *table_proto_arg,
                             int *err_arg)
  :path(path_arg), table_proto(table_proto_arg), err(err_arg) {}

  result_type operator() (argument_type engine)
  {
    int ret= engine->getTableProtoImplementation(path, table_proto);

    if (ret != ENOENT)
      *err= ret;

    return *err == EEXIST;
  }
};

static int drizzle_read_table_proto(const char* path, message::Table* table)
{
  int fd= open(path, O_RDONLY);

  if (fd == -1)
    return errno;

  google::protobuf::io::ZeroCopyInputStream* input=
    new google::protobuf::io::FileInputStream(fd);

  if (table->ParseFromZeroCopyStream(input) == false)
  {
    delete input;
    close(fd);
    return -1;
  }

  delete input;
  close(fd);
  return 0;
}

/**
  Call this function in order to give the handler the possiblity
  to ask engine if there are any new tables that should be written to disk
  or any dropped tables that need to be removed from disk
*/
int StorageEngine::getTableProto(const char* path,
                                 message::Table *table_proto)
{
  int err= ENOENT;

  Registry<plugin::StorageEngine *>::iterator iter=
    find_if(all_engines.begin(), all_engines.end(),
            StorageEngineGetTableProto(path, table_proto, &err));
  if (iter == all_engines.end())
  {
    string proto_path(path);
    string file_ext(".dfe");
    proto_path.append(file_ext);

    int error= access(proto_path.c_str(), F_OK);

    if (error == 0)
      err= EEXIST;
    else
      err= errno;

    if (table_proto)
    {
      int read_proto_err= drizzle_read_table_proto(proto_path.c_str(),
                                                   table_proto);

      if (read_proto_err)
        err= read_proto_err;
    }
  }

  return err;
}

/**
  An interceptor to hijack the text of the error message without
  setting an error in the thread. We need the text to present it
  in the form of a warning to the user.
*/

class Ha_delete_table_error_handler: public Internal_error_handler
{
public:
  Ha_delete_table_error_handler() : Internal_error_handler() {}
  virtual bool handle_error(uint32_t sql_errno,
                            const char *message,
                            DRIZZLE_ERROR::enum_warning_level level,
                            Session *session);
  char buff[DRIZZLE_ERRMSG_SIZE];
};


bool
Ha_delete_table_error_handler::
handle_error(uint32_t ,
             const char *message,
             DRIZZLE_ERROR::enum_warning_level ,
             Session *)
{
  /* Grab the error message */
  strncpy(buff, message, sizeof(buff)-1);
  return true;
}


class DeleteTableStorageEngine
  : public unary_function<plugin::StorageEngine *, void>
{
  Session *session;
  const char *path;
  handler **file;
  int *dt_error;
public:
  DeleteTableStorageEngine(Session *session_arg, const char *path_arg,
                           handler **file_arg, int *error_arg)
    : session(session_arg), path(path_arg), file(file_arg), dt_error(error_arg) {}

  result_type operator() (argument_type engine)
  {
    char tmp_path[FN_REFLEN];
    handler *tmp_file;

    if(*dt_error!=ENOENT) /* already deleted table */
      return;

    if (!engine)
      return;

    if (!engine->is_enabled())
      return;

    if ((tmp_file= engine->create(NULL, session->mem_root)))
      tmp_file->init();
    else
      return;

    path= engine->checkLowercaseNames(path, tmp_path);
    const string table_path(path);
    int tmp_error= engine->deleteTable(session, table_path);

    if (tmp_error != ENOENT)
    {
      if (tmp_error == 0)
      {
        if (engine->check_flag(HTON_BIT_HAS_DATA_DICTIONARY))
          delete_table_proto_file(path);
        else
          tmp_error= delete_table_proto_file(path);
      }

      *dt_error= tmp_error;
      if(*file)
        delete *file;
      *file= tmp_file;
      return;
    }
    else
      delete tmp_file;

    return;
  }
};


/**
  This should return ENOENT if the file doesn't exists.
  The .frm file will be deleted only if we return 0 or ENOENT
*/
int StorageEngine::deleteTable(Session *session, const char *path,
                               const char *db, const char *alias,
                               bool generate_warning)
{
  TableShare dummy_share;
  Table dummy_table;
  memset(&dummy_table, 0, sizeof(dummy_table));
  memset(&dummy_share, 0, sizeof(dummy_share));

  dummy_table.s= &dummy_share;

  int error= ENOENT;
  handler *file= NULL;

  for_each(all_engines.begin(), all_engines.end(),
           DeleteTableStorageEngine(session, path, &file, &error));

  if (error == ENOENT) /* proto may be left behind */
    error= delete_table_proto_file(path);

  if (error && generate_warning)
  {
    /*
      Because file->print_error() use my_error() to generate the error message
      we use an internal error handler to intercept it and store the text
      in a temporary buffer. Later the message will be presented to user
      as a warning.
    */
    Ha_delete_table_error_handler ha_delete_table_error_handler;

    /* Fill up strucutures that print_error may need */
    dummy_share.path.str= (char*) path;
    dummy_share.path.length= strlen(path);
    dummy_share.db.str= (char*) db;
    dummy_share.db.length= strlen(db);
    dummy_share.table_name.str= (char*) alias;
    dummy_share.table_name.length= strlen(alias);
    dummy_table.alias= alias;

    if(file != NULL)
    {
      file->change_table_ptr(&dummy_table, &dummy_share);

      session->push_internal_handler(&ha_delete_table_error_handler);
      file->print_error(error, 0);

      session->pop_internal_handler();
    }
    else
      error= -1; /* General form of fail. maybe bad FRM */

    /*
      XXX: should we convert *all* errors to warnings here?
      What if the error is fatal?
    */
    push_warning(session, DRIZZLE_ERROR::WARN_LEVEL_ERROR, error,
                 ha_delete_table_error_handler.buff);
  }

  if(file)
    delete file;

  return error;
}

class DFETableNameIterator: public plugin::TableNameIteratorImplementation
{
private:
  MY_DIR *dirp;
  uint32_t current_entry;

public:
  DFETableNameIterator(const string &database)
  : plugin::TableNameIteratorImplementation(database),
    dirp(NULL),
    current_entry(-1)
    {};

  ~DFETableNameIterator();

  int next(string *name);

};

DFETableNameIterator::~DFETableNameIterator()
{
  if (dirp)
    my_dirend(dirp);
}

int DFETableNameIterator::next(string *name)
{
  char uname[NAME_LEN + 1];
  FILEINFO *file;
  char *ext;
  uint32_t file_name_len;
  const char *wild= NULL;

  if (dirp == NULL)
  {
    bool dir= false;
    char path[FN_REFLEN];

    build_table_filename(path, sizeof(path), db.c_str(), "", false);

    dirp = my_dir(path,MYF(dir ? MY_WANT_STAT : 0));

    if (dirp == NULL)
    {
      if (my_errno == ENOENT)
        my_error(ER_BAD_DB_ERROR, MYF(ME_BELL+ME_WAITTANG), db.c_str());
      else
        my_error(ER_CANT_READ_DIR, MYF(ME_BELL+ME_WAITTANG), path, my_errno);
      return(ENOENT);
    }
    current_entry= -1;
  }

  while(true)
  {
    current_entry++;

    if (current_entry == dirp->number_off_files)
    {
      my_dirend(dirp);
      dirp= NULL;
      return -1;
    }

    file= dirp->dir_entry + current_entry;

    if (my_strcasecmp(system_charset_info, ext=fn_rext(file->name),".dfe") ||
        is_prefix(file->name, TMP_FILE_PREFIX))
      continue;
    *ext=0;

    file_name_len= filename_to_tablename(file->name, uname, sizeof(uname));

    uname[file_name_len]= '\0';

    if (wild && wild_compare(uname, wild, 0))
      continue;

    if (name)
      name->assign(uname);

    return 0;
  }
}


TableNameIterator::TableNameIterator(const string &db)
  : current_implementation(NULL), database(db)
{
  plugin::Registry &plugins= plugin::Registry::singleton();
  engine_iter= plugins.storage_engine.begin();
  default_implementation= new DFETableNameIterator(database);
}

TableNameIterator::~TableNameIterator()
{
  delete current_implementation;
}

int TableNameIterator::next(string *name)
{
  plugin::Registry &plugins= plugin::Registry::singleton();
  int err= 0;

next:
  if (current_implementation == NULL)
  {
    while(current_implementation == NULL &&
          (engine_iter != plugins.storage_engine.end()))
    {
      plugin::StorageEngine *engine= *engine_iter;
      current_implementation= engine->tableNameIterator(database);
      engine_iter++;
    }

    if (current_implementation == NULL &&
        (engine_iter == plugins.storage_engine.end()))
    {
      current_implementation= default_implementation;
    }
  }

  err= current_implementation->next(name);

  if (err == -1)
  {
    if (current_implementation != default_implementation)
    {
      delete current_implementation;
      current_implementation= NULL;
      goto next;
    }
  }

  return err;
}


} /* namespace service */
} /* namespace drizzled */



drizzled::plugin::StorageEngine *ha_resolve_by_name(Session *session,
                                                    const string &find_str)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton(); 
  return plugins.storage_engine.findByName(session, find_str);
}

void ha_close_connection(Session *session)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton(); 
  plugins.storage_engine.closeConnection(session);
}

void ha_drop_database(char* path)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton(); 
  plugins.storage_engine.dropDatabase(path);
}

int ha_commit_or_rollback_by_xid(XID *xid, bool commit)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton();
  return plugins.storage_engine.commitOrRollbackByXID(xid, commit);
}

int ha_release_temporary_latches(Session *session)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton();
  return plugins.storage_engine.releaseTemporaryLatches(session);
}

bool ha_flush_logs(drizzled::plugin::StorageEngine *engine)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton();
  return plugins.storage_engine.flushLogs(engine);
}

int ha_recover(HASH *commit_list)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton();
  return plugins.storage_engine.recover(commit_list);
}

int ha_start_consistent_snapshot(Session *session)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton();
  return plugins.storage_engine.startConsistentSnapshot(session);
}

int ha_delete_table(Session *session, const char *path,
                    const char *db, const char *alias, bool generate_warning)
{
  drizzled::plugin::Registry &plugins= drizzled::plugin::Registry::singleton();
  return plugins.storage_engine.deleteTable(session, path, db,
                                            alias, generate_warning);
}

