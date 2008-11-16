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
#include <drizzled/definitions.h>

#include <drizzled/hybrid_type_traits_decimal.h>

/* Hybrid_type_traits_decimal */
static const Hybrid_type_traits_decimal decimal_traits_instance;


Item_result Hybrid_type_traits_decimal::type() const { return DECIMAL_RESULT; }


void
Hybrid_type_traits_decimal::fix_length_and_dec(Item *item, Item *arg) const
{
  item->decimals= arg->decimals;
  item->max_length= cmin(arg->max_length + DECIMAL_LONGLONG_DIGITS,
                        (unsigned int)DECIMAL_MAX_STR_LENGTH);
}


void Hybrid_type_traits_decimal::set_zero(Hybrid_type *val) const
{
  my_decimal_set_zero(&val->dec_buf[0]);
  val->used_dec_buf_no= 0;
}


void Hybrid_type_traits_decimal::add(Hybrid_type *val, Field *f) const
{
  my_decimal_add(E_DEC_FATAL_ERROR,
                 &val->dec_buf[val->used_dec_buf_no ^ 1],
                 &val->dec_buf[val->used_dec_buf_no],
                 f->val_decimal(&val->dec_buf[2]));
  val->used_dec_buf_no^= 1;
}


/**
  @todo
  what is '4' for scale?
*/
void Hybrid_type_traits_decimal::div(Hybrid_type *val, uint64_t u) const
{
  int2my_decimal(E_DEC_FATAL_ERROR, u, true, &val->dec_buf[2]);
  /* XXX: what is '4' for scale? */
  my_decimal_div(E_DEC_FATAL_ERROR,
                 &val->dec_buf[val->used_dec_buf_no ^ 1],
                 &val->dec_buf[val->used_dec_buf_no],
                 &val->dec_buf[2], 4);
  val->used_dec_buf_no^= 1;
}


int64_t
Hybrid_type_traits_decimal::val_int(Hybrid_type *val, bool unsigned_flag) const
{
  int64_t result;
  my_decimal2int(E_DEC_FATAL_ERROR, &val->dec_buf[val->used_dec_buf_no],
                 unsigned_flag, &result);
  return result;
}


double
Hybrid_type_traits_decimal::val_real(Hybrid_type *val) const
{
  my_decimal2double(E_DEC_FATAL_ERROR, &val->dec_buf[val->used_dec_buf_no],
                    &val->real);
  return val->real;
}


my_decimal *Hybrid_type_traits_decimal::val_decimal(Hybrid_type *val,
                                                    my_decimal *) const
{ return &val->dec_buf[val->used_dec_buf_no]; }


String *
Hybrid_type_traits_decimal::val_str(Hybrid_type *val, String *to,
                                    uint8_t decimals) const
{
  my_decimal_round(E_DEC_FATAL_ERROR, &val->dec_buf[val->used_dec_buf_no],
                   decimals, false, &val->dec_buf[2]);
  my_decimal2string(E_DEC_FATAL_ERROR, &val->dec_buf[2], 0, 0, 0, to);
  return to;
}


const Hybrid_type_traits_decimal *Hybrid_type_traits_decimal::instance()
{
  return &decimal_traits_instance;
}


Hybrid_type_traits_decimal::Hybrid_type_traits_decimal()
{}