/* - mode: c++ c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2008 MySQL
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

#ifndef DRIZZLE_SERVER_FIELD_NEW_DECIMAL
#define DRIZZLE_SERVER_FIELD_NEW_DECIMAL

#include <drizzled/field/num.h>

/* New decimal/numeric field which use fixed point arithmetic */
class Field_new_decimal :public Field_num {
private:
  int do_save_field_metadata(unsigned char *first_byte);
public:
  /* The maximum number of decimal digits can be stored */
  uint32_t precision;
  uint32_t bin_size;
  /*
    Constructors take max_length of the field as a parameter - not the
    precision as the number of decimal digits allowed.
    So for example we need to count length from precision handling
    CREATE TABLE ( DECIMAL(x,y))
  */
  Field_new_decimal(unsigned char *ptr_arg, uint32_t len_arg, unsigned char *null_ptr_arg,
                    unsigned char null_bit_arg,
                    enum utype unireg_check_arg, const char *field_name_arg,
                    uint8_t dec_arg, bool zero_arg, bool unsigned_arg);
  Field_new_decimal(uint32_t len_arg, bool maybe_null_arg,
                    const char *field_name_arg, uint8_t dec_arg,
                    bool unsigned_arg);
  enum_field_types type() const { return DRIZZLE_TYPE_NEWDECIMAL;}
  enum ha_base_keytype key_type() const { return HA_KEYTYPE_BINARY; }
  Item_result result_type () const { return DECIMAL_RESULT; }
  int  reset(void);
  bool store_value(const my_decimal *decimal_value);
  void set_value_on_overflow(my_decimal *decimal_value, bool sign);
  int  store(const char *to, uint32_t length, const CHARSET_INFO * const charset);
  int  store(double nr);
  int  store(int64_t nr, bool unsigned_val);
  int store_time(DRIZZLE_TIME *ltime, enum enum_drizzle_timestamp_type t_type);
  int  store_decimal(const my_decimal *);
  double val_real(void);
  int64_t val_int(void);
  my_decimal *val_decimal(my_decimal *);
  String *val_str(String*, String *);
  int cmp(const unsigned char *, const unsigned char *);
  void sort_string(unsigned char *buff, uint32_t length);
  bool zero_pack() const { return 0; }
  void sql_type(String &str) const;
  uint32_t max_display_length() { return field_length; }
  uint32_t size_of() const { return sizeof(*this); }
  uint32_t pack_length() const { return (uint32_t) bin_size; }
  uint32_t pack_length_from_metadata(uint32_t field_metadata);
  uint32_t row_pack_length() { return pack_length(); }
  int compatible_field_size(uint32_t field_metadata);
  uint32_t is_equal(Create_field *new_field);
  virtual const unsigned char *unpack(unsigned char* to, const unsigned char *from,
                              uint32_t param_data, bool low_byte_first);
};

#endif

