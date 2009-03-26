AC_DEFUN([DRIZZLE_FUNC_ALLOCA],
[
# Since we have heard that alloca fails on IRIX never define it on a
# SGI machine
if test ! "$host_vendor" = "sgi"
then
 AC_REQUIRE_CPP()dnl Set CPP; we run AC_EGREP_CPP conditionally.
 # The Ultrix 4.2 mips builtin alloca declared by alloca.h only works
 # for constant arguments.  Useless!
 AC_CACHE_CHECK([for working alloca.h], ac_cv_header_alloca_h,
 [AC_TRY_LINK([#include <alloca.h>], [char *p = alloca(2 * sizeof(int));],
   ac_cv_header_alloca_h=yes, ac_cv_header_alloca_h=no)])
 if test "$ac_cv_header_alloca_h" = "yes"
 then
	AC_DEFINE(HAVE_ALLOCA, 1)
 fi
 
 AC_CACHE_CHECK([for alloca], ac_cv_func_alloca_works,
 [AC_TRY_LINK([
 #ifdef __GNUC__
 # define alloca __builtin_alloca
 #else
 # if HAVE_ALLOCA_H
 #  include <alloca.h>
 # else
 #  ifdef _AIX
  #pragma alloca
 #  else
 #   ifndef alloca /* predefined by HP cc +Olibcalls */
 char *alloca ();
 #   endif
 #  endif
 # endif
 #endif
 ], [char *p = (char *) alloca(1);],
   ac_cv_func_alloca_works=yes, ac_cv_func_alloca_works=no)])
 if test "$ac_cv_func_alloca_works" = "yes"; then
   AC_DEFINE([HAVE_ALLOCA], [1], [If we have a working alloca() implementation])
 fi
 
 if test "$ac_cv_func_alloca_works" = "no"; then
   # The SVR3 libPW and SVR4 libucb both contain incompatible functions
   # that cause trouble.  Some versions do not even contain alloca or
   # contain a buggy version.  If you still want to use their alloca,
   # use ar to extract alloca.o from them instead of compiling alloca.c.
   ALLOCA=alloca.o
   AC_DEFINE(C_ALLOCA, 1)
 
 fi
 AC_SUBST(ALLOCA)dnl
else
 AC_MSG_RESULT("Skipped alloca tests")
fi
])