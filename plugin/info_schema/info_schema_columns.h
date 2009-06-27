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

#ifndef DRIZZLED_INFO_SCHEMA_COLUMNS_H
#define DRIZZLED_INFO_SCHEMA_COLUMNS_H

#include "drizzled/info_schema.h"

/**
 * Create the various volumns for the PROCESSLIST I_S table and add them
 * to the std::vector of columns for the PROCESSLIST table.
 *
 * @param[out] cols vector to add columns to
 */
void createProcessListColumns(std::vector<const ColumnInfo *>& cols);

/**
 * Iterate through the given vector of columns and delete the memory that
 * has been allocated for them.
 *
 * @param[out] cols vector to clear and de-allocate memory from
 */
void clearColumns(std::vector<const ColumnInfo *>& cols);

#endif /* DRIZZLE_INFO_SCHEMA_COLUMNS_H */