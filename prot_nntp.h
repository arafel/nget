#ifndef _PROT_NNTP_H_
#define _PROT_NNTP_H_
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif

#include "cache.h"
#include <string>
#include "file.h"

struct arinfo {
	long Bps;
	time_t dtime;
	
	long anum,linesdone,linestot;
	long bytesdone,bytestot;
	time_t starttime;
	void print_retrieving_articles(time_t curtime,arinfo*tot);
};

class c_prot_nntp /*: public c_transfer_protocol */{
	public:
//		int ch;
		char *cbuf;
//		int cbuf_size;
		c_file_tcp sock;
		string host;
		string group;
		int groupselected;
		c_cache *gcache;
		c_nntp_file_cache_u *filec;
		time_t starttime;

		int putline(int echo, const char * str,...)
	        __attribute__ ((format (printf, 3, 4)));
		int getline(int echo);
		int getreply(int echo);
		int stdgetreply(int echo);
		void doxover(int low, int high);
		void nntp_queueretrieve(const char *match, int linelimit, int getflags);
		void nntp_retrieve(int doit);
		void nntp_print_retrieving_headers(long lll,long hhh,long ccc,long bbb);
//		void nntp_print_retrieving_articles(long nnn, long tot,long done,long btot,long bbb);
//		void nntp_print_retrieving_articles(arinfo *ari, arinfo*tot);
		void nntp_group(const char *group, int getheaders);
		void nntp_dogroup(int getheaders);
		//void nntp_doarticle(long num,long ltotal,long btotal,char *fn);
		void nntp_doarticle(arinfo*ari,arinfo*toti,char *fn);
		void nntp_open(const char *h);
		void nntp_doopen(void);
		void nntp_close(void);
		void cleanupcache(void);
		void cleanup(void);
		c_prot_nntp();
		~c_prot_nntp();
};
	
#endif
