/*
 * Copyright (c) 2010, Joseph Daly <skinny.moey@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *   * Neither the name of Joseph Daly nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PLUGIN_LOGGING_STATS_USER_COMMANDS_H
#define PLUGIN_LOGGING_STATS_USER_COMMANDS_H

#include <drizzled/common.h>

#include <string>

class UserCommands
{
public:

  UserCommands();

  UserCommands(const UserCommands &user_commands);

  uint64_t getSelectCount();

  void incrementSelectCount(int i= 1);


  uint64_t getUpdateCount();

  void incrementUpdateCount(int i= 1);


  uint64_t getDeleteCount();

  void incrementDeleteCount(int i= 1);


  uint64_t getInsertCount();

  void incrementInsertCount(int i= 1);


  uint64_t getRollbackCount();

  void incrementRollbackCount(int i= 1);


  uint64_t getCommitCount();

  void incrementCommitCount(int i= 1);


  uint64_t getCreateCount();

  void incrementCreateCount(int i= 1);


  uint64_t getAlterCount();

  void incrementAlterCount(int i= 1);


  uint64_t getDropCount();

  void incrementDropCount(int i= 1);


  uint64_t getAdminCount();

  void incrementAdminCount(int i= 1);

  void merge(UserCommands *user_commands);

  void reset();

private:
  uint64_t update_count;
  uint64_t delete_count;
  uint64_t insert_count;
  uint64_t select_count;
  uint64_t rollback_count;
  uint64_t commit_count;
  uint64_t create_count;
  uint64_t alter_count;
  uint64_t drop_count;
  uint64_t admin_count;
};

#endif /* PLUGIN_LOGGING_STATS_USER_COMMANDS_H */