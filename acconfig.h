/* Define if you have the uu library (-luu).  */
#undef HAVE_LIBUU

/* check consistancy */
#undef DEBUG_CACHE

/* use text mode cache */
#undef TEXT_CACHE

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef socklen_t

/* define if `timezone' is an integer var defined in time.h */
#undef TIMEZONE_IS_VAR

@BOTTOM@

/* get rid of stupid undefined errors for (v)asprintf */
#define _GNU_SOURCE

#ifndef HAVE_SLIST_H
#define slist list
#endif
