#ifndef _CACHE_H_
#define _CACHE_H_
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <string>
#include <map.h>
#ifdef HAVE_HASH_MAP_H
#include <hash_map.h>
#else
#include <multimap.h>
#endif
#include "file.h"

#define CACHE_VERSION "NGET2"

typedef unsigned long t_id;
class c_nntp_header {
	private:
		int parsepnum(const char *str,const char *soff);
		void setmid(void);
	public:
		int partnum, req;
		int partoff, tailoff;
		t_id mid;
		string subject;
		string author;
		ulong articlenum;
		time_t date;
		ulong bytes,lines;
		void set(char *s,const char *a,ulong anum,time_t d,ulong b,ulong l);
//		c_nntp_header(char *s,const char *a,ulong anum,time_t d,ulong b,ulong l);
};

class c_nntp_part {
	public:
		int partnum;
		ulong articlenum;
		time_t date;
		ulong bytes,lines;
		c_nntp_part(int pn, ulong an, time_t d, ulong b, ulong l):partnum(pn),articlenum(an),date(d),bytes(b),lines(l){};
		c_nntp_part(c_nntp_header *h);
};


typedef map<int,c_nntp_part*,less<int> > t_nntp_file_parts;

#define FILEFLAG_READ 1

class c_nntp_file {
	public:
		t_nntp_file_parts parts;
		int req,have;
		ulong bytes,lines;
		ulong flags;
		t_id mid;
		string subject,author;
		int partoff,tailoff;
		void addpart(c_nntp_part *p);
		ulong banum(void){return (*parts.begin()).second->articlenum;}
		c_nntp_file(c_nntp_header *h);
		c_nntp_file(int r,ulong f,t_id mi,const char *s,const char *a,int po,int to);
		~c_nntp_file();
};
//typedef hash_map<const char*, c_nntp_file*, hash<const char*>, eqstr> t_nntp_files;
#ifdef HAVE_HASH_MAP_H
typedef hash_multimap<t_id, c_nntp_file*, hash<t_id>, equal_to<t_id> > t_nntp_files;
#else
typedef multimap<t_id, c_nntp_file*, less<t_id> > t_nntp_files;
#endif
typedef map<ulong,c_nntp_file*,less<ulong> > t_nntp_files_u;
class c_nntp_files_u {
	public:
		ulong bytes,lines;
		t_nntp_files_u files;
		c_nntp_files_u(void):bytes(0),lines(0){}
};

#define GETFILES_CASESENSITIVE  1
#define GETFILES_GETINCOMPLETE  2
#define GETFILES_NODUPECHECK    4

class c_nntp_cache {
	public:
		string file;
		t_nntp_files files;
		ulong high,low,num;
		int saveit;
		int fileread;
		//int additem(ulong an,char *s,const char * a,time_t d, ulong b, ulong l){
		int additem(c_nntp_header *h);
		ulong flushlow(ulong newlow);
		c_nntp_files_u* getfiles(c_nntp_files_u * fc,const char *match, unsigned long linelimit,int flags);
		c_nntp_cache(string path,string hid,string nid);
		~c_nntp_cache();
};
#endif
