#ifndef _LOG_H_
#define _LOG_H_
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
//#include <string.h>
#define PERROR(a, args...) fprintf(stderr,a "\n" , ## args)
#define PMSG(a, args...) printf(a "\n" , ## args)
//#define PDEBUG(a, args...) printf(a , ## args)
#define PDEBUG(d, a, args...) {if (debug>=d) printf(a "\n" , ## args);}
//#define CURTIME curtime
//extern time_t curtime;
extern int debug;
#define DEBUG_MIN 1
#define DEBUG_MED 2
#define DEBUG_ALL 3
extern int quiet;
//transport level errors
#define EX_T_FATAL 5
#define EX_T_ERROR 2
//#define EX_T_NOCON 1
//protocol errors
#define EX_P_FATAL 3
//user errors
#define EX_U_FATAL 4
#define EX_U_WARN 1
//application errors
#define EX_A_FATAL 6

#define EX_FATALS 3

class c_error {
	public:
		int num;
		char* str;
		c_error(int n, const char * s, ...);
		~c_error();
};
#endif
