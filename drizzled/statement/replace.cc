/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2009 Sun Microsystems, Inc.
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

#include <config.h>
#include <drizzled/show.h>
#include <drizzled/lock.h>
#include <drizzled/session.h>
#include <drizzled/statement/replace.h>

namespace drizzled
{

bool statement::Replace::execute()
{
  TableList *first_table= (TableList *) getSession()->getLex()->select_lex.table_list.first;
  TableList *all_tables= getSession()->getLex()->query_tables;
  assert(first_table == all_tables && first_table != 0);

  if (insert_precheck(getSession(), all_tables))
  {
    return true;
  }

  if (getSession()->wait_if_global_read_lock(false, true))
  {
    return true;
  }

  bool res= insert_query(getSession(), 
                         all_tables, 
                         getSession()->getLex()->field_list, 
                         getSession()->getLex()->many_values,
                         getSession()->getLex()->update_list, 
                         getSession()->getLex()->value_list,
                         getSession()->getLex()->duplicates, 
                         getSession()->getLex()->ignore);
  /*
     Release the protection against the global read lock and wake
     everyone, who might want to set a global read lock.
   */
  getSession()->startWaitingGlobalReadLock();

  return res;
}

} /* namespace drizzled */

