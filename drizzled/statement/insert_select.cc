/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2009 Sun Microsystems
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <drizzled/server_includes.h>
#include <drizzled/show.h>
#include <drizzled/lock.h>
#include <drizzled/session.h>
#include <drizzled/statement/insert_select.h>

using namespace drizzled;

bool statement::InsertSelect::execute()
{
  TableList *first_table= (TableList *) session->lex->select_lex.table_list.first;
  TableList *all_tables= session->lex->query_tables;
  assert(first_table == all_tables && first_table != 0);
  Select_Lex *select_lex= &session->lex->select_lex;
  Select_Lex_Unit *unit= &session->lex->unit;
  select_result *sel_result= NULL;
  bool res= false;
  bool need_start_waiting= false;

  if (insert_precheck(session, all_tables))
  {
    return true;
  }

  /* Don't unlock tables until command is written to binary log */
  select_lex->options|= SELECT_NO_UNLOCK;

  unit->set_limit(select_lex);

  if (! (need_start_waiting= ! wait_if_global_read_lock(session, 0, 1)))
  {
    return true;
  }

  if (! (res= session->openTablesLock(all_tables)))
  {
    /* Skip first table, which is the table we are inserting in */
    TableList *second_table= first_table->next_local;
    select_lex->table_list.first= (unsigned char*) second_table;
    select_lex->context.table_list=
      select_lex->context.first_name_resolution_table= second_table;
    res= mysql_insert_select_prepare(session);
    if (! res && (sel_result= new select_insert(first_table,
                                                first_table->table,
                                                &session->lex->field_list,
                                                &session->lex->update_list,
                                                &session->lex->value_list,
                                                session->lex->duplicates,
                                                session->lex->ignore)))
    {
      res= handle_select(session, 
                         session->lex, 
                         sel_result, 
                         OPTION_SETUP_TABLES_DONE);
      /*
         Invalidate the table in the query cache if something changed
         after unlocking when changes become visible.
         TODO: this is a workaround. right way will be move invalidating in
         the unlock procedure.
       */
      if (first_table->lock_type == TL_WRITE_CONCURRENT_INSERT &&
          session->lock)
      {
        /* INSERT ... SELECT should invalidate only the very first table */
        TableList *save_table= first_table->next_local;
        first_table->next_local= 0;
        first_table->next_local= save_table;
      }
      delete sel_result;
    }
    /* revert changes for SP */
    select_lex->table_list.first= (unsigned char*) first_table;
  }

  /*
     Release the protection against the global read lock and wake
     everyone, who might want to set a global read lock.
   */
  start_waiting_global_read_lock(session);

  return res;
}