#include <string>
//#include <list>
#include <map.h>
#include "file.h"
class c_nntp_cache_item {
	public:
		long num;
		time_t date;
		long size,lines;
		string subject,email;
		int store(c_file *f);
		static c_nntp_cache_item* load(char *s);
//		c_nntp_cache_item(void){num=-1;};
		c_nntp_cache_item(long n,time_t d,long s, long l, string a, string e);
};

typedef map<long,c_nntp_cache_item*,less<long> > t_cache_items;
//typedef list<c_nntp_cache_item*> t_cache_items;

class c_cache {
	public:
		string file;
		int high,low;
		t_cache_items items;
		void additem(c_nntp_cache_item* i);
		c_cache(string path,string nid);
		~c_cache();
};
