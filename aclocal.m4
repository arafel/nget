AC_DEFUN(SOCK_CHECK_TYPE,
[AC_REQUIRE([AC_HEADER_STDC])dnl
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(ac_cv_type_$1,
[AC_EGREP_CPP(dnl
changequote(<<,>>)dnl
<<$1[^a-zA-Z_0-9]>>dnl
changequote([,]), [#include <sys/types.h>
#include <sys/socket.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif], ac_cv_type_$1=yes, ac_cv_type_$1=no)])dnl
AC_MSG_RESULT($ac_cv_type_$1)
if test $ac_cv_type_$1 = no; then
  AC_DEFINE($1, $2)
fi
])

AC_DEFUN(MY_DECL__TIMEZONE,
[AC_CACHE_CHECK([for _timezone declaration in time.h],
  ac_cv_decl__timezone,
[AC_TRY_COMPILE([#include <time.h>], [long l = 1l-_timezone;],
  ac_cv_decl__timezone=yes, ac_cv_decl__timezone=no)])
if test $ac_cv_decl__timezone = yes; then
  AC_DEFINE(_TIMEZONE_IS_VAR)
fi
])
AC_DEFUN(MY_DECL_TIMEZONE,
[AC_CACHE_CHECK([for timezone declaration in time.h],
  ac_cv_decl_timezone,
[AC_TRY_COMPILE([#include <time.h>], [long l = 1l-timezone;],
  ac_cv_decl_timezone=yes, ac_cv_decl_timezone=no)])
if test $ac_cv_decl_timezone = yes; then
  AC_DEFINE(TIMEZONE_IS_VAR)
fi
])


dnl AC_TRY_COMPILE(INCLUDES, FUNCTION-BODY,
dnl             [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN(MY_CHECK_EXCEPTIONS,
 [
  AC_MSG_CHECKING(for exception handling support)
  AC_CACHE_VAL(my_cv_exceptions,[
   AC_TRY_COMPILE([#include <stdlib.h>], [try { throw 1; } catch (int i) {int z=i;}], my_cv_exceptions=yes)
   if test -z $my_cv_exceptions; then
     CXXFLAGS="$CXXFLAGS -fhandle-exceptions"
     AC_TRY_COMPILE([#include <stdlib.h>], [try { throw 1; } catch (int i) {int z=i;}], my_cv_exceptions=yes, my_cv_exceptions=no)
   fi
  ])
  AC_MSG_RESULT($my_cv_exceptions)
  if test "x$my_cv_exceptions" = xno; then
   AC_ERROR(exceptions not supported by your compiler,
if you need a special flag try CXXFLAGS="-flag-to-enable-exceptions" ./configure)
  fi
 ]
)

AC_DEFUN(MY_CHECK_OPTIMIZATION,
 [
  AC_MSG_CHECKING(for optimization problems)
  AC_CACHE_VAL(my_cv_optimization_problems,[
   my_cv_optimization_problems=no
   opt_prob_why=", should be ok, if not report the output of $CXX -v"
   ac_try="$CXX -v 2> conftest.out"
dnl   AC_MSG_RESULT(try: $ac_try)
   AC_TRY_EVAL(ac_try)
 dnl  AC_MSG_RESULT(blah)
 dnl  cat conftest.out
 dnl  AC_MSG_RESULT(blah1)
   changequote(, )dnl
   exp="s/^[^0-9]*\([0-9]*\.[0-9]*\).*/\1/p"
   gcc_ver=`grep "gcc version" conftest.out | sed -n $exp`
 dnl  AC_MSG_RESULT(exp: $exp)
   changequote([, ])dnl
 dnl  AC_MSG_RESULT(ver: $gcc_ver)
   if test -n '$gcc_ver'; then
    gcc_major=`echo $gcc_ver | sed "s/\..*//"`
    gcc_minor=`echo $gcc_ver | sed "s/.*\.//"`
    if test -n '$gcc_major' -a -n '$gcc_minor'; then
 dnl   AC_MSG_RESULT(maj: $gcc_major min: $gcc_minor)
     if test 0$gcc_major -le 2 -a 0$gcc_minor -le 7; then
      my_cv_optimization_problems=yes
	  MY_DISABLE_OPT
      opt_prob_why=", gcc <= 2.7.x detected. optimization disabled"
     fi
    fi
   fi
  ])
  AC_MSG_RESULT([$my_cv_optimization_problems$opt_prob_why])
 ]
)

AC_DEFUN(MY_DISABLE_OPT,[
 changequote(, )dnl
 CXXFLAGS=`echo $CXXFLAGS | sed "s/-O[1-9] *//"`
 changequote([, ])dnl
])
