/* - mode: c; c-basic-offset: 2; indent-tabs-mode: nil; -*-
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

/**
 * @file
 *
 * Defines the implementation of a simple replicator that can filter
 * events based on a schema or table name.
 *
 * @details
 *
 * This is a very simple implementation.  All we do is maintain two
 * std::vectors:
 *
 *  1) contains all the schema names to filter
 *  2) contains all the table names to filter
 *
 * If an event is on a schema or table in the vectors described above, then
 * the event will not be passed along to the applier.
 */

#include "filtered_replicator.h"

#include <drizzled/gettext.h>
#include <drizzled/message/transaction.pb.h>

#include <vector>
#include <string>

using namespace std;

static bool sysvar_filtered_replicator_enabled= false;
static char *sysvar_filtered_replicator_sch_filters= NULL;
static char *sysvar_filtered_replicator_tab_filters= NULL;

FilteredReplicator::FilteredReplicator(const char *in_sch_filters,
                                       const char *in_tab_filters)
{
  /* 
   * Add each of the specified schemas to the vector of schemas
   * to filter.
   */
  if (in_sch_filters)
  {
    populateFilter(in_sch_filters, schemas_to_filter);
  }

  /* 
   * Add each of the specified tables to the vector of tables
   * to filter.
   */
  if (in_tab_filters)
  {
    populateFilter(in_tab_filters, tables_to_filter);
  }
}

bool FilteredReplicator::isActive()
{
  return sysvar_filtered_replicator_enabled;
}

void FilteredReplicator::replicate(drizzled::plugin::Applier *in_applier, drizzled::message::Command *to_replicate)
{
  /* 
   * We first check if this event should be filtered or not...
   */
  if (isSchemaFiltered(to_replicate->schema()) ||
      isTableFiltered(to_replicate->table()))
  {
    return;
  }

  /*
   * We can now simply call the applier's apply() method, passing
   * along the supplied command.
   */
  in_applier->apply(to_replicate);
}

void FilteredReplicator::populateFilter(const char *input,
                                        vector<string> &filter)
{
  string filter_list(input);
  string::size_type last_pos= filter_list.find_first_not_of(',', 0);
  string::size_type pos= filter_list.find_first_of(',', last_pos);

  while (pos != string::npos || last_pos != string::npos)
  {
    filter.push_back(filter_list.substr(last_pos, pos - last_pos));
    last_pos= filter_list.find_first_not_of(',', pos);
    pos= filter_list.find_first_of(',', last_pos);
  }
}

bool FilteredReplicator::isSchemaFiltered(const string &schema_name)
{
  vector<string>::iterator it= find(schemas_to_filter.begin(),
                                    schemas_to_filter.end(),
                                    schema_name);
  if (it != schemas_to_filter.end())
  {
    return true;
  }
  return false;
}

bool FilteredReplicator::isTableFiltered(const string &table_name)
{
  vector<string>::iterator it= find(tables_to_filter.begin(),
                                    tables_to_filter.end(),
                                    table_name);
  if (it != tables_to_filter.end())
  {
    return true;
  }
  return false;
}

static FilteredReplicator *filtered_replicator= NULL; /* The singleton replicator */

static int init(PluginRegistry &registry)
{
  if (sysvar_filtered_replicator_enabled)
  {
    filtered_replicator= new(std::nothrow) 
      FilteredReplicator(sysvar_filtered_replicator_sch_filters,
                         sysvar_filtered_replicator_tab_filters);
    if (filtered_replicator == NULL)
    {
      return 1;
    }
    registry.add(filtered_replicator);
  }
  return 0;
}

static int deinit(PluginRegistry &registry)
{
  if (filtered_replicator)
  {
    registry.remove(filtered_replicator);
    delete filtered_replicator;
  }
  return 0;
}

static DRIZZLE_SYSVAR_BOOL(enable,
                           sysvar_filtered_replicator_enabled,
                           PLUGIN_VAR_NOCMDARG,
                           N_("Enable filtered replicator"),
                           NULL, /* check func */
                           NULL, /* update func */
                           false /*default */);
static DRIZZLE_SYSVAR_STR(filteredschemas,
                          sysvar_filtered_replicator_sch_filters,
                          PLUGIN_VAR_OPCMDARG,
                          N_("List of schemas to filter"),
                          NULL,
                          NULL,
                          false);
static DRIZZLE_SYSVAR_STR(filteredtables,
                          sysvar_filtered_replicator_tab_filters,
                          PLUGIN_VAR_OPCMDARG,
                          N_("List of tables to filter"),
                          NULL,
                          NULL,
                          false);

static struct st_mysql_sys_var* filtered_replicator_system_variables[]= {
  DRIZZLE_SYSVAR(enable),
  DRIZZLE_SYSVAR(filteredschemas),
  DRIZZLE_SYSVAR(filteredtables),
  NULL
};

drizzle_declare_plugin(filtered_replicator)
{
  "filtered_replicator",
  "0.1",
  "Padraig O'Sullivan",
  N_("Filtered Replicator"),
  PLUGIN_LICENSE_GPL,
  init, /* Plugin Init */
  deinit, /* Plugin Deinit */
  NULL, /* status variables */
  filtered_replicator_system_variables, /* system variables */
  NULL    /* config options */
}
drizzle_declare_plugin_end;
