/* get rid of stupid undefined errors for (v)asprintf */
#define _GNU_SOURCE

#ifndef HAVE_SLIST_H
#define slist list
#define SLIST_H <list.h>
#else
#define SLIST_H <slist.h>
#endif

#ifndef HAVE_LIBZ
#undef CHECKSUM
#endif

/*#ifdef CHECKSUM
#define CACHE_USE_CHECKSUM
#else
#undef SHORT_TEMPNAMES
#endif*/
