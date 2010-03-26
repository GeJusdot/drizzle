/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (c) 2010 Jay Pipes
 *
 *  Authors:
 *
 *    Jay Pipes <jaypipes@gmail.com>
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

#ifndef DRIZZLED_PLUGIN_REPLICATION_H
#define DRIZZLED_PLUGIN_REPLICATION_H

/**
 * @file Common structs and enums for the replication API
 */

namespace drizzled
{

namespace plugin
{

typedef enum replication_return_code_t
{
  SUCCESS= 0, /* no error */
  UNKNOWN_ERROR= 1
} ReplicationReturnCode;

} /* namespace plugin */
} /* namespace drizzled */

#endif /* DRIZZLED_PLUGIN_REPLICATION_H */
