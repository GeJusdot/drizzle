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

#ifndef DRIZZLED_FUNCTIONS_MINUS_H
#define DRIZZLED_FUNCTIONS_MINUS_H

#include <drizzled/functions/func.h> 
#include <drizzled/functions/num_op.h> 

class Item_func_minus :public Item_func_additive_op
{
public:
  Item_func_minus(Item *a,Item *b) :Item_func_additive_op(a,b) {}
  const char *func_name() const { return "-"; }
  int64_t int_op();
  double real_op();
  my_decimal *decimal_op(my_decimal *);
  void fix_length_and_dec();
};

#endif /* DRIZZLED_FUNCTIONS_MINUS_H */