#include <string.h>
#define errtoa strerror
#define PERROR(a, args...) fprintf(stderr,a "\n" , ## args)
#define PMSG(a, args...) printf(a , ## args)
//#define CURTIME curtime
//extern time_t curtime;
