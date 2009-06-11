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

#ifndef DRIZZLED_ITEM_SET_H
#define DRIZZLED_ITEM_SET_H

#include <drizzled/function/func.h>
#include <drizzled/function/set_user_var.h>
#include <drizzled/item/string.h>

#include <string>

/* Classes to support the SET command */


/****************************************************************************
  Variables that are changable runtime are declared using the
  following classes
****************************************************************************/

class sys_var;
class set_var;
class sys_var_pluginvar; /* opaque */
class Time_zone;
typedef struct system_variables SV;
typedef struct my_locale_st MY_LOCALE;

extern TYPELIB bool_typelib, delay_key_write_typelib, sql_mode_typelib,
       optimizer_switch_typelib, slave_exec_mode_typelib;

typedef int (*sys_check_func)(Session *,  set_var *);
typedef bool (*sys_update_func)(Session *, set_var *);
typedef void (*sys_after_update_func)(Session *,enum_var_type);
typedef void (*sys_set_default_func)(Session *, enum_var_type);
typedef unsigned char *(*sys_value_ptr_func)(Session *session);

static const std::vector<std::string> empty_aliases;


struct sys_var_chain
{
  sys_var *first;
  sys_var *last;
};

class sys_var
{
public:
  sys_var *next;
  struct my_option *option_limits;	/* Updated by by set_var_init() */
  uint32_t name_length;			/* Updated by by set_var_init() */
  const std::string name;

  sys_after_update_func after_update;
  sys_var(const char *name_arg, sys_after_update_func func= NULL)
    :name(name_arg), after_update(func),
    m_allow_empty_value(true)
  {}
  virtual ~sys_var() {}
  void chain_sys_var(sys_var_chain *chain_arg)
  {
    if (chain_arg->last)
      chain_arg->last->next= this;
    else
      chain_arg->first= this;
    chain_arg->last= this;
  }

/* So that we can exist in a Registry. We really need to formalize that */
  std::string getName() const { return name; }
  const std::vector<std::string>& getAliases() const { return empty_aliases; }

  virtual bool check(Session *session, set_var *var);
  bool check_enum(Session *session, set_var *var, const TYPELIB *enum_names);
  bool check_set(Session *session, set_var *var, TYPELIB *enum_names);
  virtual bool update(Session *session, set_var *var)=0;
  virtual void set_default(Session *, enum_var_type)
  {}
  virtual SHOW_TYPE show_type() { return SHOW_UNDEF; }
  virtual unsigned char *value_ptr(Session *, enum_var_type, const LEX_STRING *)
  { return 0; }
  virtual bool check_type(enum_var_type type)
  { return type != OPT_GLOBAL; }		/* Error if not GLOBAL */
  virtual bool check_update_type(Item_result type)
  { return type != INT_RESULT; }		/* Assume INT */
  virtual bool check_default(enum_var_type)
  { return option_limits == 0; }
  Item *item(Session *session, enum_var_type type, const LEX_STRING *base);
  virtual bool is_struct() { return 0; }
  virtual bool is_readonly() const { return 0; }
  virtual sys_var_pluginvar *cast_pluginvar() { return 0; }

protected:
  void set_allow_empty_value(bool allow_empty_value)
  {
    m_allow_empty_value= allow_empty_value;
  }

private:
  bool m_allow_empty_value;
};


/*
  A base class for all variables that require its access to
  be guarded with a mutex.
*/

class sys_var_global: public sys_var
{
protected:
  pthread_mutex_t *guard;
public:
  sys_var_global(const char *name_arg, sys_after_update_func after_update_arg,
                 pthread_mutex_t *guard_arg)
    :sys_var(name_arg, after_update_arg), guard(guard_arg) {}
};


class sys_var_uint32_t_ptr :public sys_var
{
  uint32_t *value;
public:
  sys_var_uint32_t_ptr(sys_var_chain *chain, const char *name_arg,
                       uint32_t *value_ptr_arg)
    :sys_var(name_arg),value(value_ptr_arg)
  { chain_sys_var(chain); }
  sys_var_uint32_t_ptr(sys_var_chain *chain, const char *name_arg,
                       uint32_t *value_ptr_arg,
                       sys_after_update_func func)
    :sys_var(name_arg,func), value(value_ptr_arg)
  { chain_sys_var(chain); }
  bool check(Session *session, set_var *var);
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_INT; }
  unsigned char *value_ptr(Session *, enum_var_type, const LEX_STRING *)
  { return (unsigned char*) value; }
};


class sys_var_uint64_t_ptr :public sys_var
{
  uint64_t *value;
public:
  sys_var_uint64_t_ptr(sys_var_chain *chain, const char *name_arg, uint64_t *value_ptr_arg)
    :sys_var(name_arg),value(value_ptr_arg)
  { chain_sys_var(chain); }
  sys_var_uint64_t_ptr(sys_var_chain *chain, const char *name_arg, uint64_t *value_ptr_arg,
		       sys_after_update_func func)
    :sys_var(name_arg,func), value(value_ptr_arg)
  { chain_sys_var(chain); }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_LONGLONG; }
  unsigned char *value_ptr(Session *, enum_var_type,
                           const LEX_STRING *)
  { return (unsigned char*) value; }
};

class sys_var_size_t_ptr :public sys_var
{
  size_t *value;
public:
  sys_var_size_t_ptr(sys_var_chain *chain, const char *name_arg, size_t *value_ptr_arg)
    :sys_var(name_arg),value(value_ptr_arg)
  { chain_sys_var(chain); }
  sys_var_size_t_ptr(sys_var_chain *chain, const char *name_arg, size_t *value_ptr_arg,
                     sys_after_update_func func)
    :sys_var(name_arg,func), value(value_ptr_arg)
  { chain_sys_var(chain); }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_SIZE; }
  unsigned char *value_ptr(Session *, enum_var_type, const LEX_STRING *)
  { return (unsigned char*) value; }
};

class sys_var_bool_ptr :public sys_var
{
public:
  bool *value;
  sys_var_bool_ptr(sys_var_chain *chain, const char *name_arg, bool *value_arg)
    :sys_var(name_arg),value(value_arg)
  { chain_sys_var(chain); }
  bool check(Session *session, set_var *var)
  {
    return check_enum(session, var, &bool_typelib);
  }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_MY_BOOL; }
  unsigned char *value_ptr(Session *, enum_var_type, const LEX_STRING *)
  { return (unsigned char*) value; }
  bool check_update_type(Item_result)
  { return 0; }
};

class sys_var_bool_ptr_readonly :public sys_var_bool_ptr
{
public:
  sys_var_bool_ptr_readonly(sys_var_chain *chain, const char *name_arg,
                            bool *value_arg)
    :sys_var_bool_ptr(chain, name_arg, value_arg)
  {}
  bool is_readonly() const { return 1; }
};


class sys_var_str :public sys_var
{
public:
  char *value;					// Pointer to allocated string
  uint32_t value_length;
  sys_check_func check_func;
  sys_update_func update_func;
  sys_set_default_func set_default_func;
  sys_var_str(sys_var_chain *chain, const char *name_arg,
	      sys_check_func check_func_arg,
	      sys_update_func update_func_arg,
	      sys_set_default_func set_default_func_arg,
              char *value_arg)
    :sys_var(name_arg), value(value_arg), check_func(check_func_arg),
    update_func(update_func_arg),set_default_func(set_default_func_arg)
  { chain_sys_var(chain); }
  bool check(Session *session, set_var *var);
  bool update(Session *session, set_var *var)
  {
    return (*update_func)(session, var);
  }
  void set_default(Session *session, enum_var_type type)
  {
    (*set_default_func)(session, type);
  }
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  unsigned char *value_ptr(Session *, enum_var_type, const LEX_STRING *)
  { return (unsigned char*) value; }
  bool check_update_type(Item_result type)
  {
    return type != STRING_RESULT;		/* Only accept strings */
  }
  bool check_default(enum_var_type)
  { return 0; }
};


class sys_var_const_str :public sys_var
{
  char *value;					// Pointer to const value
public:
  sys_var_const_str(sys_var_chain *chain, const char *name_arg,
                    const char *value_arg)
    :sys_var(name_arg), value((char*) value_arg)
  { chain_sys_var(chain); }
  inline void set (char *new_value)
  {
    value= new_value;
  }
  bool check(Session *, set_var *)
  {
    return 1;
  }
  bool update(Session *, set_var *)
  {
    return 1;
  }
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  unsigned char *value_ptr(Session *, enum_var_type, const LEX_STRING *)
  {
    return (unsigned char*) value;
  }
  bool check_update_type(Item_result)
  {
    return 1;
  }
  bool check_default(enum_var_type)
  { return 1; }
  bool is_readonly() const { return 1; }
};


class sys_var_const_str_ptr :public sys_var
{
  char **value;					// Pointer to const value
public:
  sys_var_const_str_ptr(sys_var_chain *chain, const char *name_arg, char **value_arg)
    :sys_var(name_arg),value(value_arg)
  { chain_sys_var(chain); }
  bool check(Session *, set_var *)
  {
    return 1;
  }
  bool update(Session *, set_var *)
  {
    return 1;
  }
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  unsigned char *value_ptr(Session *, enum_var_type, const LEX_STRING *)
  {
    return (unsigned char*) *value;
  }
  bool check_update_type(Item_result)
  {
    return 1;
  }
  bool check_default(enum_var_type)
  { return 1; }
  bool is_readonly(void) const { return 1; }
};


class sys_var_enum :public sys_var
{
  uint32_t *value;
  TYPELIB *enum_names;
public:
  sys_var_enum(sys_var_chain *chain, const char *name_arg, uint32_t *value_arg,
	       TYPELIB *typelib, sys_after_update_func func)
    :sys_var(name_arg,func), value(value_arg), enum_names(typelib)
  { chain_sys_var(chain); }
  bool check(Session *session, set_var *var)
  {
    return check_enum(session, var, enum_names);
  }
  bool update(Session *session, set_var *var);
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  bool check_update_type(Item_result)
  { return 0; }
};


class sys_var_enum_const :public sys_var
{
  uint32_t SV::*offset;
  TYPELIB *enum_names;
public:
  sys_var_enum_const(sys_var_chain *chain, const char *name_arg, uint32_t SV::*offset_arg,
      TYPELIB *typelib, sys_after_update_func func)
    :sys_var(name_arg,func), offset(offset_arg), enum_names(typelib)
  { chain_sys_var(chain); }
  bool check(Session *, set_var *)
  { return 1; }
  bool update(Session *, set_var *)
  { return 1; }
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  bool check_update_type(Item_result)
  { return 1; }
  bool is_readonly() const { return 1; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
};


class sys_var_session :public sys_var
{
public:
  sys_var_session(const char *name_arg,
              sys_after_update_func func= NULL)
    :sys_var(name_arg, func)
  {}
  bool check_type(enum_var_type)
  { return 0; }
  bool check_default(enum_var_type type)
  {
    return type == OPT_GLOBAL && !option_limits;
  }
};

class sys_var_session_uint32_t :public sys_var_session
{
  sys_check_func check_func;
public:
  uint32_t SV::*offset;
  sys_var_session_uint32_t(sys_var_chain *chain, const char *name_arg,
                           uint32_t SV::*offset_arg,
                           sys_check_func c_func= NULL,
                           sys_after_update_func au_func= NULL)
    :sys_var_session(name_arg, au_func), check_func(c_func),
    offset(offset_arg)
  { chain_sys_var(chain); }
  bool check(Session *session, set_var *var);
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_INT; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
};


class sys_var_session_ha_rows :public sys_var_session
{
public:
  ha_rows SV::*offset;
  sys_var_session_ha_rows(sys_var_chain *chain, const char *name_arg,
                      ha_rows SV::*offset_arg)
    :sys_var_session(name_arg), offset(offset_arg)
  { chain_sys_var(chain); }
  sys_var_session_ha_rows(sys_var_chain *chain, const char *name_arg,
                      ha_rows SV::*offset_arg,
		      sys_after_update_func func)
    :sys_var_session(name_arg,func), offset(offset_arg)
  { chain_sys_var(chain); }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_HA_ROWS; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
};


class sys_var_session_uint64_t :public sys_var_session
{
  sys_check_func check_func;
public:
  uint64_t SV::*offset;
  bool only_global;
  sys_var_session_uint64_t(sys_var_chain *chain, 
                           const char *name_arg,
                           uint64_t SV::*offset_arg,
                           sys_after_update_func au_func= NULL,
                           sys_check_func c_func= NULL)
    :sys_var_session(name_arg, au_func),
    check_func(c_func),
    offset(offset_arg)
  { chain_sys_var(chain); }
  sys_var_session_uint64_t(sys_var_chain *chain,
                           const char *name_arg,
                           uint64_t SV::*offset_arg,
                           sys_after_update_func func,
                           bool only_global_arg,
                           sys_check_func cfunc= NULL)
    :sys_var_session(name_arg, func),
    check_func(cfunc),
    offset(offset_arg),
    only_global(only_global_arg)
  { chain_sys_var(chain); }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_LONGLONG; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  bool check(Session *session, set_var *var);
  bool check_default(enum_var_type type)
  {
    return type == OPT_GLOBAL && !option_limits;
  }
  bool check_type(enum_var_type type)
  {
    return (only_global && type != OPT_GLOBAL);
  }
};

class sys_var_session_size_t :public sys_var_session
{
  sys_check_func check_func;
public:
  size_t SV::*offset;
  bool only_global;
  sys_var_session_size_t(sys_var_chain *chain, const char *name_arg,
                         size_t SV::*offset_arg,
                         sys_after_update_func au_func= NULL,
                         sys_check_func c_func= NULL)
    :sys_var_session(name_arg, au_func),
     check_func(c_func),
     offset(offset_arg)
  { chain_sys_var(chain); }
  sys_var_session_size_t(sys_var_chain *chain,
                         const char *name_arg,
                         size_t SV::*offset_arg,
                         sys_after_update_func func,
                         bool only_global_arg,
                         sys_check_func cfunc= NULL)
    :sys_var_session(name_arg, func),
     check_func(cfunc),
     offset(offset_arg),
     only_global(only_global_arg)
  { chain_sys_var(chain); }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_SIZE; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  bool check(Session *session, set_var *var);
  bool check_default(enum_var_type type)
  {
    return type == OPT_GLOBAL && !option_limits;
  }
  bool check_type(enum_var_type type)
  {
    return (only_global && type != OPT_GLOBAL);
  }
};


class sys_var_session_bool :public sys_var_session
{
public:
  bool SV::*offset;
  sys_var_session_bool(sys_var_chain *chain, const char *name_arg, bool SV::*offset_arg)
    :sys_var_session(name_arg), offset(offset_arg)
  { chain_sys_var(chain); }
  sys_var_session_bool(sys_var_chain *chain, const char *name_arg, bool SV::*offset_arg,
		   sys_after_update_func func)
    :sys_var_session(name_arg,func), offset(offset_arg)
  { chain_sys_var(chain); }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_MY_BOOL; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  bool check(Session *session, set_var *var)
  {
    return check_enum(session, var, &bool_typelib);
  }
  bool check_update_type(Item_result)
  { return 0; }
};


class sys_var_session_enum :public sys_var_session
{
protected:
  uint32_t SV::*offset;
  TYPELIB *enum_names;
  sys_check_func check_func;
public:
  sys_var_session_enum(sys_var_chain *chain, const char *name_arg,
                   uint32_t SV::*offset_arg, TYPELIB *typelib,
                   sys_after_update_func func= NULL,
                   sys_check_func check_f= NULL)
    :sys_var_session(name_arg, func), offset(offset_arg),
    enum_names(typelib), check_func(check_f)
  { chain_sys_var(chain); }
  bool check(Session *session, set_var *var)
  {
    int ret= 0;
    if (check_func)
      ret= (*check_func)(session, var);
    return ret ? ret : check_enum(session, var, enum_names);
  }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  bool check_update_type(Item_result)
  { return 0; }
};



class sys_var_session_optimizer_switch :public sys_var_session_enum
{
public:
  sys_var_session_optimizer_switch(sys_var_chain *chain, const char *name_arg,
                                   uint32_t SV::*offset_arg)
    :sys_var_session_enum(chain, name_arg, offset_arg, &optimizer_switch_typelib)
  {}
  bool check(Session *session, set_var *var)
  {
    return check_set(session, var, enum_names);
  }
  void set_default(Session *session, enum_var_type type);
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  static bool symbolic_mode_representation(Session *session, uint32_t sql_mode,
                                           LEX_STRING *rep);
};


class sys_var_session_storage_engine :public sys_var_session
{
protected:
  StorageEngine *SV::*offset;
public:
  sys_var_session_storage_engine(sys_var_chain *chain, const char *name_arg,
                                 StorageEngine *SV::*offset_arg)
    :sys_var_session(name_arg), offset(offset_arg)
  { chain_sys_var(chain); }
  bool check(Session *session, set_var *var);
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  bool check_update_type(Item_result type)
  {
    return type != STRING_RESULT;		/* Only accept strings */
  }
  void set_default(Session *session, enum_var_type type);
  bool update(Session *session, set_var *var);
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
};

class sys_var_session_bit :public sys_var_session
{
  sys_check_func check_func;
  sys_update_func update_func;
public:
  uint64_t bit_flag;
  bool reverse;
  sys_var_session_bit(sys_var_chain *chain, const char *name_arg,
                  sys_check_func c_func, sys_update_func u_func,
                  uint64_t bit, bool reverse_arg=0)
    :sys_var_session(name_arg, NULL), check_func(c_func),
    update_func(u_func), bit_flag(bit), reverse(reverse_arg)
  { chain_sys_var(chain); }
  bool check(Session *session, set_var *var);
  bool update(Session *session, set_var *var);
  bool check_update_type(Item_result)
  { return 0; }
  bool check_type(enum_var_type type) { return type == OPT_GLOBAL; }
  SHOW_TYPE show_type() { return SHOW_MY_BOOL; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
};

/* some variables that require special handling */

class sys_var_timestamp :public sys_var
{
public:
  sys_var_timestamp(sys_var_chain *chain, const char *name_arg)
    :sys_var(name_arg, NULL)
  { chain_sys_var(chain); }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  bool check_type(enum_var_type type)    { return type == OPT_GLOBAL; }
  bool check_default(enum_var_type)
  { return 0; }
  SHOW_TYPE show_type(void) { return SHOW_LONG; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
};


class sys_var_last_insert_id :public sys_var
{
public:
  sys_var_last_insert_id(sys_var_chain *chain, const char *name_arg)
    :sys_var(name_arg, NULL)
  { chain_sys_var(chain); }
  bool update(Session *session, set_var *var);
  bool check_type(enum_var_type type) { return type == OPT_GLOBAL; }
  SHOW_TYPE show_type() { return SHOW_LONGLONG; }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
};


class sys_var_collation :public sys_var_session
{
public:
  sys_var_collation(const char *name_arg)
    :sys_var_session(name_arg, NULL)
  { }
  bool check(Session *session, set_var *var);
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  bool check_update_type(Item_result type)
  {
    return ((type != STRING_RESULT) && (type != INT_RESULT));
  }
  bool check_default(enum_var_type) { return 0; }
  virtual void set_default(Session *session, enum_var_type type)= 0;
};

class sys_var_collation_sv :public sys_var_collation
{
  const CHARSET_INFO *SV::*offset;
  const CHARSET_INFO **global_default;
public:
  sys_var_collation_sv(sys_var_chain *chain, const char *name_arg,
                       const CHARSET_INFO *SV::*offset_arg,
                       const CHARSET_INFO **global_default_arg)
    :sys_var_collation(name_arg),
    offset(offset_arg), global_default(global_default_arg)
  {
    chain_sys_var(chain);
  }
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
};


class sys_var_key_cache_param :public sys_var
{
protected:
  size_t offset;
public:
  sys_var_key_cache_param(sys_var_chain *chain, const char *name_arg,
                          size_t offset_arg)
    :sys_var(name_arg), offset(offset_arg)
  { chain_sys_var(chain); }
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  bool check_default(enum_var_type)
  { return 1; }
  bool is_struct() { return 1; }
};


class sys_var_key_buffer_size :public sys_var_key_cache_param
{
public:
  sys_var_key_buffer_size(sys_var_chain *chain, const char *name_arg)
    :sys_var_key_cache_param(chain, name_arg,
                             offsetof(KEY_CACHE, param_buff_size))
  {}
  bool update(Session *session, set_var *var);
  SHOW_TYPE show_type() { return SHOW_LONGLONG; }
};


class sys_var_key_cache_uint32_t :public sys_var_key_cache_param
{
public:
  sys_var_key_cache_uint32_t(sys_var_chain *chain, const char *name_arg, size_t offset_arg)
    :sys_var_key_cache_param(chain, name_arg, offset_arg)
  {}
  bool update(Session *session, set_var *var);
  SHOW_TYPE show_type() { return SHOW_INT; }
};

/* Variable that you can only read from */

class sys_var_readonly: public sys_var
{
public:
  enum_var_type var_type;
  SHOW_TYPE show_type_value;
  sys_value_ptr_func value_ptr_func;
  sys_var_readonly(sys_var_chain *chain, const char *name_arg, enum_var_type type,
		   SHOW_TYPE show_type_arg,
		   sys_value_ptr_func value_ptr_func_arg)
    :sys_var(name_arg), var_type(type),
       show_type_value(show_type_arg), value_ptr_func(value_ptr_func_arg)
  { chain_sys_var(chain); }
  bool update(Session *, set_var *)
  { return 1; }
  bool check_default(enum_var_type)
  { return 1; }
  bool check_type(enum_var_type type) { return type != var_type; }
  bool check_update_type(Item_result)
  { return 1; }
  unsigned char *value_ptr(Session *session, enum_var_type,
                           const LEX_STRING *)
  {
    return (*value_ptr_func)(session);
  }
  SHOW_TYPE show_type(void) { return show_type_value; }
  bool is_readonly(void) const { return 1; }
};


class sys_var_have_option: public sys_var
{
protected:
  virtual SHOW_COMP_OPTION get_option() = 0;
public:
  sys_var_have_option(sys_var_chain *chain, const char *variable_name):
    sys_var(variable_name)
  { chain_sys_var(chain); }
  unsigned char *value_ptr(Session *, enum_var_type,
                           const LEX_STRING *)
  {
    return (unsigned char*) show_comp_option_name[get_option()];
  }
  bool update(Session *, set_var *) { return 1; }
  bool check_default(enum_var_type)
  { return 1; }
  bool check_type(enum_var_type type) { return type != OPT_GLOBAL; }
  bool check_update_type(Item_result)
  { return 1; }
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  bool is_readonly() const { return 1; }
};


class sys_var_have_variable: public sys_var_have_option
{
  SHOW_COMP_OPTION *have_variable;

public:
  sys_var_have_variable(sys_var_chain *chain, const char *variable_name,
                        SHOW_COMP_OPTION *have_variable_arg):
    sys_var_have_option(chain, variable_name),
    have_variable(have_variable_arg)
  { }
  SHOW_COMP_OPTION get_option() { return *have_variable; }
};


class sys_var_have_plugin: public sys_var_have_option
{
  const char *plugin_name_str;
  const uint32_t plugin_name_len;
  const int plugin_type;

public:
  sys_var_have_plugin(sys_var_chain *chain, const char *variable_name,
                      const char *plugin_name_str_arg, uint32_t plugin_name_len_arg,
                      int plugin_type_arg):
    sys_var_have_option(chain, variable_name),
    plugin_name_str(plugin_name_str_arg), plugin_name_len(plugin_name_len_arg),
    plugin_type(plugin_type_arg)
  { }
  /* the following method is declared in sql_plugin.cc */
  SHOW_COMP_OPTION get_option();
};


class sys_var_session_time_zone :public sys_var_session
{
public:
  sys_var_session_time_zone(sys_var_chain *chain, const char *name_arg)
    :sys_var_session(name_arg, NULL)
  {
    chain_sys_var(chain);
  }
  bool check(Session *session, set_var *var);
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  bool check_update_type(Item_result type)
  {
    return type != STRING_RESULT;		/* Only accept strings */
  }
  bool check_default(enum_var_type)
  { return 0; }
  bool update(Session *session, set_var *var);
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  virtual void set_default(Session *session, enum_var_type type);
};


class sys_var_microseconds :public sys_var_session
{
  uint64_t SV::*offset;
public:
  sys_var_microseconds(sys_var_chain *chain, const char *name_arg,
                       uint64_t SV::*offset_arg):
    sys_var_session(name_arg), offset(offset_arg)
  { chain_sys_var(chain); }
  bool check(Session *, set_var *) {return 0;}
  bool update(Session *session, set_var *var);
  void set_default(Session *session, enum_var_type type);
  SHOW_TYPE show_type() { return SHOW_DOUBLE; }
  bool check_update_type(Item_result type)
  {
    return (type != INT_RESULT && type != REAL_RESULT && type != DECIMAL_RESULT);
  }
};

class sys_var_session_lc_time_names :public sys_var_session
{
public:
  sys_var_session_lc_time_names(sys_var_chain *chain, const char *name_arg)
    : sys_var_session(name_arg, NULL)
  {
    chain_sys_var(chain);
  }
  bool check(Session *session, set_var *var);
  SHOW_TYPE show_type() { return SHOW_CHAR; }
  bool check_update_type(Item_result type)
  {
    return ((type != STRING_RESULT) && (type != INT_RESULT));
  }
  bool check_default(enum_var_type)
  { return 0; }
  bool update(Session *session, set_var *var);
  unsigned char *value_ptr(Session *session, enum_var_type type,
                           const LEX_STRING *base);
  virtual void set_default(Session *session, enum_var_type type);
};


/****************************************************************************
  Classes for parsing of the SET command
****************************************************************************/

class set_var_base :public Sql_alloc
{
public:
  set_var_base() {}
  virtual ~set_var_base() {}
  virtual int check(Session *session)=0;	/* To check privileges etc. */
  virtual int update(Session *session)=0;	/* To set the value */
  /* light check for PS */
};


/* MySQL internal variables */

class set_var :public set_var_base
{
public:
  sys_var *var;
  Item *value;
  enum_var_type type;
  union
  {
    const CHARSET_INFO *charset;
    uint32_t uint32_t_value;
    uint64_t uint64_t_value;
    size_t size_t_value;
    StorageEngine *storage_engine;
    Time_zone *time_zone;
    MY_LOCALE *locale_value;
  } save_result;
  LEX_STRING base;			/* for structs */

  set_var(enum_var_type type_arg, sys_var *var_arg,
          const LEX_STRING *base_name_arg, Item *value_arg)
    :var(var_arg), type(type_arg), base(*base_name_arg)
  {
    /*
      If the set value is a field, change it to a string to allow things like
      SET table_type=MYISAM;
    */
    if (value_arg && value_arg->type() == Item::FIELD_ITEM)
    {
      Item_field *item= (Item_field*) value_arg;
      if (!(value=new Item_string(item->field_name,
                  (uint32_t) strlen(item->field_name),
				  item->collation.collation)))
	value=value_arg;			/* Give error message later */
    }
    else
      value=value_arg;
  }
  int check(Session *session);
  int update(Session *session);
};


/* User variables like @my_own_variable */

class set_var_user: public set_var_base
{
  Item_func_set_user_var *user_var_item;
public:
  set_var_user(Item_func_set_user_var *item)
    :user_var_item(item)
  {}
  int check(Session *session);
  int update(Session *session);
};


extern "C"
{
  typedef int (*process_key_cache_t) (const char *, KEY_CACHE *);
}

/* Named lists (used for keycaches) */

class NAMED_LIST :public ilink
{
  std::string name;
public:
  unsigned char* data;

  NAMED_LIST(I_List<NAMED_LIST> *links, const char *name_arg,
	           uint32_t name_length_arg, unsigned char* data_arg);
  bool cmp(const char *name_cmp, uint32_t length);
  friend bool process_key_caches(process_key_cache_t func);
  friend void delete_elements(I_List<NAMED_LIST> *list,
                              void (*free_element)(const char*,
                                                   unsigned char*));
};

extern sys_var_session_bool sys_old_alter_table;
extern LEX_STRING default_key_cache_base;

/* For sql_yacc */
struct sys_var_with_base
{
  sys_var *var;
  LEX_STRING base_name;
};

/*
  Prototypes for helper functions
*/

int set_var_init();
void set_var_free();
int mysql_append_static_vars(const SHOW_VAR *show_vars, uint32_t count);
SHOW_VAR* enumerate_sys_vars(Session *session, bool sorted);
int mysql_add_sys_var_chain(sys_var *chain, struct my_option *long_options);
int mysql_del_sys_var_chain(sys_var *chain);
sys_var *find_sys_var(Session *session, const char *str, uint32_t length=0);
int sql_set_variables(Session *session, List<set_var_base> *var_list);
bool not_all_support_one_shot(List<set_var_base> *var_list);
void fix_delay_key_write(Session *session, enum_var_type type);
void fix_slave_exec_mode(enum_var_type type);
extern sys_var_session_time_zone sys_time_zone;
extern sys_var_session_bit sys_autocommit;
const CHARSET_INFO *get_old_charset_by_name(const char *old_name);
unsigned char* find_named(I_List<NAMED_LIST> *list, const char *name, uint32_t length,
		NAMED_LIST **found);

extern sys_var_str sys_var_general_log_path, sys_var_slow_log_path;

/* key_cache functions */
KEY_CACHE *get_key_cache(const LEX_STRING *cache_name);
KEY_CACHE *get_or_create_key_cache(const char *name, uint32_t length);
void free_key_cache(const char *name, KEY_CACHE *key_cache);
bool process_key_caches(process_key_cache_t func);
void delete_elements(I_List<NAMED_LIST> *list,
		     void (*free_element)(const char*, unsigned char*));

#endif /* DRIZZLED_ITEM_SET_H */
