/* get rid of stupid undefined errors for (v)asprintf */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef HAVE_SLIST_H
#define slist list
#define SLIST_H <list.h>
#else
#define SLIST_H <slist.h>
#endif

#ifndef HAVE_LIBZ
#undef CHECKSUM
#endif

#include <assert.h>

#ifdef POPT_CONST_ARGV
#define POPT_ARGV_T (const char **)
#define POPT_ARGV_p_T (const char ***)
#else
#define POPT_ARGV_T
#define POPT_ARGV_p_T
#endif
/*#ifdef CHECKSUM
#define CACHE_USE_CHECKSUM
#else
#undef SHORT_TEMPNAMES
#endif*/

using namespace std;

#ifdef HAVE_INTTYPES_H
# ifndef __STDC_FORMAT_MACROS
# define __STDC_FORMAT_MACROS
# endif
# include <inttypes.h>
#endif
#ifndef HAVE_INT_FAST64_T
# ifdef HAVE_LONG_LONG
#  define int_fast64_t long long
#  define uint_fast64_t unsigned long long
# else
//well, they may not be 64 bits but at least it should still work.
#  define int_fast64_t long
#  define uint_fast64_t unsigned long
# endif
#endif
#ifndef PRIuFAST64
# ifdef HAVE_LONG_LONG
#  define PRIuFAST64 "llu"
# else
#  define PRIuFAST64 "lu"
# endif
#endif
