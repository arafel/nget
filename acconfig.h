/* Define if you have the uu library (-luu).  */
#undef HAVE_LIBUU

/* check consistancy */
#undef DEBUG_CACHE

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef socklen_t

/* Define to `unsigned long' if <sys/types.h> doesn't define.  */
#undef ulong

/* define if `timezone' is an integer var defined in time.h */
#undef TIMEZONE_IS_VAR

/* use shorter filenames for tempfiles */
#undef SHORT_TEMPNAMES

/* checksum method to use */
#undef CHECKSUM

/* compare duplicate files? */
#undef USE_FILECOMPARE

@BOTTOM@

/* tests for some features that depend on other features */
#include "_subconf.h"
