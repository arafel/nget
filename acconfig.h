/* Define if you have the uu library (-luu).  */
#undef HAVE_LIBUU

/* check consistancy */
#undef DEBUG_CACHE

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef socklen_t

/* Define to `unsigned long' if <sys/types.h> doesn't define.  */
#undef ulong

/* Define to `unsigned char' if <sys/types.h> doesn't define.  */
#undef uchar

/* define if `timezone' is an integer var defined in time.h */
#undef TIMEZONE_IS_VAR

/* define if `_timezone' is an integer var defined in time.h */
#undef _TIMEZONE_IS_VAR

/* checksum method to use */
#undef CHECKSUM

/* compare duplicate files? */
#undef USE_FILECOMPARE

/* debug mode? */
#undef NDEBUG

@BOTTOM@

/* tests for some features that depend on other features */
#include "_subconf.h"
