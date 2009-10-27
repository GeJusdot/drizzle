/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2008-2009 Sun Microsystems
 *
 *  Authors:
 *
 *    Jay Pipes <joinfu@sun.com>
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

#ifndef DRIZZLED_PLUGIN_COMMAND_READER_H
#define DRIZZLED_PLUGIN_COMMAND_READER_H

#include <drizzled/replication_services.h> /* For global transaction ID typedef */

/**
 * @file Defines the API for a CommandReader
 *
 * A command reader is a class which is able to read Command messages from some source
 */

namespace drizzled
{
/* some forward declarations needed */
namespace message { class Command; }

namespace plugin
{


/**
 * Class which can read Command messages from some source
 */
class CommandReader : public Plugin
{
  CommandReader();
  CommandReader(const CommandReader &);
  CommandReader& operator=(const CommandReader &);
public:
  explicit CommandReader(std::string name_arg)
    : Plugin(name_arg, "CommandReader") {}
  virtual ~CommandReader() {}
  /**
   * Read and fill a Command message with the supplied
   * Command message global transaction ID.
   *
   * @param Global transaction ID to find
   * @param Pointer to a command message to fill
   *
   * @retval
   *  true if Command message was read successfully and the supplied pointer
   *  to message was filled
   * @retval
   *  false if not found or read successfully
   */
  virtual bool read(const ReplicationServices::GlobalTransactionId &to_read, 
                    message::Command *to_fill)= 0;
};

} /* end namespace drizzled::plugin */
} /* end namespace drizzled */

#endif /* DRIZZLED_PLUGIN_COMMAND_READER_H */
