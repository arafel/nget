#ifndef _CACHE_H_
#define _CACHE_H_
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include <string>
//#include "mystring.h"
#include <list.h>
#include <map.h>
#include "file.h"
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include <time.h>
class c_nntp_cache_item {
	public:
		long num;
		time_t date;
		long size,lines;
//		mystring subject,email;
		string subject,email;
		int storei(c_file *f);
#ifdef TEXT_CACHE
		static c_nntp_cache_item* loadi(char *s);
#else
		static c_nntp_cache_item* loadi(long l,c_file *f,char *buf);
#endif
//		c_nntp_cache_item(void){num=-1;};
		c_nntp_cache_item(long n,time_t d,long s, long l, char * a, char * e);
		~c_nntp_cache_item();
};

typedef map<long,c_nntp_cache_item*,less<long> > t_cache_items;
//typedef list<c_nntp_cache_item*> t_cache_items;

struct s_part {
//	long anum,partnum;
	c_nntp_cache_item *i;//we *don't* want to delete this!
	long partnum;
	int partoff,tailoff;
	int parseitem(c_nntp_cache_item *i);
	int parsepnum(const char *str,const char *soff);
};

struct s_part_u{
    long partnum;
	long anum,size,lines;
	s_part_u (s_part &p);
};
//typedef list<s_part> t_nntp_file_parts;
typedef map<int,s_part*,less<int> > t_nntp_file_parts;
typedef map<int,s_part_u*,less<int> > t_nntp_file_u_parts;

class c_nntp_file {
	public:
		t_nntp_file_parts parts;
		int req,have;
		long bytes,lines;
		//	string name;
		//	string author;
		int storef(c_file *f);
#ifdef TEXT_CACHE
		int loadf(char *str,t_cache_items *itp);
#else
		int loadf(c_file *f,t_cache_items *itp);
#endif
		int delitems(long lownum);
		void addpart(s_part *i);
		c_nntp_file();
		c_nntp_file(c_nntp_file &f);
		~c_nntp_file();
};
typedef list<c_nntp_file*> t_nntp_files;

class c_nntp_file_u {
	public:
		t_nntp_file_u_parts parts;
		int req,have;
		long bytes,lines;
		long banum;//article number of first part
		string filename; //what we think the filename is.
		c_nntp_file_u();
		c_nntp_file_u(c_nntp_file &f,const char *fn);
		~c_nntp_file_u();
};
typedef map<long, c_nntp_file_u*, less<long> > t_nntp_files_u;
//typedef list<c_nntp_file_u*> t_nntp_files_u;
class c_nntp_file_cache_u {
	public:
		//c_regex freg;
		long lines,bytes;
		t_nntp_files_u files;
		c_nntp_file_cache_u();
		~c_nntp_file_cache_u();
};

class c_nntp_file_cache {
	public:
		//c_regex freg;
		t_nntp_files files;
#ifdef DEBUG_CACHE
		void check_consistancy(t_cache_items *itp);
#endif
		void additem(c_nntp_cache_item* i);
		int delitems(long lownum);
		int store(c_file *f);
#ifdef TEXT_CACHE
		c_nntp_file * load(char *str);
#else
		void load(long l, c_file *f,t_cache_items *itp);
#endif
		c_nntp_file_cache();
		~c_nntp_file_cache();
};


#define GETFILES_CASESENSITIVE	1
#define GETFILES_GETINCOMPLETE	2
#define GETFILES_NODUPECHECK	4
class c_cache {
	public:
		string file;
		int high,low;
		int saveit;
		t_cache_items items;
		c_nntp_file_cache *files;
//		void initfiles(void);
		int fileread;
		void additem(c_nntp_cache_item* i, int do_file_cache=1);
		c_nntp_file_cache_u * getfiles(c_nntp_file_cache_u *fu,const char *match, int linelimit,int flags);
		int flushlow(int newlow);
#ifdef DEBUG_CACHE
		void check_consistancy(const char *s);
#endif
		c_cache(string path,string hid,string nid);
		~c_cache();
};


#ifdef DEBUG_CACHE_DESTRUCTION
void cache_dbginfo(void);
#endif

#endif
