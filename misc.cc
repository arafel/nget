#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#ifdef WRFTP
#include "main.h"
#endif
#include "misc.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>
#include <utime.h>
#include <errno.h>
#include "myregex.h"

#ifndef HAVE_LOCALTIME_R
struct tm * localtime_r(const time_t *t,struct tm * tmu){
	struct tm *t1=localtime(t);
	if (!t1) return NULL;
	memcpy(tmu,t1,sizeof(struct tm));
	return tmu;
}
#endif
#ifdef TIMEZONE_IS_VAR 
#define my_timezone timezone
void init_my_timezone(void){
//	printf("my_timezone=%li\n",my_timezone);
}
#else
long my_timezone=0;
void init_my_timezone(void){
	struct tm tm;
	time_t t;
	time(&t);
	localtime_r(&t,&tm);
	my_timezone=tm.tm_gmtoff;
//	printf("my_timezone=%li\n",my_timezone);
}
#endif

int doopen(int &handle,const char * name,int access,int mode=0) {
   if ((handle=open(name,access,mode))==-1){
//      if (domiscquiet)
//      domiscquiet--;
//      else
      PERROR("Error opening %s: %s",name,strerror(errno));
      return -1;
   }
   else return 0;
}

int dofopen(FILE * &f,const char * name,const char * mode,int domiscquiet=0) {
//   FILE *f;
   if ((f=fopen(name,mode))==NULL){
      if (!domiscquiet)
        PERROR("Error opening %s: %s",name,strerror(errno));
      return -1;
   }
   return 0;
}

int dobackup(const char * name){
	//###### TODO: use crope/basic_string?
//   char buf[FTP_PATH_LEN];
   char buf[256];
   strcpy(buf,name);
   strcat(buf,"~");
   return rename(name,buf);
}

const char * getfname(const char * src){
   int i=strlen(src)-1;
   if (i<0) i=0;
   while ((i>0)&&(src[i-1]!='/'))
     i--;
   return &src[i];
}


int fexists(const char * f){
	struct stat statbuf;
	return (!stat(f,&statbuf));
	
}
int testmkdir(const char * dir,int mode){
	struct stat statbuf;
	if (stat(dir,&statbuf)){
		if (mkdir(dir,mode)){
			return -1;
		}
	} else if (!S_ISDIR(statbuf.st_mode)){
		return -2;
	}
	return 0;
}																	

int do_utime(const char *f,time_t t){
	struct utimbuf buf;
	buf.actime=t;
	buf.modtime=t;
	return utime(f,&buf);
}



size_t tconv(char * timestr, int max, time_t *curtime,const char * formatstr="%m-%d-%y %H:%M", int local=1) {
//	static char timestr[80];
	struct tm *time_now;
	if (local)
	     time_now = localtime(curtime);
	else
	     time_now = gmtime(curtime);
	return strftime(timestr,max,formatstr,time_now);
//	return timestr;
}

#ifndef HAVE_STRERROR
//extern int _sys_nerr;
//extern const char *const _sys_errlist[];
const char * strerror(int err){
//	if (err>_sys_nerr)
//	     return "max errno exceeded";
//	else if (err<0)
//	     return "min errno exceeded";
//	else
//	     return _sys_errlist[err];	
	static char buf[5];
	sprintf(buf,"%i",err);
	return buf;
}
#endif 
// threadsafe.
char * goodstrtok(char **cur, char sep){
//   static char * cur=NULL;
   char * tmp, *old;
//   if (buf)
//     cur=buf;
   if (!*cur) return NULL;
   old=*cur;
   if ((tmp=strchr(*cur,sep))==NULL){
      *cur=NULL;
      return old;
   }
   tmp[0]=0;
   *cur=tmp+1;
   return old;
}

char txt_exts[]=".txt.cpp.c.cc.h.pas.htm.html.pl.tcl";
int is_text(const char * f){
      char *t,*s;
      t=strrchr(f,'.');
      if (t==NULL)return 0;
//   if ((s=strstr(cfg.txt_exts,t))){
	if ((s=strstr(txt_exts,t))){
		switch (s[strlen(t)]){
		  case '.':case 0:
			return 1;
		}
	}
	return 0;
}

//MDTM INDEX.txt
//213 19951006235406
time_t decode_mdtm(const char * cbuf){
   char buf[5];
   struct tm tblock;
   memset(&tblock,0,sizeof(struct tm));

   buf[4]=0;
   strncpy(buf,cbuf,4);
   tblock.tm_year=atoi(buf)-1900;

   buf[2]=0;
   strncpy(buf,cbuf+4,2);
   tblock.tm_mon=atoi(buf)-1;

   strncpy(buf,cbuf+6,2);
   tblock.tm_mday=atoi(buf);

   strncpy(buf,cbuf+8,2);
   tblock.tm_hour=atoi(buf);

   strncpy(buf,cbuf+10,2);
   tblock.tm_min=atoi(buf);

   strncpy(buf,cbuf+12,2);
   tblock.tm_sec=atoi(buf);

   return mktime(&tblock)-my_timezone;
}

char *text_month[13]={"Jan", "Feb", "Mar", "Apr",
	"May", "Jun", "Jul", "Aug",
	"Sep", "Oct", "Nov", "Dec"
};

int decode_textmonth(const char * buf){
	for (int i=0;i<12;i++){
		if (!strncasecmp(text_month[i],buf,3))
		     return i;
	}
	return -1;
}
int decode_texttz(const char * buf){
	int i=0;
	if (*buf=='-' || *buf=='+'){
		i=atoi(buf+1);
		i=((i/100)*60+(i%100))*60;
		if (*buf=='-')
			return -i;
	}
	return i;
}

//Tue, 25 May 1999 06:23:23 GMT

//Last-modified: Friday, 13-Nov-98 20:41:28 GMT
//012345678901234567890123456789
//Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
//23 May 1999 22:46:41 GMT ; no textual day
//25 May 99 01:01:48 +0100 ; 2 digit year
//Mon, 24 May 99 11:53:47 GMT ; 2 digit year
//3 Jun 1999 12:35:14 -0500 ; non padded day. blah.
//Tue, 1 Jun 1999 20:36:29 +0100 ; blah again
//Sun, 23 May 1999 19:34:35 -0500 ; ack, timezone

//Sunday,
// 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
//Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format (note, this is used by ls -l --full-time)
//Jan  4 17:11 			;ls -l format, 17:11 may be replaced by " 1998"
//c_regex rfc1123("^[A-Za-z, ]*([0-9]{1,2}) (...) ([0-9]{2,4}) ([0-9]{1,2}):([0-9]{2}):([0-9]{2}) (.*)$"),
//	rfc850("^[A-Za-z, ]*([0-9]{1,2})-(...)-([0-9]{2,4}) ([0-9]{1,2}):([0-9]{2}):([0-9]{2}) (.*)$"),
c_regex xrfc("^[A-Za-z, ]*([0-9]{1,2})[- ](...)[- ]([0-9]{2,4}) ([0-9]{1,2}):([0-9]{2}):([0-9]{2}) (.*)$"),
	xasctime("^[A-Za-z, ]*(...) +([0-9]{1,2}) ([0-9]{1,2}):([0-9]{2}):([0-9]{2}) ([0-9]{2,4}) *(.*)$"),
	xlsl("^(...) +([0-9]{1,2}) +([0-9:]{4-5})$")
		;
time_t decode_textdate(const char * cbuf){
	struct tm tblock;
	memset(&tblock,0,sizeof(struct tm));
	char *tdt=NULL;
	int td_tz=0;
	if (!xrfc.match(cbuf)){
		tdt="xrfc*-date";
		tblock.tm_mday=atoi(xrfc.subs(1));
		tblock.tm_mon=decode_textmonth(xrfc.subs(2));
		tblock.tm_year=atoi(xrfc.subs(3));
		if(xrfc.sublen(3)>=4)
			tblock.tm_year-=1900;
		tblock.tm_hour=atoi(xrfc.subs(4));
		tblock.tm_min=atoi(xrfc.subs(5));
		tblock.tm_sec=atoi(xrfc.subs(6));
		td_tz=decode_texttz(xrfc.subs(7));
		xrfc.freesub();
	}else if (!xasctime.match(cbuf)){
		tdt="asctime-date";
		tblock.tm_mon=decode_textmonth(xasctime.subs(1));
			tblock.tm_mday=atoi(xasctime.subs(2));
		tblock.tm_hour=atoi(xasctime.subs(3));
		tblock.tm_min=atoi(xasctime.subs(4));
		tblock.tm_sec=atoi(xasctime.subs(5));
		tblock.tm_year=atoi(xasctime.subs(6));
		if(xasctime.sublen(6)>=4)
			tblock.tm_year-=1900;
		xasctime.freesub();
	}else if (!xlsl.match(cbuf)){
		tdt="ls-l-date";
		tblock.tm_mon=decode_textmonth(xlsl.subs(1));
		tblock.tm_mday=atoi(xlsl.subs(2));
		if (xlsl.subs(3)[2]==':'){
			struct tm lt;
#ifdef CURTIME
            localtime_r(&CURTIME,&lt);
#else
			time_t curtime;
			time(&curtime);
            localtime_r(&curtime,&lt);
#endif
			tblock.tm_hour=atoi(xlsl.subs(3));
			tblock.tm_min=atoi(xlsl.subs(3)+3);

			if (lt.tm_mon>=tblock.tm_mon)
			     tblock.tm_year=lt.tm_year;
			else
			     tblock.tm_year=lt.tm_year-1;
		}else{
			tblock.tm_year=atoi(xlsl.subs(3))-1900;
		}
	}
	if(!tdt){
		PERROR("decode_textdate: unknown %s",cbuf);
		return 0;
	}else
		PDEBUG("decode_textdate: %s %i %i %i %i %i %i %i",tdt,tblock.tm_year,tblock.tm_mon,tblock.tm_mday,tblock.tm_hour,tblock.tm_min,tblock.tm_sec,td_tz);
	return mktime(&tblock)-my_timezone-td_tz;
}
#ifdef NO_REGEXPS
time_t decode_textdate(const char * cbuf){
	int len=strlen(cbuf);
	struct tm tblock;
	memset(&tblock,0,sizeof(struct tm));
	char *tdt=NULL;
	int td_tz=0;
	//      PERROR("decode_textdate(%s)",cbuf);
	if (len==12 && cbuf[3]==' ' && cbuf[6]==' '){
		tdt="ls-l-date";
		tblock.tm_mon=decode_textmonth(cbuf);
		if (cbuf[4]==' ')
			tblock.tm_mday=atoi(cbuf+5);
		else
		     tblock.tm_mday=atoi(cbuf+4);
		if (cbuf[9]==':'){
			struct tm lt;
			
#ifdef CURTIME
            localtime_r(&CURTIME,&lt);
#else
			time_t curtime;
			time(&curtime);
            localtime_r(&curtime,&lt);
#endif
			tblock.tm_hour=atoi(cbuf+7);
			tblock.tm_min=atoi(cbuf+10);

			if (lt.tm_mon>=tblock.tm_mon)
			     tblock.tm_year=lt.tm_year;
			else
			     tblock.tm_year=lt.tm_year-1;
		}else{
			tblock.tm_year=atoi(cbuf+8)-1900;
		}
	}
	else if ((len>28 && cbuf[3]==',' && cbuf[19]==':' && cbuf[22]==':')||
			(len>27 && cbuf[3]==',' && cbuf[18]==':' && cbuf[21]==':')||
			(len>23 && cbuf[2]==' ' && cbuf[14]==':' && cbuf[17]==':') ||
			(len>22 && cbuf[1]==' ' && cbuf[13]==':' && cbuf[16]==':')){
		//		PERROR("decode_textdate:  rfc1123-date");
		tdt="rfc1123-date";
		int ofs,nopad=0;
		if (cbuf[1]==' '){
			ofs=-1;nopad=1;
		}else if (cbuf[2]==' ')
			ofs=0;
		else if (cbuf[18]==':') {
			ofs=4;nopad=1;
		}else
			ofs=5;
		tblock.tm_mday=atoi(cbuf+ofs+nopad);
		tblock.tm_mon=decode_textmonth(cbuf+3+ofs);
		tblock.tm_year=atoi(cbuf+7+ofs)-1900;
		tblock.tm_hour=atoi(cbuf+12+ofs);
		tblock.tm_min=atoi(cbuf+15+ofs);
		tblock.tm_sec=atoi(cbuf+18+ofs);
		td_tz=decode_texttz(cbuf+21+ofs);
	}
	else if (len>23 && cbuf[3]==' ' && cbuf[13]==':' && cbuf[16]==':'){
//			PERROR("decode_textdate:  asctime-date");
		tdt="asctime-date";
		if (len>=24){
			tblock.tm_mon=decode_textmonth(cbuf+4);
			if (cbuf[8]==' ')
			     tblock.tm_mday=atoi(cbuf+9);
			else
			     tblock.tm_mday=atoi(cbuf+8);
			tblock.tm_hour=atoi(cbuf+11);
			tblock.tm_min=atoi(cbuf+14);
			tblock.tm_sec=atoi(cbuf+17);
			tblock.tm_year=atoi(cbuf+20)-1900;
		}
	}
	else if (len>27){
		char *p=strchr(cbuf,' ');
		if (p && strlen(p)>=19 && p[13]==':' && p[16]==':'){
			tdt="rfc850-date";
//		PERROR("decode_textdate:  rfc850-date");
			tblock.tm_mday=atoi(p+1);
			tblock.tm_mon=decode_textmonth(p+4);
			tblock.tm_year=atoi(p+8);
			if (tblock.tm_year>99){
				tblock.tm_year-=1900;
				p+=2;//account for four digit year
			}else if (tblock.tm_year<70){
				tblock.tm_year+=100;//avoid y2k probs with 2 digit years
			}
			tblock.tm_hour=atoi(p+11);
			tblock.tm_min=atoi(p+14);
			tblock.tm_sec=atoi(p+17);
			td_tz=decode_texttz(p+20);
		}
	}
	if(!tdt){
		PERROR("decode_textdate: unknown %s",cbuf);
		return 0;
	}else
		PDEBUG("decode_textdate: %s %i %i %i %i %i %i %i",tdt,tblock.tm_year,tblock.tm_mon,tblock.tm_mday,tblock.tm_hour,tblock.tm_min,tblock.tm_sec,td_tz);
	return mktime(&tblock)-my_timezone-td_tz;
}
#endif

void setint0 (int *i){
	PDEBUG("setint0 %p",i);
	*i=0;
}
