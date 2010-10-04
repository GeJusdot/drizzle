/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2009 - 2010 Toru Maesaka
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

#include "config.h"
#include "ha_blitz.h"

using namespace std;
using namespace drizzled;

int BlitzData::startup(const char *path) {
  int rv = 0;

  if ((rv = open_data_table(path, HDBOWRITER)) != 0)
    return rv;

  current_hidden_row_id = read_meta_row_id();
  return rv;
}

int BlitzData::shutdown() {
  int rv = 0;

  /* Copy the latest autogenerated ID back to TC's metadata buffer.
     This data will be sync'd by TC. */
  write_meta_row_id(current_hidden_row_id);

  if ((rv = close_data_table()) != 0)
    return rv;

  return rv;
}

/* Similar to UNIX touch(1) but generates a tuned TCHDB file. */
int BlitzData::create_data_table(drizzled::message::Table &proto,
                                 drizzled::Table &table_info,
                                 const drizzled::TableIdentifier &identifier) {

  std::string path = identifier.getPath() + BLITZ_DATA_EXT;

  uint64_t autoinc = (proto.options().has_auto_increment_value())
                     ? proto.options().auto_increment_value() - 1 : 0;

  uint64_t hash_buckets = (blitz_estimated_rows == 0) ? BLITZ_TC_BUCKETS
                                                      : blitz_estimated_rows;
  int n_options = proto.engine().options_size();

  for (int i = 0; i < n_options; i++) {
    if (proto.engine().options(i).name() == "estimated_rows" ||
        proto.engine().options(i).name() == "ESTIMATED_ROWS") {
      std::istringstream stream(proto.engine().options(i).state());
      stream >> hash_buckets;

      if (hash_buckets <= 0)
        hash_buckets = BLITZ_TC_BUCKETS;
    }
  }

  if (data_table != NULL)
    return HA_ERR_GENERIC;

  if ((data_table = tchdbnew()) == NULL)
    return HA_ERR_OUT_OF_MEM;

  if (!tchdbtune(data_table, hash_buckets, -1, -1, 0)) {
    tchdbdel(data_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  if (!tchdbopen(data_table, path.c_str(), (HDBOWRITER | HDBOCREAT))) {
    tchdbdel(data_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  /* Write the Meta Data for this Table. */
  tc_meta_buffer = tchdbopaque(data_table);
  write_meta_autoinc(autoinc);
  write_meta_keycount(table_info.s->keys);

  /* We're Done. */
  if (close_data_table() != 0)
    return HA_ERR_CRASHED_ON_USAGE;

  return 0;
}

int BlitzData::open_data_table(const char *path, const int mode) {
  char buf[FN_REFLEN];

  if ((data_table = tchdbnew()) == NULL)
    return HA_ERR_OUT_OF_MEM;

  if (!tchdbsetmutex(data_table)) {
    tchdbdel(data_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  if (!tchdbsetxmsiz(data_table, BLITZ_TC_EXTRA_MMAP_SIZE)) {
    tchdbdel(data_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  snprintf(buf, FN_REFLEN, "%s%s", path, BLITZ_DATA_EXT);

  if (!tchdbopen(data_table, buf, mode)) {
    tchdbdel(data_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  tc_meta_buffer = tchdbopaque(data_table);
  return 0;
}

bool BlitzData::rename_table(const char *from, const char *to) {
  char from_buf[FN_REFLEN];
  char to_buf[FN_REFLEN];

  snprintf(from_buf, FN_REFLEN, "%s%s", from, BLITZ_DATA_EXT);
  snprintf(to_buf, FN_REFLEN, "%s%s", to, BLITZ_DATA_EXT);

  if (rename(from_buf, to_buf) != 0)
    return false;

  snprintf(from_buf, FN_REFLEN, "%s%s", from, BLITZ_SYSTEM_EXT);
  snprintf(to_buf, FN_REFLEN, "%s%s", to, BLITZ_SYSTEM_EXT);

  if (rename(from_buf, to_buf) != 0)
    return false;

  return true;
}

int BlitzData::close_data_table(void) {
  assert(data_table);

  if (!tchdbclose(data_table)) {
    tchdbdel(data_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  tchdbdel(data_table);
  data_table = NULL;
  tc_meta_buffer = NULL;
  return 0;
}

uint64_t BlitzData::nrecords(void) {
  return tchdbrnum(data_table);
}

uint64_t BlitzData::table_size(void) {
  return tchdbfsiz(data_table);
}

uint64_t BlitzData::read_meta_row_id(void) {
  assert(tc_meta_buffer);
  return (uint64_t)uint8korr(tc_meta_buffer);
}

uint64_t BlitzData::read_meta_autoinc(void) {
  assert(tc_meta_buffer);
  char *pos = tc_meta_buffer + sizeof (current_hidden_row_id);
  return (uint64_t)uint8korr(pos);
}

uint32_t BlitzData::read_meta_keycount(void) {
  assert(tc_meta_buffer);
  char *pos = tc_meta_buffer;
  pos += sizeof(current_hidden_row_id) + sizeof(uint64_t);
  return (uint32_t)uint4korr(pos);
}

void BlitzData::write_meta_row_id(uint64_t row_id) {
  assert(tc_meta_buffer);
  int8store(tc_meta_buffer, row_id);
}

void BlitzData::write_meta_autoinc(uint64_t num) {
  assert(tc_meta_buffer);
  char *pos = tc_meta_buffer + sizeof(current_hidden_row_id);
  int8store(pos, num);
}

void BlitzData::write_meta_keycount(uint32_t nkeys) {
  assert(tc_meta_buffer);
  char *pos = tc_meta_buffer;
  pos += sizeof(current_hidden_row_id) + sizeof(uint64_t);
  int4store(pos, nkeys);
}

char *BlitzData::get_row(const char *key, const size_t klen, int *vlen) {
  return (char *)tchdbget(data_table, key, klen, vlen);
}

/* Fastest way to fetch both key and value from TCHDB since it only
   involves one allocation. That is, both key and value are living
   on the same block of memory. The return value is a pointer to the
   next key. Technically it is a pointer to the region of memory that
   holds both key and value. */
char *BlitzData::next_key_and_row(const char *key, const size_t klen,
                                  int *next_key_len, const char **value,
                                  int *value_len) {
  return tchdbgetnext3(data_table, key, klen, next_key_len, value, value_len);
}

char *BlitzData::first_row(int *row_len) {
  return (char *)tchdbgetnext(data_table, NULL, 0, row_len);
}

uint64_t BlitzData::next_hidden_row_id(void) {
  /* current_hidden_row_id is an atomic type */
  uint64_t rv = current_hidden_row_id.increment();
  return rv;
}

int BlitzData::write_row(const char *key, const size_t klen,
                         const unsigned char *row, const size_t rlen) {
  return (tchdbput(data_table, key, klen, row, rlen)) ? 0 : 1;
}

int BlitzData::write_unique_row(const char *key, const size_t klen,
                                const unsigned char *row, const size_t rlen) {
  int rv = 0;

  if (!tchdbputkeep(data_table, key, klen, row, rlen)) {
    if (tchdbecode(data_table) == TCEKEEP) {
      errno = HA_ERR_FOUND_DUPP_KEY;
      rv = HA_ERR_FOUND_DUPP_KEY;
    }
  }
  return rv;
}

int BlitzData::delete_row(const char *key, const size_t klen) {
  return (tchdbout(data_table, key, klen)) ? 0 : -1;
}

bool BlitzData::delete_all_rows() {
  char buf[BLITZ_MAX_META_LEN];

  /* Evacuate the meta data buffer since this will be wiped out. */
  memcpy(buf, tc_meta_buffer, BLITZ_MAX_META_LEN);

  /* Now it's safe to wipe everything. */
  if (!tchdbvanish(data_table))
    return false;

  /* Copy the evacuated meta buffer back to the fresh TCHDB file. */
  tc_meta_buffer = tchdbopaque(data_table);
  memcpy(tc_meta_buffer, buf, BLITZ_MAX_META_LEN);
  
  return true;
}

/* Code from here on is for BlitzDB's internal system information management.
   It is deliberately separated from the data dictionary code because we
   might move to a simple flat file structure in the future. For now we
   use a micro Tokyo Cabinet database for this. */
int BlitzData::create_system_table(const std::string &path) {
  int rv = 0;
  int mode = (HDBOWRITER | HDBOCREAT);

  if ((rv = open_system_table(path.c_str(), mode)) != 0)
    return rv;

  return close_system_table();
}

int BlitzData::open_system_table(const std::string &path, const int mode) {
  char buf[FN_REFLEN];
  const int BUCKETS = 7;

  if ((system_table = tchdbnew()) == NULL)
    return HA_ERR_OUT_OF_MEM;

  if (!tchdbsetmutex(system_table)) {
    tchdbdel(system_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  if (!tchdbtune(system_table, BUCKETS, -1, -1, 0)) {
    tchdbdel(system_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  snprintf(buf, FN_REFLEN, "%s%s", path.c_str(), BLITZ_SYSTEM_EXT);

  if (!tchdbopen(system_table, buf, mode)) {
    tchdbdel(system_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }

  return 0;
}

int BlitzData::close_system_table(void) {
  assert(system_table);

  if (!tchdbclose(system_table)) {
    tchdbdel(system_table);
    return HA_ERR_CRASHED_ON_USAGE;
  }
  tchdbdel(system_table);
  return 0;
}

bool BlitzData::write_table_definition(drizzled::message::Table &proto) {
  assert(system_table);

  std::string serialized_proto;
  proto.SerializeToString(&serialized_proto);

  if (!tchdbput(system_table, BLITZ_TABLE_PROTO_KEY.c_str(),
                BLITZ_TABLE_PROTO_KEY.length(), serialized_proto.c_str(),
                serialized_proto.length())) {
    return false;
  }

  if (proto.options().has_comment()) {
    if (!tchdbput(system_table, BLITZ_TABLE_PROTO_COMMENT_KEY.c_str(),
                  BLITZ_TABLE_PROTO_COMMENT_KEY.length(),
                  proto.options().comment().c_str(),
                  proto.options().comment().length())) {
      return false;
    }
  }
  return true;
}

char *BlitzData::get_system_entry(const char *key, const size_t klen,
                                  int *vlen) {
  assert(system_table);
  return (char *)tchdbget(system_table, key, klen, vlen);
}
