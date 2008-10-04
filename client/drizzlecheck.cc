/* Copyright (C) 2008 Drizzle development team

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* By Jani Tolonen, 2001-04-20, MySQL, MYSQL Development Team */

#define CHECK_VERSION "2.5.0"

#include "config.h"
#include <vector>
#include <string>
#include "client_priv.h"
#include <mystrings/m_ctype.h>

template class std::vector<std::string>;

using namespace std;
/* Exit codes */

#define EX_USAGE 1
#define EX_MYSQLERR 2

static DRIZZLE drizzle_connection, *sock = 0;
static bool opt_alldbs = 0, opt_check_only_changed = 0, opt_extended = 0,
               opt_compress = 0, opt_databases = 0, opt_fast = 0,
               opt_medium_check = 0, opt_quick = 0, opt_all_in_1 = 0,
               opt_silent = 0, opt_auto_repair = 0, ignore_errors = 0,
               tty_password= 0, opt_frm= 0, debug_info_flag= 0, debug_check_flag= 0,
               opt_fix_table_names= 0, opt_fix_db_names= 0, opt_upgrade= 0,
               opt_write_binlog= 1;
static uint verbose = 0, opt_mysql_port=0;
static int my_end_arg;
static char * opt_mysql_unix_port = 0;
static char *opt_password = 0, *current_user = 0,
      *default_charset = (char *)DRIZZLE_DEFAULT_CHARSET_NAME,
      *current_host = 0;
static int first_error = 0;
vector<string> tables4repair;
static uint opt_protocol=0;
static const CHARSET_INFO *charset_info= &my_charset_utf8_general_ci;

enum operations { DO_CHECK, DO_REPAIR, DO_ANALYZE, DO_OPTIMIZE, DO_UPGRADE };

static struct my_option my_long_options[] =
{
  {"all-databases", 'A',
   "Check all the databases. This will be same as  --databases with all databases selected.",
   (char**) &opt_alldbs, (char**) &opt_alldbs, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0,
   0, 0},
  {"analyze", 'a', "Analyze given tables.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0,
   0, 0, 0, 0},
  {"all-in-1", '1',
   "Instead of issuing one query for each table, use one query per database, naming all tables in the database in a comma-separated list.",
   (char**) &opt_all_in_1, (char**) &opt_all_in_1, 0, GET_BOOL, NO_ARG, 0, 0, 0,
   0, 0, 0},
  {"auto-repair", OPT_AUTO_REPAIR,
   "If a checked table is corrupted, automatically fix it. Repairing will be done after all tables have been checked, if corrupted ones were found.",
   (char**) &opt_auto_repair, (char**) &opt_auto_repair, 0, GET_BOOL, NO_ARG, 0,
   0, 0, 0, 0, 0},
  {"character-sets-dir", OPT_CHARSETS_DIR,
   "Directory where character sets are.", (char**) &charsets_dir,
   (char**) &charsets_dir, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"check", 'c', "Check table for errors.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0,
   0, 0, 0, 0},
  {"check-only-changed", 'C',
   "Check only tables that have changed since last check or haven't been closed properly.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"check-upgrade", 'g',
   "Check tables for version-dependent changes. May be used with --auto-repair to correct tables requiring version-dependent updates.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"compress", OPT_COMPRESS, "Use compression in server/client protocol.",
   (char**) &opt_compress, (char**) &opt_compress, 0, GET_BOOL, NO_ARG, 0, 0, 0,
   0, 0, 0},
  {"databases", 'B',
   "To check several databases. Note the difference in usage; In this case no tables are given. All name arguments are regarded as databasenames.",
   (char**) &opt_databases, (char**) &opt_databases, 0, GET_BOOL, NO_ARG,
   0, 0, 0, 0, 0, 0},
  {"debug-check", OPT_DEBUG_CHECK, "Check memory and open file usage at exit.",
   (char**) &debug_check_flag, (char**) &debug_check_flag, 0,
   GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"debug-info", OPT_DEBUG_INFO, "Print some debug info at exit.",
   (char**) &debug_info_flag, (char**) &debug_info_flag,
   0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"default-character-set", OPT_DEFAULT_CHARSET,
   "Set the default character set.", (char**) &default_charset,
   (char**) &default_charset, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"fast",'F', "Check only tables that haven't been closed properly.",
   (char**) &opt_fast, (char**) &opt_fast, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0,
   0},
  {"fix-db-names", OPT_FIX_DB_NAMES, "Fix database names.",
    (char**) &opt_fix_db_names, (char**) &opt_fix_db_names,
    0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"fix-table-names", OPT_FIX_TABLE_NAMES, "Fix table names.",
    (char**) &opt_fix_table_names, (char**) &opt_fix_table_names,
    0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"force", 'f', "Continue even if we get an sql-error.",
   (char**) &ignore_errors, (char**) &ignore_errors, 0, GET_BOOL, NO_ARG, 0, 0,
   0, 0, 0, 0},
  {"extended", 'e',
   "If you are using this option with CHECK TABLE, it will ensure that the table is 100 percent consistent, but will take a long time. If you are using this option with REPAIR TABLE, it will force using old slow repair with keycache method, instead of much faster repair by sorting.",
   (char**) &opt_extended, (char**) &opt_extended, 0, GET_BOOL, NO_ARG, 0, 0, 0,
   0, 0, 0},
  {"help", '?', "Display this help message and exit.", 0, 0, 0, GET_NO_ARG,
   NO_ARG, 0, 0, 0, 0, 0, 0},
  {"host",'h', "Connect to host.", (char**) &current_host,
   (char**) &current_host, 0, GET_STR_ALLOC, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"medium-check", 'm',
   "Faster than extended-check, but only finds 99.99 percent of all errors. Should be good enough for most cases.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"write-binlog", OPT_WRITE_BINLOG,
   "Log ANALYZE, OPTIMIZE and REPAIR TABLE commands. Use --skip-write-binlog when commands should not be sent to replication slaves.",
   (char**) &opt_write_binlog, (char**) &opt_write_binlog, 0, GET_BOOL, NO_ARG,
   1, 0, 0, 0, 0, 0},
  {"optimize", 'o', "Optimize table.", 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0,
   0, 0},
  {"password", 'p',
   "Password to use when connecting to server. If password is not given it's solicited on the tty.",
   0, 0, 0, GET_STR, OPT_ARG, 0, 0, 0, 0, 0, 0},
  {"port", 'P', "Port number to use for connection or 0 for default to, in "
   "order of preference, my.cnf, $DRIZZLE_TCP_PORT, "
   "built-in default (" STRINGIFY_ARG(DRIZZLE_PORT) ").",
   (char**) &opt_mysql_port,
   (char**) &opt_mysql_port, 0, GET_UINT, REQUIRED_ARG, 0, 0, 0, 0, 0,
   0},
  {"protocol", OPT_DRIZZLE_PROTOCOL, "The protocol of connection (tcp,socket,pipe,memory).",
   0, 0, 0, GET_STR,  REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"quick", 'q',
   "If you are using this option with CHECK TABLE, it prevents the check from scanning the rows to check for wrong links. This is the fastest check. If you are using this option with REPAIR TABLE, it will try to repair only the index tree. This is the fastest repair method for a table.",
   (char**) &opt_quick, (char**) &opt_quick, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0,
   0},
  {"repair", 'r',
   "Can fix almost anything except unique keys that aren't unique.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"silent", 's', "Print only error messages.", (char**) &opt_silent,
   (char**) &opt_silent, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"socket", 'S', "Socket file to use for connection.",
   (char**) &opt_mysql_unix_port, (char**) &opt_mysql_unix_port, 0, GET_STR,
   REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"tables", OPT_TABLES, "Overrides option --databases (-B).", 0, 0, 0,
   GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"use-frm", OPT_FRM,
   "When used with REPAIR, get table structure from .frm file, so the table can be repaired even if .MYI header is corrupted.",
   (char**) &opt_frm, (char**) &opt_frm, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0,
   0},
#ifndef DONT_ALLOW_USER_CHANGE
  {"user", 'u', "User for login if not current user.", (char**) &current_user,
   (char**) &current_user, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
#endif
  {"verbose", 'v', "Print info about the various stages.", 0, 0, 0, GET_NO_ARG,
   NO_ARG, 0, 0, 0, 0, 0, 0},
  {"version", 'V', "Output version information and exit.", 0, 0, 0, GET_NO_ARG,
   NO_ARG, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};

static const char *load_default_groups[] = { "mysqlcheck", "client", 0 };


static void print_version(void);
static void usage(void);
static int get_options(int *argc, char ***argv);
static int process_all_databases(void);
static int process_databases(char **db_names);
static int process_selected_tables(char *db, char **table_names, int tables);
static int process_all_tables_in_db(char *database);
static int process_one_db(char *database);
static int use_db(char *database);
static int handle_request_for_tables(const char *tables, uint length);
static int dbConnect(char *host, char *user,char *passwd);
static void dbDisconnect(char *host);
static void DBerror(DRIZZLE *drizzle, const char *when);
static void safe_exit(int error);
static void print_result(void);
static uint fixed_name_length(const char *name);
static char *fix_table_name(char *dest, const char *src);
int what_to_do = 0;

static void print_version(void)
{
  printf("%s  Ver %s Distrib %s, for %s (%s)\n", my_progname, CHECK_VERSION,
   drizzle_get_client_info(), SYSTEM_TYPE, MACHINE_TYPE);
} /* print_version */

static void usage(void)
{
  print_version();
  puts("By Jani Tolonen, 2001-04-20, MySQL Development Team\n");
  puts("This software comes with ABSOLUTELY NO WARRANTY. This is free software,\n");
  puts("and you are welcome to modify and redistribute it under the GPL license.\n");
  puts("This program can be used to CHECK (-c,-m,-C), REPAIR (-r), ANALYZE (-a)");
  puts("or OPTIMIZE (-o) tables. Some of the options (like -e or -q) can be");
  puts("used at the same time. Not all options are supported by all storage engines.");
  puts("Please consult the Drizzle manual for latest information about the");
  puts("above. The options -c,-r,-a and -o are exclusive to each other, which");
  puts("means that the last option will be used, if several was specified.\n");
  puts("The option -c will be used by default, if none was specified. You");
  puts("can change the default behavior by making a symbolic link, or");
  puts("copying this file somewhere with another name, the alternatives are:");
  puts("mysqlrepair:   The default option will be -r");
  puts("mysqlanalyze:  The default option will be -a");
  puts("mysqloptimize: The default option will be -o\n");
  printf("Usage: %s [OPTIONS] database [tables]\n", my_progname);
  printf("OR     %s [OPTIONS] --databases DB1 [DB2 DB3...]\n",
   my_progname);
  printf("OR     %s [OPTIONS] --all-databases\n", my_progname);
  print_defaults("my", load_default_groups);
  my_print_help(my_long_options);
  my_print_variables(my_long_options);
} /* usage */

static bool
get_one_option(int optid, const struct my_option *opt __attribute__((unused)),
         char *argument)
{
  switch(optid) {
  case 'a':
    what_to_do = DO_ANALYZE;
    break;
  case 'c':
    what_to_do = DO_CHECK;
    break;
  case 'C':
    what_to_do = DO_CHECK;
    opt_check_only_changed = 1;
    break;
  case 'I': /* Fall through */
  case '?':
    usage();
    exit(0);
  case 'm':
    what_to_do = DO_CHECK;
    opt_medium_check = 1;
    break;
  case 'o':
    what_to_do = DO_OPTIMIZE;
    break;
  case OPT_FIX_DB_NAMES:
    what_to_do= DO_UPGRADE;
    default_charset= (char*) "utf8";
    opt_databases= 1;
    break;
  case OPT_FIX_TABLE_NAMES:
    what_to_do= DO_UPGRADE;
    default_charset= (char*) "utf8";
    break;
  case 'p':
    if (argument)
    {
      char *start = argument;
      my_free(opt_password, MYF(MY_ALLOW_ZERO_PTR));
      opt_password = my_strdup(argument, MYF(MY_FAE));
      while (*argument) *argument++= 'x';    /* Destroy argument */
      if (*start)
  start[1] = 0;                             /* Cut length of argument */
      tty_password= 0;
    }
    else
      tty_password = 1;
    break;
  case 'r':
    what_to_do = DO_REPAIR;
    break;
  case 'g':
    what_to_do= DO_CHECK;
    opt_upgrade= 1;
    break;
  case OPT_TABLES:
    opt_databases = 0;
    break;
  case 'v':
    verbose++;
    break;
  case 'V': print_version(); exit(0);
  case OPT_DRIZZLE_PROTOCOL:
    break;
  }
  return 0;
}


static int get_options(int *argc, char ***argv)
{
  int ho_error;

  if (*argc == 1)
  {
    usage();
    exit(0);
  }

  load_defaults("my", load_default_groups, argc, argv);

  if ((ho_error=handle_options(argc, argv, my_long_options, get_one_option)))
    exit(ho_error);

  if (!what_to_do)
  {
    int pnlen = strlen(my_progname);

    if (pnlen < 6) /* name too short */
      what_to_do = DO_CHECK;
    else if (!strcmp("repair", my_progname + pnlen - 6))
      what_to_do = DO_REPAIR;
    else if (!strcmp("analyze", my_progname + pnlen - 7))
      what_to_do = DO_ANALYZE;
    else if  (!strcmp("optimize", my_progname + pnlen - 8))
      what_to_do = DO_OPTIMIZE;
    else
      what_to_do = DO_CHECK;
  }

  /* TODO: This variable is not yet used */
  if (strcmp(default_charset, charset_info->csname) &&
      !(charset_info= get_charset_by_csname(default_charset,
                MY_CS_PRIMARY, MYF(MY_WME))))
      exit(1);
  if (*argc > 0 && opt_alldbs)
  {
    printf("You should give only options, no arguments at all, with option\n");
    printf("--all-databases. Please see %s --help for more information.\n",
     my_progname);
    return 1;
  }
  if (*argc < 1 && !opt_alldbs)
  {
    printf("You forgot to give the arguments! Please see %s --help\n",
     my_progname);
    printf("for more information.\n");
    return 1;
  }
  if (tty_password)
    opt_password = get_tty_password(NULL);
  if (debug_info_flag)
    my_end_arg= MY_CHECK_ERROR | MY_GIVE_INFO;
  if (debug_check_flag)
    my_end_arg= MY_CHECK_ERROR;
  return(0);
} /* get_options */


static int process_all_databases()
{
  DRIZZLE_ROW row;
  DRIZZLE_RES *tableres;
  int result = 0;

  if (drizzle_query(sock, "SHOW DATABASES") ||
      !(tableres = drizzle_store_result(sock)))
  {
    my_printf_error(0, "Error: Couldn't execute 'SHOW DATABASES': %s",
        MYF(0), drizzle_error(sock));
    return 1;
  }
  while ((row = drizzle_fetch_row(tableres)))
  {
    if (process_one_db(row[0]))
      result = 1;
  }
  return result;
}
/* process_all_databases */


static int process_databases(char **db_names)
{
  int result = 0;
  for ( ; *db_names ; db_names++)
  {
    if (process_one_db(*db_names))
      result = 1;
  }
  return result;
} /* process_databases */


static int process_selected_tables(char *db, char **table_names, int tables)
{
  if (use_db(db))
    return 1;
  if (opt_all_in_1)
  {
    /*
      We need table list in form `a`, `b`, `c`
      that's why we need 2 more chars added to to each table name
      space is for more readable output in logs and in case of error
    */   
    char *table_names_comma_sep, *end;
    int i, tot_length = 0;

    for (i = 0; i < tables; i++)
      tot_length+= fixed_name_length(*(table_names + i)) + 2;

    if (!(table_names_comma_sep = (char *)
    my_malloc((sizeof(char) * tot_length) + 4, MYF(MY_WME))))
      return 1;

    for (end = table_names_comma_sep + 1; tables > 0;
   tables--, table_names++)
    {
      end= fix_table_name(end, *table_names);
      *end++= ',';
    }
    *--end = 0;
    handle_request_for_tables(table_names_comma_sep + 1, tot_length - 1);
    my_free(table_names_comma_sep, MYF(0));
  }
  else
    for (; tables > 0; tables--, table_names++)
      handle_request_for_tables(*table_names, fixed_name_length(*table_names));
  return 0;
} /* process_selected_tables */


static uint fixed_name_length(const char *name)
{
  const char *p;
  uint extra_length= 2;  /* count the first/last backticks */
 
  for (p= name; *p; p++)
  {
    if (*p == '`')
      extra_length++;
    else if (*p == '.')
      extra_length+= 2;
  }
  return (p - name) + extra_length;
}


static char *fix_table_name(char *dest, const char *src)
{
  *dest++= '`';
  for (; *src; src++)
  {
    switch (*src) {
    case '.':            /* add backticks around '.' */
      *dest++= '`';
      *dest++= '.';
      *dest++= '`';
      break;
    case '`':            /* escape backtick character */
      *dest++= '`';
      /* fall through */
    default:
      *dest++= *src;
    }
  }
  *dest++= '`';
  return dest;
}


static int process_all_tables_in_db(char *database)
{
  DRIZZLE_RES *res;
  DRIZZLE_ROW row;
  uint num_columns;

  if (use_db(database))
    return 1;
  if (drizzle_query(sock, "SHOW /*!50002 FULL*/ TABLES") ||
  !((res= drizzle_store_result(sock))))
    return 1;

  num_columns= drizzle_num_fields(res);

  if (opt_all_in_1)
  {
    /*
      We need table list in form `a`, `b`, `c`
      that's why we need 2 more chars added to to each table name
      space is for more readable output in logs and in case of error
     */

    char *tables, *end;
    uint tot_length = 0;

    while ((row = drizzle_fetch_row(res)))
      tot_length+= fixed_name_length(row[0]) + 2;
    drizzle_data_seek(res, 0);

    if (!(tables=(char *) my_malloc(sizeof(char)*tot_length+4, MYF(MY_WME))))
    {
      drizzle_free_result(res);
      return 1;
    }
    for (end = tables + 1; (row = drizzle_fetch_row(res)) ;)
    {
      if ((num_columns == 2) && (strcmp(row[1], "VIEW") == 0))
        continue;

      end= fix_table_name(end, row[0]);
      *end++= ',';
    }
    *--end = 0;
    if (tot_length)
      handle_request_for_tables(tables + 1, tot_length - 1);
    my_free(tables, MYF(0));
  }
  else
  {
    while ((row = drizzle_fetch_row(res)))
    {
      /* Skip views if we don't perform renaming. */
      if ((what_to_do != DO_UPGRADE) && (num_columns == 2) && (strcmp(row[1], "VIEW") == 0))
        continue;

      handle_request_for_tables(row[0], fixed_name_length(row[0]));
    }
  }
  drizzle_free_result(res);
  return 0;
} /* process_all_tables_in_db */



static int fix_table_storage_name(const char *name)
{
  char qbuf[100 + NAME_LEN*4];
  int rc= 0;
  if (strncmp(name, "#mysql50#", 9))
    return 1;
  sprintf(qbuf, "RENAME TABLE `%s` TO `%s`", name, name + 9);
  if (drizzle_query(sock, qbuf))
  {
    fprintf(stderr, "Failed to %s\n", qbuf);
    fprintf(stderr, "Error: %s\n", drizzle_error(sock));
    rc= 1;
  }
  if (verbose)
    printf("%-50s %s\n", name, rc ? "FAILED" : "OK");
  return rc;
}

static int fix_database_storage_name(const char *name)
{
  char qbuf[100 + NAME_LEN*4];
  int rc= 0;
  if (strncmp(name, "#mysql50#", 9))
    return 1;
  sprintf(qbuf, "ALTER DATABASE `%s` UPGRADE DATA DIRECTORY NAME", name);
  if (drizzle_query(sock, qbuf))
  {
    fprintf(stderr, "Failed to %s\n", qbuf);
    fprintf(stderr, "Error: %s\n", drizzle_error(sock));
    rc= 1;
  }
  if (verbose)
    printf("%-50s %s\n", name, rc ? "FAILED" : "OK");
  return rc;
}

static int process_one_db(char *database)
{
  if (what_to_do == DO_UPGRADE)
  {
    int rc= 0;
    if (opt_fix_db_names && !strncmp(database,"#mysql50#", 9))
    {
      rc= fix_database_storage_name(database);
      database+= 9;
    }
    if (rc || !opt_fix_table_names)
      return rc;
  }
  return process_all_tables_in_db(database);
}


static int use_db(char *database)
{
  if (drizzle_get_server_version(sock) >= 50003 &&
      !my_strcasecmp(&my_charset_utf8_general_ci, database, "information_schema"))
    return 1;
  if (drizzle_select_db(sock, database))
  {
    DBerror(sock, "when selecting the database");
    return 1;
  }
  return 0;
} /* use_db */


static int handle_request_for_tables(const char *tables, uint length)
{
  char *query, *end, options[100], message[100];
  uint query_length= 0;
  const char *op = 0;

  options[0] = 0;
  end = options;
  switch (what_to_do) {
  case DO_CHECK:
    op = "CHECK";
    if (opt_quick)              end = my_stpcpy(end, " QUICK");
    if (opt_fast)               end = my_stpcpy(end, " FAST");
    if (opt_medium_check)       end = my_stpcpy(end, " MEDIUM"); /* Default */
    if (opt_extended)           end = my_stpcpy(end, " EXTENDED");
    if (opt_check_only_changed) end = my_stpcpy(end, " CHANGED");
    if (opt_upgrade)            end = my_stpcpy(end, " FOR UPGRADE");
    break;
  case DO_REPAIR:
    op= (opt_write_binlog) ? "REPAIR" : "REPAIR NO_WRITE_TO_BINLOG";
    if (opt_quick)              end = my_stpcpy(end, " QUICK");
    if (opt_extended)           end = my_stpcpy(end, " EXTENDED");
    if (opt_frm)                end = my_stpcpy(end, " USE_FRM");
    break;
  case DO_ANALYZE:
    op= (opt_write_binlog) ? "ANALYZE" : "ANALYZE NO_WRITE_TO_BINLOG";
    break;
  case DO_OPTIMIZE:
    op= (opt_write_binlog) ? "OPTIMIZE" : "OPTIMIZE NO_WRITE_TO_BINLOG";
    break;
  case DO_UPGRADE:
    return fix_table_storage_name(tables);
  }

  if (!(query =(char *) my_malloc((sizeof(char)*(length+110)), MYF(MY_WME))))
    return 1;
  if (opt_all_in_1)
  {
    /* No backticks here as we added them before */
    query_length= sprintf(query, "%s TABLE %s %s", op, tables, options);
  }
  else
  {
    char *ptr;

    ptr= my_stpcpy(my_stpcpy(query, op), " TABLE ");
    ptr= fix_table_name(ptr, tables);
    ptr= strxmov(ptr, " ", options, NULL);
    query_length= (uint) (ptr - query);
  }
  if (drizzle_real_query(sock, query, query_length))
  {
    sprintf(message, "when executing '%s TABLE ... %s'", op, options);
    DBerror(sock, message);
    return 1;
  }
  print_result();
  my_free(query, MYF(0));
  return 0;
}


static void print_result()
{
  DRIZZLE_RES *res;
  DRIZZLE_ROW row;
  char prev[NAME_LEN*2+2];
  uint i;
  bool found_error=0;

  res = drizzle_use_result(sock);

  prev[0] = '\0';
  for (i = 0; (row = drizzle_fetch_row(res)); i++)
  {
    int changed = strcmp(prev, row[0]);
    bool status = !strcmp(row[2], "status");

    if (status)
    {
      /*
        if there was an error with the table, we have --auto-repair set,
        and this isn't a repair op, then add the table to the tables4repair
        list
      */
      if (found_error && opt_auto_repair && what_to_do != DO_REPAIR &&
          strcmp(row[3],"OK"))
        tables4repair.push_back(string(prev));
      found_error=0;
      if (opt_silent)
  continue;
    }
    if (status && changed)
      printf("%-50s %s", row[0], row[3]);
    else if (!status && changed)
    {
      printf("%s\n%-9s: %s", row[0], row[2], row[3]);
      if (strcmp(row[2],"note"))
  found_error=1;
    }
    else
      printf("%-9s: %s", row[2], row[3]);
    my_stpcpy(prev, row[0]);
    putchar('\n');
  }
  /* add the last table to be repaired to the list */
  if (found_error && opt_auto_repair && what_to_do != DO_REPAIR)
    tables4repair.push_back(string(prev));
  drizzle_free_result(res);
}


static int dbConnect(char *host, char *user, char *passwd)
{

  if (verbose)
  {
    fprintf(stderr, "# Connecting to %s...\n", host ? host : "localhost");
  }
  drizzle_create(&drizzle_connection);
  if (opt_compress)
    drizzle_options(&drizzle_connection, DRIZZLE_OPT_COMPRESS, NULL);
  if (opt_protocol)
    drizzle_options(&drizzle_connection,DRIZZLE_OPT_PROTOCOL,(char*)&opt_protocol);
  if (!(sock = drizzle_connect(&drizzle_connection, host, user, passwd,
         NULL, opt_mysql_port, opt_mysql_unix_port, 0)))
  {
    DBerror(&drizzle_connection, "when trying to connect");
    return 1;
  }
  drizzle_connection.reconnect= 1;
  return 0;
} /* dbConnect */


static void dbDisconnect(char *host)
{
  if (verbose)
    fprintf(stderr, "# Disconnecting from %s...\n", host ? host : "localhost");
  drizzle_close(sock);
} /* dbDisconnect */


static void DBerror(DRIZZLE *drizzle, const char *when)
{
  my_printf_error(0,"Got error: %d: %s %s", MYF(0),
      drizzle_errno(drizzle), drizzle_error(drizzle), when);
  safe_exit(EX_MYSQLERR);
  return;
} /* DBerror */


static void safe_exit(int error)
{
  if (!first_error)
    first_error= error;
  if (ignore_errors)
    return;
  if (sock)
    drizzle_close(sock);
  exit(error);
}


int main(int argc, char **argv)
{
  MY_INIT(argv[0]);
  /*
  ** Check out the args
  */
  if (get_options(&argc, &argv))
  {
    my_end(my_end_arg);
    exit(EX_USAGE);
  }
  if (dbConnect(current_host, current_user, opt_password))
    exit(EX_MYSQLERR);

  if (opt_auto_repair)
  {
    tables4repair.reserve(64);
    if (tables4repair.capacity() == 0)
    {
      first_error = 1;
      goto end;
    }
  }


  if (opt_alldbs)
    process_all_databases();
  /* Only one database and selected table(s) */
  else if (argc > 1 && !opt_databases)
    process_selected_tables(*argv, (argv + 1), (argc - 1));
  /* One or more databases, all tables */
  else
    process_databases(argv);
  if (opt_auto_repair)
  {

    if (!opt_silent && (tables4repair.size() > 0))
      puts("\nRepairing tables");
    what_to_do = DO_REPAIR;
    vector<string>::iterator i;
    for ( i= tables4repair.begin() ; i < tables4repair.end() ; i++)
    {
      const char *name= (*i).c_str();
      handle_request_for_tables(name, fixed_name_length(name));
    }
  }
 end:
  dbDisconnect(current_host);
  my_free(opt_password, MYF(MY_ALLOW_ZERO_PTR));
  my_end(my_end_arg);
  return(first_error!=0);
} /* main */
