/*
    misc.* - misc functions
    Copyright (C) 1999  Matthew Mueller <donut@azstarnet.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef _MISC_H_
#define _MISC_H_
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
//misc.h

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef HAVE_ATOUL
inline ulong atoul(const char *str){return strtoul(str,NULL,10);}
#endif

#ifndef HAVE_STRERROR
const char * strerror(int err);
#endif
void init_my_timezone(void);

int doopen(int &handle,const char * name,int access,int mode=0);
int dofopen(FILE * &f,const char * name,const char * mode,int domiscquiet=0);

int dobackup(const char * name);

int fexists(const char * f);
int testmkdir(const char * dir,int mode);

const char * getfname(const char * src);

int do_utime(const char *f,time_t t);

size_t tconv(char * timestr, int max, time_t *curtime,const char * formatstr="%m-%d-%y %H:%M", int local=1);


char * goodstrtok(char **cur, char sep);

int is_text(const char * f);

time_t decode_mdtm(const char * cbuf);
time_t decode_textdate(const char * cbuf);
int decode_textmonth(const char * buf);
int decode_texttz(const char * buf);

void setint0 (int *i);

#ifdef	USE_FILECOMPARE					// check for duplicate files
int filecompare(const char *old_fn,const char *nfn);
#endif

#endif
