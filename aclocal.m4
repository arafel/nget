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

AC_DEFUN(MY_DECL_TIMEZONE,
[AC_CACHE_CHECK([for timezone declaration in time.h],
  ac_cv_decl_timezone,
[AC_TRY_COMPILE([#include <time.h>], [long l = 1l-timezone;],
  ac_cv_decl_timezone=yes, ac_cv_decl_timezone=no)])
if test $ac_cv_decl_timezone = yes; then
  AC_DEFINE(TIMEZONE_IS_VAR)
fi
])
      
