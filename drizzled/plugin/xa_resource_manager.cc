/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2008 Sun Microsystems, Inc.
 *  Copyright (C) 2010 Jay Pipes <jaypipes@gmail.com>
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

#include "config.h"

#include "drizzled/cached_directory.h"

#include <drizzled/definitions.h>
#include <drizzled/session.h>
#include <drizzled/error.h>
#include <drizzled/gettext.h>
#include <drizzled/plugin/xa_resource_manager.h>
#include "drizzled/xid.h"


#include <string>
#include <vector>
#include <algorithm>
#include <functional>

namespace drizzled
{

namespace plugin
{

static std::vector<XaResourceManager *> xa_resource_managers;

int XaResourceManager::commitOrRollbackXID(XID *xid, bool commit)
{
  std::vector<int> results;
  
  if (commit)
    transform(xa_resource_managers.begin(), xa_resource_managers.end(), results.begin(),
              std::bind2nd(std::mem_fun(&XaResourceManager::xaCommitXid), xid));
  else
    transform(xa_resource_managers.begin(), xa_resource_managers.end(), results.begin(),
              std::bind2nd(std::mem_fun(&XaResourceManager::xaRollbackXid), xid));

  if (std::find_if(results.begin(), results.end(), std::bind2nd(std::equal_to<int>(),0)) == results.end())
    return 1;

  return 0;
}

/**
  recover() step of xa.

  @note
    there are three modes of operation:
    - automatic recover after a crash
    in this case commit_list.size() != 0, tc_heuristic_recover==0
    all xids from commit_list are committed, others are rolled back
    - manual (heuristic) recover
    in this case commit_list.size()==0, tc_heuristic_recover != 0
    DBA has explicitly specified that all prepared transactions should
    be committed (or rolled back).
    - no recovery (Drizzle did not detect a crash)
    in this case commit_list.size()==0, tc_heuristic_recover == 0
    there should be no prepared transactions in this case.
*/
class XaRecover : std::unary_function<XaResourceManager *, void>
{
private:
  int trans_len, found_foreign_xids, found_my_xids;
  bool result;
  XID *trans_list;
  const XaResourceManager::commit_list_set &commit_list;
  bool dry_run;
public:
  XaRecover(XID *trans_list_arg, int trans_len_arg,
            const XaResourceManager::commit_list_set& commit_list_arg,
            bool dry_run_arg) 
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

  result_type operator() (argument_type resource_manager)
  {
  
    int got;
  
    while ((got= resource_manager->xaRecover(trans_list, trans_len)) > 0 )
    {
      errmsg_printf(ERRMSG_LVL_INFO,
                    _("Found %d prepared transaction(s) in resource manager."),
                    got);
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
        if (commit_list.size() ?
            commit_list.find(x) != commit_list.end() :
            tc_heuristic_recover == TC_HEURISTIC_RECOVER_COMMIT)
        {
          resource_manager->xaCommitXid(trans_list+i);
        }
        else
        {
          resource_manager->xaRollbackXid(trans_list+i);
        }
      }
      if (got < trans_len)
        break;
    }
  }
};

int XaResourceManager::recoverAllXids()
{
  const XaResourceManager::commit_list_set empty_commit_set;
  return recoverAllXids(empty_commit_set);
}

int XaResourceManager::recoverAllXids(const XaResourceManager::commit_list_set &commit_list)
{
  XID *trans_list= NULL;
  int trans_len= 0;

  bool dry_run= (commit_list.size() == 0 && tc_heuristic_recover==0);

  /* commit_list and tc_heuristic_recover cannot be set both */
  assert(commit_list.size() == 0 || tc_heuristic_recover == 0);

  if (xa_resource_managers.size() <= 1)
    return 0;

  tc_heuristic_recover= TC_HEURISTIC_RECOVER_ROLLBACK; // forcing ROLLBACK
  dry_run=false;
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

  if (commit_list.size())
    errmsg_printf(ERRMSG_LVL_INFO, _("Starting crash recovery..."));

  XaRecover recover_func(trans_list, trans_len, commit_list, dry_run);
  std::for_each(xa_resource_managers.begin(),
                xa_resource_managers.end(),
                recover_func);
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
  if (commit_list.size())
    errmsg_printf(ERRMSG_LVL_INFO, _("Crash recovery finished."));
  return(0);
}

bool XaResourceManager::addPlugin(XaResourceManager *resource_manager)
{
  xa_resource_managers.push_back(resource_manager);
  return false;
}

void XaResourceManager::removePlugin(XaResourceManager *)
{
  xa_resource_managers.clear();
}

} /* namespace plugin */
} /* namespace drizzled */
