//misc.h

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int doopen(int &handle,const char * name,int access,int mode=0);
int dofopen(FILE * &f,const char * name,const char * mode,int domiscquiet=0);

int dobackup(const char * name);

const char * getfname(const char * src);

int do_utime(const char *f,time_t t);

size_t tconv(char * timestr, int max, time_t *curtime,const char * formatstr="%m-%d-%y %H:%M", int local=1);

const char * errtoa(int err);

char * goodstrtok(char **cur, char sep);

int is_text(const char * f);

time_t decode_mdtm(const char * cbuf);
time_t decode_textdate(const char * cbuf);
int decode_textmonth(const char * buf);
int decode_texttz(const char * buf);

void setint0 (int *i);
