/* - mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2010 Sun Microsystems
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

#include <plugin/data_engine/function.h>
#include <drizzled/charset.h>

using namespace std;
using namespace drizzled;

IndexesTool::IndexesTool() :
  TablesTool("INDEXES")
{
  add_field("TABLE_SCHEMA");
  add_field("TABLE_NAME");
  add_field("INDEX_NAME");
  add_field("IS_PRIMARY", Tool::BOOLEAN);
  add_field("IS_UNIQUE", Tool::BOOLEAN);
  add_field("IS_NULLABLE", Tool::BOOLEAN);
  add_field("KEY_LENGTH", Tool::NUMBER);
  add_field("INDEX_TYPE");
  add_field("INDEX_COMMENT", 1024);
}

IndexesTool::Generator::Generator(Field **arg) :
  TablesTool::Generator(arg),
  index_iterator(0),
  is_index_primed(false)
{
}

bool IndexesTool::Generator::nextIndexCore()
{
  if (isIndexesPrimed())
  {
    index_iterator++;
  }
  else
  {
    if (not isTablesPrimed())
      return false;

    index_iterator= 0;
    is_index_primed= true;
  }

  if (index_iterator >= getTableProto().indexes_size())
    return false;

  index= getTableProto().indexes(index_iterator);

  return true;
}

bool IndexesTool::Generator::nextIndex()
{
  while (not nextIndexCore())
  {
    if (not nextTable())
      return false;
    is_index_primed= false;
  }

  return true;
}

bool IndexesTool::Generator::populate()
{
  if (not nextIndex())
    return false;

  fill();

  return true;
}

void IndexesTool::Generator::fill()
{
  /* TABLE_SCHEMA */
  push(schema_name());

  /* TABLE_NAME */
  push(table_name());

  /* INDEX_NAME */
  push(index.name());

  /* IS_PRIMARY */
  push(index.is_primary());

  /* IS_UNIQUE */
  push(index.is_unique());

  /* IS_NULLABLE */
  push(index.options().null_part_key());

  /* KEY_LENGTH */
  push(index.key_length());

  /* INDEX_TYPE */
  {
    const char *str;
    uint32_t length;

    switch (index.type())
    {
    default:
    case message::Table::Index::UNKNOWN_INDEX:
      str= "UNKNOWN";
      length= sizeof("UNKNOWN");
      break;
    case message::Table::Index::BTREE:
      str= "BTREE";
      length= sizeof("BTREE");
      break;
    case message::Table::Index::RTREE:
      str= "RTREE";
      length= sizeof("RTREE");
      break;
    case message::Table::Index::HASH:
      str= "HASH";
      length= sizeof("HASH");
      break;
    case message::Table::Index::FULLTEXT:
      str= "FULLTEXT";
      length= sizeof("FULLTEXT");
      break;
    }
    push(str, length);
  }

 /* "INDEX_COMMENT" */
  push(index.comment());
}
