/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2008 Sun Microsystems
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

#include <drizzled/server_includes.h>
#include CSTDINT_H
#include <drizzled/function/math/rand.h>
#include <libdrizzleclient/password.h>
#include <drizzled/session.h>

void Item_func_rand::seed_random(Item *arg)
{
  /*
    TODO: do not do reinit 'rand' for every execute of PS/SP if
    args[0] is a constant.
  */
  uint64_t tmp= (uint64_t) arg->val_int();
  drizzleclient_randominit(rand, (tmp*0x10001L+55555555L),
                           (tmp*0x10000001L));
}

bool Item_func_rand::fix_fields(Session *session,Item **ref)
{
  if (Item_real_func::fix_fields(session, ref))
    return true;
  used_tables_cache|= RAND_TABLE_BIT;
  if (arg_count)
  {                                     // Only use argument once in query
    /*
      No need to send a Rand log event if seed was given eg: RAND(seed),
      as it will be replicated in the query as such.
    */
    if (!rand && !(rand= (struct rand_struct*)
                   session->alloc(sizeof(*rand))))
      return true;

    if (args[0]->const_item())
      seed_random (args[0]);
  }
  else
  {
    rand= &session->rand;
  }
  return false;
}

void Item_func_rand::update_used_tables()
{
  Item_real_func::update_used_tables();
  used_tables_cache|= RAND_TABLE_BIT;
}


double Item_func_rand::val_real()
{
  assert(fixed == 1);
  if (arg_count && !args[0]->const_item())
    seed_random (args[0]);
  return drizzleclient_my_rnd(rand);
}
