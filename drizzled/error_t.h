/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
 *  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 *  Copyright (C) 2009 Sun Microsystems, Inc.
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

#ifndef DRIZZLED_ERROR_T_H
#define DRIZZLED_ERROR_T_H

namespace drizzled { 

enum error_t {

  EE_OK=0,
  EE_ERROR_FIRST=1,
  EE_CANTCREATEFILE,
  EE_READ,
  EE_WRITE,
  EE_BADCLOSE,
  EE_OUTOFMEMORY,
  EE_DELETE,
  EE_LINK,
  EE_EOFERR,
  EE_CANTLOCK,
  EE_CANTUNLOCK,
  EE_DIR,
  EE_STAT,
  EE_CANT_CHSIZE,
  EE_CANT_OPEN_STREAM,
  EE_LINK_WARNING,
  EE_OPEN_WARNING,
  EE_DISK_FULL,
  EE_CANT_MKDIR,
  EE_UNKNOWN_CHARSET,
  EE_OUT_OF_FILERESOURCES,
  EE_CANT_READLINK,
  EE_CANT_SYMLINK,
  EE_REALPATH,
  EE_SYNC,
  EE_UNKNOWN_COLLATION,
  EE_FILENOTFOUND,
  EE_FILE_NOT_CLOSED,
  EE_ERROR_LAST= EE_FILE_NOT_CLOSED,

  HA_ERR_KEY_NOT_FOUND= 120,        /* Didn't find key on read or update */
  HA_ERR_FOUND_DUPP_KEY= 121,       /* Dupplicate key on write */
  HA_ERR_INTERNAL_ERROR=  122,      /* Internal error */
  HA_ERR_RECORD_CHANGED= 123,       /* Uppdate with is recoverable */
  HA_ERR_WRONG_INDEX= 124,          /* Wrong index given to function */
  HA_ERR_CRASHED= 126,              /* Indexfile is crashed */
  HA_ERR_WRONG_IN_RECORD= 127,      /* Record-file is crashed */
  HA_ERR_OUT_OF_MEM= 128,           /* Record-file is crashed */
  HA_ERR_NOT_A_TABLE= 130,          /* not a MYI file - no signature */
  HA_ERR_WRONG_COMMAND= 131,        /* Command not supported */
  HA_ERR_OLD_FILE= 132,             /* old databasfile */
  HA_ERR_NO_ACTIVE_RECORD= 133,     /* No record read in update() */
  HA_ERR_RECORD_DELETED= 134,       /* A record is not there */
  HA_ERR_RECORD_FILE_FULL= 135,     /* No more room in file */
  HA_ERR_INDEX_FILE_FULL= 136,      /* No more room in file */
  HA_ERR_END_OF_FILE= 137,          /* end in next/prev/first/last */
  HA_ERR_UNSUPPORTED= 138,          /* unsupported extension used */
  HA_ERR_TO_BIG_ROW= 139,           /* Too big row */
  HA_WRONG_CREATE_OPTION= 140,      /* Wrong create option */
  HA_ERR_FOUND_DUPP_UNIQUE= 141,    /* Dupplicate unique on write */
  HA_ERR_UNKNOWN_CHARSET = 142,     /* Can't open charset */
  HA_ERR_WRONG_MRG_TABLE_DEF= 143,  /* conflicting tables in MERGE */
  HA_ERR_CRASHED_ON_REPAIR=144,     /* Last (automatic?) repair failed */
  HA_ERR_CRASHED_ON_USAGE= 145,     /* Table must be repaired */
  HA_ERR_LOCK_WAIT_TIMEOUT= 146,
  HA_ERR_LOCK_TABLE_FULL=  147,
  HA_ERR_READ_ONLY_TRANSACTION= 148, /* Updates not allowed */
  HA_ERR_LOCK_DEADLOCK = 149,
  HA_ERR_CANNOT_ADD_FOREIGN= 150,    /* Cannot add a foreign key constr. */
  HA_ERR_NO_REFERENCED_ROW= 151,     /* Cannot add a child row */
  HA_ERR_ROW_IS_REFERENCED= 152,     /* Cannot delete a parent row */
  HA_ERR_NO_SAVEPOINT= 153,          /* No savepoint with that name */
  HA_ERR_NON_UNIQUE_BLOCK_SIZE= 154, /* Non unique key block size */
  HA_ERR_NO_SUCH_TABLE= 155,         /* The table does not exist in engine */
  HA_ERR_TABLE_EXIST= 156,           /* The table existed in storage engine */
  HA_ERR_NO_CONNECTION= 157,         /* Could not connect to storage engine */
  HA_ERR_NULL_IN_SPATIAL= 158,       /* NULLs are not supported in spatial index */
  HA_ERR_TABLE_DEF_CHANGED= 159,     /* The table changed in storage engine */
  HA_ERR_NO_PARTITION_FOUND= 160,    /* There's no partition in table for given value */
  HA_ERR_RBR_LOGGING_FAILED= 161,    /* Row-based binlogging of row failed */
  HA_ERR_DROP_INDEX_FK= 162,         /* Index needed in foreign key constr */
  HA_ERR_FOREIGN_DUPLICATE_KEY= 163, // Upholding foreign key constraints would lead to a duplicate key error
  HA_ERR_TABLE_NEEDS_UPGRADE= 164,   /* The table changed in storage engine */
  HA_ERR_TABLE_READONLY= 165,        /* The table is not writable */
  HA_ERR_AUTOINC_READ_FAILED= 166,   /* Failed to get next autoinc value */
  HA_ERR_AUTOINC_ERANGE= 167,        /* Failed to set row autoinc value */
  HA_ERR_GENERIC= 168,               /* Generic error */
  HA_ERR_RECORD_IS_THE_SAME= 169,    /* row not actually updated: new values same as the old values */
  HA_ERR_LOGGING_IMPOSSIBLE= 170,    /* It is not possible to log this statement */
  HA_ERR_TABLESPACE_EXIST= 171,
  HA_ERR_CORRUPT_EVENT= 172,         /* The event was corrupt, leading to illegal data being read */
  HA_ERR_NEW_FILE= 173,              /* New file format */
  HA_ERR_ROWS_EVENT_APPLY= 174,      /* The event could not be processed no other hanlder error happened */
  HA_ERR_INITIALIZATION= 175,        /* Error during initialization */
  HA_ERR_FILE_TOO_SHORT= 176,        /* File too short */
  HA_ERR_WRONG_CRC= 177,             /* Wrong CRC on page */
  HA_ERR_LOCK_OR_ACTIVE_TRANSACTION= 178,
  HA_ERR_NO_SUCH_TABLESPACE= 179,
  HA_ERR_TABLESPACE_NOT_EMPTY= 180,

  ER_ERROR_FIRST= 1000,
  ER_UNUSED1000= ER_ERROR_FIRST,
  ER_UNUSED1001,
  ER_NO,
  ER_YES,
  ER_CANT_CREATE_FILE,
  ER_CANT_CREATE_TABLE,
  ER_CANT_CREATE_DB,
  ER_DB_CREATE_EXISTS,
  ER_DB_DROP_EXISTS,
  ER_UNUSED1009,
  ER_UNUSED1010,
  ER_CANT_DELETE_FILE,
  ER_UNUSED1012,
  ER_CANT_GET_STAT,
  ER_UNUSED1014,
  ER_CANT_LOCK,
  ER_CANT_OPEN_FILE,
  ER_FILE_NOT_FOUND,
  ER_CANT_READ_DIR,
  ER_UNUSED1019,
  ER_CHECKREAD,
  ER_DISK_FULL,
  ER_DUP_KEY,
  ER_ERROR_ON_CLOSE,
  ER_ERROR_ON_READ,
  ER_ERROR_ON_RENAME,
  ER_ERROR_ON_WRITE,
  ER_FILE_USED,
  ER_FILSORT_ABORT,
  ER_UNUSED1029,
  ER_GET_ERRNO,
  ER_ILLEGAL_HA,
  ER_KEY_NOT_FOUND,
  ER_NOT_FORM_FILE,
  ER_NOT_KEYFILE,
  ER_OLD_KEYFILE,
  ER_OPEN_AS_READONLY,
  ER_OUTOFMEMORY,
  ER_OUT_OF_SORTMEMORY,
  ER_UNEXPECTED_EOF,
  ER_CON_COUNT_ERROR,
  ER_OUT_OF_RESOURCES,
  ER_BAD_HOST_ERROR,
  ER_HANDSHAKE_ERROR,
  ER_DBACCESS_DENIED_ERROR,
  ER_ACCESS_DENIED_ERROR,
  ER_NO_DB_ERROR,
  ER_UNKNOWN_COM_ERROR,
  ER_BAD_NULL_ERROR,
  ER_BAD_DB_ERROR,
  ER_TABLE_EXISTS_ERROR,
  ER_BAD_TABLE_ERROR,
  ER_NON_UNIQ_ERROR,
  ER_SERVER_SHUTDOWN,
  ER_BAD_FIELD_ERROR,
  ER_WRONG_FIELD_WITH_GROUP,
  ER_WRONG_GROUP_FIELD,
  ER_WRONG_SUM_SELECT,
  ER_WRONG_VALUE_COUNT,
  ER_TOO_LONG_IDENT,
  ER_DUP_FIELDNAME,
  ER_DUP_KEYNAME,
  ER_DUP_ENTRY,
  ER_WRONG_FIELD_SPEC,
  ER_PARSE_ERROR,
  ER_EMPTY_QUERY,
  ER_NONUNIQ_TABLE,
  ER_INVALID_DEFAULT,
  ER_MULTIPLE_PRI_KEY,
  ER_TOO_MANY_KEYS,
  ER_TOO_MANY_KEY_PARTS,
  ER_TOO_LONG_KEY,
  ER_KEY_COLUMN_DOES_NOT_EXITS,
  ER_BLOB_USED_AS_KEY,
  ER_TOO_BIG_FIELDLENGTH,
  ER_WRONG_AUTO_KEY,
  ER_UNUSED1076,
  ER_NORMAL_SHUTDOWN,
  ER_GOT_SIGNAL,
  ER_SHUTDOWN_COMPLETE,
  ER_FORCING_CLOSE,
  ER_IPSOCK_ERROR,
  ER_NO_SUCH_INDEX,
  ER_WRONG_FIELD_TERMINATORS,
  ER_BLOBS_AND_NO_TERMINATED,
  ER_TEXTFILE_NOT_READABLE,
  ER_FILE_EXISTS_ERROR,
  ER_LOAD_INFO,
  ER_UNUSED1088,
  ER_WRONG_SUB_KEY,
  ER_CANT_REMOVE_ALL_FIELDS,
  ER_CANT_DROP_FIELD_OR_KEY,
  ER_INSERT_INFO,
  ER_UPDATE_TABLE_USED,
  ER_NO_SUCH_THREAD,
  ER_KILL_DENIED_ERROR,
  ER_NO_TABLES_USED,
  ER_UNUSED1097,
  ER_UNUSED1098,
  ER_UNUSED1099,
  ER_UNUSED1100,
  ER_BLOB_CANT_HAVE_DEFAULT,
  ER_WRONG_DB_NAME,
  ER_WRONG_TABLE_NAME,
  ER_TOO_BIG_SELECT,
  ER_UNKNOWN_ERROR,
  ER_UNKNOWN_PROCEDURE,
  ER_WRONG_PARAMCOUNT_TO_PROCEDURE,
  ER_UNUSED1108,
  ER_UNKNOWN_TABLE,
  ER_FIELD_SPECIFIED_TWICE,
  ER_INVALID_GROUP_FUNC_USE,
  ER_UNSUPPORTED_EXTENSION,
  ER_TABLE_MUST_HAVE_COLUMNS,
  ER_RECORD_FILE_FULL,
  ER_UNUSED1115,
  ER_TOO_MANY_TABLES,
  ER_TOO_MANY_FIELDS,
  ER_TOO_BIG_ROWSIZE,
  ER_UNUSED1119,
  ER_WRONG_OUTER_JOIN,
  ER_NULL_COLUMN_IN_INDEX,
  ER_UNUSED1122,
  ER_UNUSED1123,
  ER_PLUGIN_NO_PATHS,
  ER_PLUGIN_EXISTS,
  ER_CANT_OPEN_LIBRARY,
  ER_CANT_FIND_DL_ENTRY,
  ER_UNUSED1128,
  ER_UNUSED1129,
  ER_UNUSED1130,
  ER_UNUSED1131,
  ER_UNUSED1132,
  ER_UNUSED1133,
  ER_UPDATE_INFO,
  ER_CANT_CREATE_THREAD,
  ER_WRONG_VALUE_COUNT_ON_ROW,
  ER_CANT_REOPEN_TABLE,
  ER_UNUSED1138,
  ER_UNUSED1139,
  ER_MIX_OF_GROUP_FUNC_AND_FIELDS,
  ER_UNUSED1141,
  ER_UNUSED1142,
  ER_UNUSED1143,
  ER_UNUSED1144,
  ER_UNUSED1145,
  ER_NO_SUCH_TABLE,
  ER_UNUSED1147,
  ER_UNUSED1148,
  ER_SYNTAX_ERROR,
  ER_UNUSED1150,
  ER_UNUSED1151,
  ER_UNUSED1152,
  ER_NET_PACKET_TOO_LARGE,
  ER_UNUSED1154,
  ER_UNUSED1155,
  ER_NET_PACKETS_OUT_OF_ORDER,
  ER_UNUSED1157,
  ER_UNUSED1158,
  ER_UNUSED1159,
  ER_UNUSED1160,
  ER_UNUSED1161,
  ER_UNUSED1162,
  ER_TABLE_CANT_HANDLE_BLOB,
  ER_TABLE_CANT_HANDLE_AUTO_INCREMENT,
  ER_UNUSED1165,
  ER_WRONG_COLUMN_NAME,
  ER_WRONG_KEY_COLUMN,
  ER_WRONG_MRG_TABLE,
  ER_DUP_UNIQUE,
  ER_BLOB_KEY_WITHOUT_LENGTH,
  ER_PRIMARY_CANT_HAVE_NULL,
  ER_TOO_MANY_ROWS,
  ER_REQUIRES_PRIMARY_KEY,
  ER_UNUSED1174,
  ER_UNUSED1175,
  ER_KEY_DOES_NOT_EXITS,
  ER_CHECK_NO_SUCH_TABLE,
  ER_CHECK_NOT_IMPLEMENTED,
  ER_UNUSED1179,
  ER_ERROR_DURING_COMMIT,
  ER_ERROR_DURING_ROLLBACK,
  ER_UNUSED1182,
  ER_UNUSED1183,
  ER_NEW_ABORTING_CONNECTION,
  ER_UNUSED1185,
  ER_UNUSED1186,
  ER_UNUSED1187,
  ER_UNUSED1188,
  ER_UNUSED1189,
  ER_UNUSED1190,
  ER_UNUSED1191,
  ER_LOCK_OR_ACTIVE_TRANSACTION,
  ER_UNKNOWN_SYSTEM_VARIABLE,
  ER_CRASHED_ON_USAGE,
  ER_CRASHED_ON_REPAIR,
  ER_WARNING_NOT_COMPLETE_ROLLBACK,
  ER_UNUSED1197,
  ER_UNUSED1198,
  ER_UNUSED1199,
  ER_UNUSED1200,
  ER_UNUSED1201,
  ER_UNUSED1202,
  ER_UNUSED1203,
  ER_SET_CONSTANTS_ONLY,
  ER_LOCK_WAIT_TIMEOUT,
  ER_LOCK_TABLE_FULL,
  ER_READ_ONLY_TRANSACTION,
  ER_DROP_DB_WITH_READ_LOCK,
  ER_UNUSED1209,
  ER_WRONG_ARGUMENTS,
  ER_UNUSED1211,
  ER_UNUSED1212,
  ER_LOCK_DEADLOCK,
  ER_TABLE_CANT_HANDLE_FT,
  ER_CANNOT_ADD_FOREIGN,
  ER_NO_REFERENCED_ROW,
  ER_ROW_IS_REFERENCED,
  ER_UNUSED1218,
  ER_UNUSED1219,
  ER_UNUSED1220,
  ER_WRONG_USAGE,
  ER_WRONG_NUMBER_OF_COLUMNS_IN_SELECT,
  ER_CANT_UPDATE_WITH_READLOCK,
  ER_UNUSED1224,
  ER_UNUSED1225,
  ER_UNUSED1226,
  ER_UNUSED1227,
  ER_LOCAL_VARIABLE,
  ER_GLOBAL_VARIABLE,
  ER_NO_DEFAULT,
  ER_WRONG_VALUE_FOR_VAR,
  ER_WRONG_TYPE_FOR_VAR,
  ER_VAR_CANT_BE_READ,
  ER_CANT_USE_OPTION_HERE,
  ER_NOT_SUPPORTED_YET,
  ER_UNUSED1236,
  ER_UNUSED1237,
  ER_INCORRECT_GLOBAL_LOCAL_VAR,
  ER_WRONG_FK_DEF,
  ER_KEY_REF_DO_NOT_MATCH_TABLE_REF,
  ER_OPERAND_COLUMNS,
  ER_SUBQUERY_NO_1_ROW,
  ER_UNUSED1243,
  ER_UNUSED1244,
  ER_UNUSED1245,
  ER_AUTO_CONVERT,
  ER_ILLEGAL_REFERENCE,
  ER_DERIVED_MUST_HAVE_ALIAS,
  ER_SELECT_REDUCED,
  ER_TABLENAME_NOT_ALLOWED_HERE,
  ER_UNUSED1251,
  ER_SPATIAL_CANT_HAVE_NULL,
  ER_COLLATION_CHARSET_MISMATCH,
  ER_UNUSED1254,
  ER_UNUSED1255,
  ER_TOO_BIG_FOR_UNCOMPRESS,
  ER_ZLIB_Z_MEM_ERROR,
  ER_ZLIB_Z_BUF_ERROR,
  ER_ZLIB_Z_DATA_ERROR,
  ER_CUT_VALUE_GROUP_CONCAT,
  ER_WARN_TOO_FEW_RECORDS,
  ER_WARN_TOO_MANY_RECORDS,
  ER_WARN_NULL_TO_NOTNULL,
  ER_WARN_DATA_OUT_OF_RANGE,
  ER_WARN_DATA_TRUNCATED,
  ER_UNUSED1266,
  ER_CANT_AGGREGATE_2COLLATIONS,
  ER_UNUSED1268,
  ER_UNUSED1269,
  ER_CANT_AGGREGATE_3COLLATIONS,
  ER_CANT_AGGREGATE_NCOLLATIONS,
  ER_VARIABLE_IS_NOT_STRUCT,
  ER_UNKNOWN_COLLATION,
  ER_UNUSED1274,
  ER_UNUSED1275,
  ER_WARN_FIELD_RESOLVED,
  ER_UNUSED1277,
  ER_UNUSED1278,
  ER_UNUSED1279,
  ER_WRONG_NAME_FOR_INDEX,
  ER_WRONG_NAME_FOR_CATALOG,
  ER_UNUSED1282,
  ER_BAD_FT_COLUMN,
  ER_UNUSED1284,
  ER_UNUSED1285,
  ER_UNKNOWN_STORAGE_ENGINE,
  ER_UNUSED1287,
  ER_NON_UPDATABLE_TABLE,
  ER_FEATURE_DISABLED,
  ER_OPTION_PREVENTS_STATEMENT,
  ER_DUPLICATED_VALUE_IN_TYPE,
  ER_TRUNCATED_WRONG_VALUE,
  ER_TOO_MUCH_AUTO_TIMESTAMP_COLS,
  ER_INVALID_ON_UPDATE,
  ER_UNUSED1295,
  ER_GET_ERRMSG,
  ER_GET_TEMPORARY_ERRMSG,
  ER_UNKNOWN_TIME_ZONE,
  ER_UNUSED1299,
  ER_INVALID_CHARACTER_STRING,
  ER_WARN_ALLOWED_PACKET_OVERFLOWED,
  ER_UNUSED1302,
  ER_UNUSED1303,
  ER_UNUSED1304,
  ER_SP_DOES_NOT_EXIST,
  ER_UNUSED1306,
  ER_UNUSED1307,
  ER_UNUSED1308,
  ER_UNUSED1309,
  ER_UNUSED1310,
  ER_UNUSED1311,
  ER_UNUSED1312,
  ER_UNUSED1313,
  ER_UNUSED1314,
  ER_UNUSED1315,
  ER_UNUSED1316,
  ER_QUERY_INTERRUPTED,
  ER_UNUSED1318,
  ER_UNUSED1319,
  ER_UNUSED1320,
  ER_UNUSED1321,
  ER_UNUSED1322,
  ER_UNUSED1323,
  ER_UNUSED1324,
  ER_UNUSED1325,
  ER_UNUSED1326,
  ER_UNUSED1327,
  ER_UNUSED1328,
  ER_SP_FETCH_NO_DATA,
  ER_UNUSED1330,
  ER_UNUSED1331,
  ER_UNUSED1332,
  ER_UNUSED1333,
  ER_UNUSED1334,
  ER_UNUSED1335,
  ER_UNUSED1336,
  ER_UNUSED1337,
  ER_UNUSED1338,
  ER_UNUSED1339,
  ER_UNUSED1340,
  ER_UNUSED1341,
  ER_UNUSED1342,
  ER_UNUSED1343,
  ER_UNUSED1344,
  ER_UNUSED1345,
  ER_UNUSED1346,
  ER_UNUSED1347,
  ER_UNUSED1348,
  ER_UNUSED1349,
  ER_UNUSED1350,
  ER_UNUSED1351,
  ER_UNUSED1352,
  ER_UNUSED1353,
  ER_UNUSED1354,
  ER_UNUSED1355,
  ER_VIEW_INVALID,
  ER_UNUSED1357,
  ER_UNUSED1358,
  ER_UNUSED1359,
  ER_UNUSED1360,
  ER_UNUSED1361,
  ER_UNUSED1362,
  ER_UNUSED1363,
  ER_NO_DEFAULT_FOR_FIELD,
  ER_DIVISION_BY_ZERO,
  ER_TRUNCATED_WRONG_VALUE_FOR_FIELD,
  ER_ILLEGAL_VALUE_FOR_TYPE,
  ER_UNUSED1368,
  ER_UNUSED1369,
  ER_UNUSED1370,
  ER_UNUSED1371,
  ER_UNUSED1372,
  ER_UNUSED1373,
  ER_UNUSED1374,
  ER_UNUSED1375,
  ER_UNUSED1376,
  ER_UNUSED1377,
  ER_UNUSED1378,
  ER_UNUSED1379,
  ER_UNUSED1380,
  ER_UNUSED1381,
  ER_UNUSED1382,
  ER_UNUSED1383,
  ER_UNUSED1384,
  ER_UNUSED1385,
  ER_UNUSED1386,
  ER_UNUSED1387,
  ER_UNUSED1388,
  ER_UNUSED1389,
  ER_UNUSED1390,
  ER_KEY_PART_0,
  ER_UNUSED1392,
  ER_UNUSED1393,
  ER_UNUSED1394,
  ER_UNUSED1395,
  ER_UNUSED1396,
  ER_UNUSED1397,
  ER_UNUSED1398,
  ER_XAER_RMFAIL,
  ER_UNUSED1400,
  ER_UNUSED1401,
  ER_UNUSED1402,
  ER_UNUSED1403,
  ER_UNUSED1404,
  ER_UNUSED1405,
  ER_DATA_TOO_LONG,
  ER_UNUSED1407,
  ER_STARTUP,
  ER_LOAD_FROM_FIXED_SIZE_ROWS_TO_VAR,
  ER_UNUSED1410,
  ER_WRONG_VALUE_FOR_TYPE,
  ER_TABLE_DEF_CHANGED,
  ER_UNUSED1413,
  ER_UNUSED1414,
  ER_SP_NO_RETSET,
  ER_CANT_CREATE_GEOMETRY_OBJECT,
  ER_UNUSED1417,
  ER_UNUSED1418,
  ER_UNUSED1419,
  ER_UNUSED1420,
  ER_UNUSED1421,
  ER_COMMIT_NOT_ALLOWED_IN_SF_OR_TRG,
  ER_UNUSED1423,
  ER_UNUSED1424,
  ER_TOO_BIG_SCALE,
  ER_TOO_BIG_PRECISION,
  ER_M_BIGGER_THAN_D,
  ER_UNUSED1428,
  ER_UNUSED1429,
  ER_UNUSED1430,
  ER_UNUSED1431,
  ER_UNUSED1432,
  ER_UNUSED1433,
  ER_UNUSED1434,
  ER_TRG_IN_WRONG_SCHEMA,
  ER_STACK_OVERRUN_NEED_MORE=1436, // TODO: Test case looks for this int
  ER_UNUSED1437,
  ER_UNUSED1438,
  ER_TOO_BIG_DISPLAYWIDTH,
  ER_UNUSED1440,
  ER_DATETIME_FUNCTION_OVERFLOW,
  ER_UNUSED1442,
  ER_UNUSED1443,
  ER_UNUSED1444,
  ER_UNUSED1445,
  ER_UNUSED1446,
  ER_UNUSED1447,
  ER_UNUSED1448,
  ER_UNUSED1449,
  ER_UNUSED1450,
  ER_ROW_IS_REFERENCED_2,
  ER_NO_REFERENCED_ROW_2,
  ER_UNUSED1453,
  ER_UNUSED1454,
  ER_UNUSED1455,
  ER_UNUSED1456,
  ER_UNUSED1457,
  ER_UNUSED1458,
  ER_TABLE_NEEDS_UPGRADE,
  ER_UNUSED1460,
  ER_UNUSED1461,
  ER_UNUSED1462,
  ER_NON_GROUPING_FIELD_USED,
  ER_TABLE_CANT_HANDLE_SPKEYS,
  ER_UNUSED1465,
  ER_REMOVED_SPACES,
  ER_AUTOINC_READ_FAILED,
  ER_UNUSED1468,
  ER_UNUSED1469,
  ER_WRONG_STRING_LENGTH,
  ER_UNUSED1471,
  ER_UNUSED1472,
  ER_TOO_HIGH_LEVEL_OF_NESTING_FOR_SELECT,
  ER_NAME_BECOMES_EMPTY,
  ER_AMBIGUOUS_FIELD_TERM,
  ER_UNUSED1476,
  ER_UNUSED1477,
  ER_ILLEGAL_HA_CREATE_OPTION,
  ER_UNUSED1479,
  ER_UNUSED1480,
  ER_UNUSED1481,
  ER_UNUSED1482,
  ER_UNUSED1483,
  ER_UNUSED1484,
  ER_UNUSED1485,
  ER_UNUSED1486,
  ER_UNUSED1487,
  ER_UNUSED1488,
  ER_UNUSED1489,
  ER_UNUSED1490,
  ER_UNUSED1491,
  ER_UNUSED1492,
  ER_UNUSED1493,
  ER_UNUSED1494,
  ER_UNUSED1495,
  ER_UNUSED1496,
  ER_UNUSED1497,
  ER_UNUSED1498,
  ER_UNUSED1499,
  ER_UNUSED1500,
  ER_UNUSED1501,
  ER_UNUSED1502,
  ER_UNUSED1503,
  ER_UNUSED1504,
  ER_UNUSED1505,
  ER_UNUSED1506,
  ER_UNUSED1507,
  ER_UNUSED1508,
  ER_UNUSED1509,
  ER_UNUSED1510,
  ER_UNUSED1511,
  ER_UNUSED1512,
  ER_UNUSED1513,
  ER_UNUSED1514,
  ER_UNUSED1515,
  ER_UNUSED1516,
  ER_UNUSED1517,
  ER_UNUSED1518,
  ER_UNUSED1519,
  ER_UNUSED1520,
  ER_UNUSED1521,
  ER_UNUSED1522,
  ER_UNUSED1523,
  ER_INVALID_OPTION_VALUE,
  ER_WRONG_VALUE,
  ER_NO_PARTITION_FOR_GIVEN_VALUE,
  ER_UNUSED1527,
  ER_UNUSED1528,
  ER_UNUSED1529,
  ER_UNUSED1530,
  ER_UNUSED1531,
  ER_UNUSED1532,
  ER_UNUSED1533,
  ER_BINLOG_ROW_LOGGING_FAILED,
  ER_UNUSED1535,
  ER_UNUSED1536,
  ER_UNUSED1537,
  ER_UNUSED1538,
  ER_UNUSED1539,
  ER_UNUSED1540,
  ER_UNUSED1541,
  ER_UNUSED1542,
  ER_UNUSED1543,
  ER_UNUSED1544,
  ER_UNUSED1545,
  ER_UNUSED1546,
  ER_UNUSED1547,
  ER_UNUSED1548,
  ER_UNUSED1549,
  ER_UNUSED1550,
  ER_UNUSED1551,
  ER_UNUSED1552,
  ER_DROP_INDEX_FK,
  ER_UNUSED1554,
  ER_UNUSED1555,
  ER_UNUSED1556,
  ER_FOREIGN_DUPLICATE_KEY,
  ER_UNUSED1558,
  ER_UNUSED1559,
  ER_UNUSED1560,
  ER_UNUSED1561,
  ER_UNUSED1562,
  ER_UNUSED1563,
  ER_UNUSED1564,
  ER_UNUSED1565,
  ER_UNUSED1566,
  ER_UNUSED1567,
  ER_CANT_CHANGE_TX_ISOLATION,
  ER_UNUSED1569,
  ER_UNUSED1570,
  ER_UNUSED1571,
  ER_UNUSED1572,
  ER_UNUSED1573,
  ER_UNUSED1574,
  ER_UNUSED1575,
  ER_UNUSED1576,
  ER_UNUSED1577,
  ER_UNUSED1578,
  ER_UNUSED1579,
  ER_UNUSED1580,
  ER_UNUSED1581,
  ER_WRONG_PARAMCOUNT_TO_FUNCTION,
  ER_WRONG_PARAMETERS_TO_NATIVE_FCT,
  ER_UNUSED1584,
  ER_UNUSED1585,
  ER_DUP_ENTRY_WITH_KEY_NAME,
  ER_UNUSED1587,
  ER_UNUSED1588,
  ER_UNUSED1589,
  ER_UNUSED1590,
  ER_UNUSED1591,
  ER_UNUSED1592,
  ER_UNUSED1593,
  ER_UNUSED1594,
  ER_UNUSED1595,
  ER_UNUSED1596,
  ER_UNUSED1597,
  ER_UNUSED1598,
  ER_UNUSED1599,
  ER_UNUSED1600,
  ER_UNUSED1601,
  ER_UNUSED1602,
  ER_UNUSED1603,
  ER_UNUSED1604,
  ER_UNUSED1605,
  ER_UNUSED1606,
  ER_UNUSED1607,
  ER_UNUSED1608,
  ER_UNUSED1609,
  ER_UNUSED1610,
  ER_LOAD_DATA_INVALID_COLUMN,
  ER_UNUSED1612,
  ER_UNUSED1613,
  ER_UNUSED1614,
  ER_UNUSED1615,
  ER_UNUSED1616,
  ER_UNUSED1617,
  ER_UNUSED1618,
  ER_UNUSED1619,
  ER_UNUSED1620,
  ER_UNUSED1621,
  ER_UNUSED1622,
  ER_UNUSED1623,
  ER_UNUSED1624,
  ER_UNUSED1625,
  ER_UNUSED1626,
  ER_UNUSED1627,
  ER_UNUSED1628,
  ER_UNUSED1629,
  ER_UNUSED1630,
  ER_UNUSED1631,
  ER_UNUSED1632,
  ER_UNUSED1633,
  ER_UNUSED1634,
  ER_UNUSED1635,
  ER_UNUSED1636,
  ER_UNUSED1637,
  ER_UNUSED1638,
  ER_UNUSED1639,
  ER_UNUSED1640,
  ER_UNUSED1641,
  ER_UNUSED1642,
  ER_UNUSED1643,
  ER_UNUSED1644,
  ER_UNUSED1645,
  ER_UNUSED1646,
  ER_UNUSED1647,
  ER_UNUSED1648,
  ER_UNUSED1649,
  ER_UNUSED1650,
  ER_UNUSED1651,
  ER_UNUSED1652,
  ER_UNUSED1653,
  ER_UNUSED1654,
  ER_UNUSED1655,
  ER_UNUSED1656,
  ER_UNUSED1657,
  ER_UNUSED1658,
  ER_UNUSED1659,
  ER_UNUSED1660,
  ER_UNUSED1661,
  ER_UNUSED1662,
  ER_UNUSED1663,
  ER_UNUSED1664,
  ER_UNUSED1665,
  ER_UNUSED1666,
  ER_UNUSED1667,
  ER_UNUSED1668,
  ER_UNUSED1669,
  ER_UNUSED1670,
  ER_UNUSED1671,
  ER_UNUSED1672,
  ER_UNUSED1673,
  ER_UNUSED1674,
  ER_UNUSED1675,
  ER_UNUSED1676,
  ER_UNUSED1677,
  ER_UNUSED1678,
  ER_UNUSED1679,
  ER_UNUSED1680,
  ER_UNUSED1681,
  ER_UNUSED1682,
  ER_UNUSED1683,
  ER_UNUSED1684,
  ER_INVALID_UNIX_TIMESTAMP_VALUE,
  ER_INVALID_DATETIME_VALUE,
  ER_INVALID_NULL_ARGUMENT,
  ER_UNUSED1688,
  ER_ARGUMENT_OUT_OF_RANGE,
  ER_UNUSED1690,
  ER_INVALID_ENUM_VALUE,
  ER_NO_PRIMARY_KEY_ON_REPLICATED_TABLE,
  ER_CORRUPT_TABLE_DEFINITION,
  ER_SCHEMA_DOES_NOT_EXIST,
  ER_ALTER_SCHEMA,
  ER_DROP_SCHEMA,
  ER_USE_SQL_BIG_RESULT,
  ER_UNKNOWN_ENGINE_OPTION,
  ER_UNKNOWN_SCHEMA_OPTION,
  ER_EVENT_OBSERVER_PLUGIN,
  ER_CORRUPT_SCHEMA_DEFINITION,
  ER_OUT_OF_GLOBAL_SORTMEMORY,
  ER_OUT_OF_GLOBAL_JOINMEMORY,
  ER_OUT_OF_GLOBAL_READRNDMEMORY,
  ER_OUT_OF_GLOBAL_READMEMORY,
  ER_USER_LOCKS_CANT_WAIT_ON_OWN_BARRIER,
  ER_USER_LOCKS_UNKNOWN_BARRIER,
  ER_USER_LOCKS_NOT_OWNER_OF_BARRIER,
  ER_USER_LOCKS_CANT_WAIT_ON_OWN_LOCK,
  ER_USER_LOCKS_NOT_OWNER_OF_LOCK,
  ER_USER_LOCKS_INVALID_NAME_BARRIER,
  ER_USER_LOCKS_INVALID_NAME_LOCK,
  ER_KILL_DENY_SELF_ERROR,
  ER_INVALID_ALTER_TABLE_FOR_NOT_NULL,
  ER_ADMIN_ACCESS,
  ER_INVALID_UUID_VALUE,
  ER_INVALID_UUID_TIME,
  ER_CORRUPT_TABLE_DEFINITION_ENUM,
  ER_CORRUPT_TABLE_DEFINITION_UNKNOWN_COLLATION,
  ER_CORRUPT_TABLE_DEFINITION_UNKNOWN,
  ER_INVALID_CAST_TO_SIGNED,
  ER_INVALID_CAST_TO_UNSIGNED,
  ER_SQL_KEYWORD,
  ER_INVALID_BOOLEAN_VALUE,
  ER_ASSERT,
  ER_ASSERT_NULL,
  ER_CATALOG_CANNOT_CREATE,
  ER_CATALOG_CANNOT_DROP,
  ER_CATALOG_DOES_NOT_EXIST,
  ER_CATALOG_NO_DROP_LOCAL,
  ER_CATALOG_NO_LOCK,
  ER_CORRUPT_CATALOG_DEFINITION,
  ER_TABLE_DROP,
  ER_TABLE_DROP_ERROR_OCCURRED,
  ER_TABLE_PERMISSION_DENIED,
  ER_TABLE_UNKNOWN,

  ER_INVALID_CAST_TO_BOOLEAN,

  // Leave ER_INVALID_BOOLEAN_VALUE as LAST, and force people to use tags
  // instead of numbers in error messages in test.
  ER_ERROR_LAST= ER_INVALID_CAST_TO_BOOLEAN
};


} /* namespace drizzled */

#endif /* DRIZZLED_ERROR_T_H */
