#ifndef _PROT_NNTP_H_
#define _PROT_NNTP_H_

#include "cache.h"
#include <string>
class c_prot_nntp /*: public c_transfer_protocol */{
	public:
		int ch;
		char *cbuf;
		int cbuf_size;
		string host;
		string group;
		int groupselected;
		c_cache *gcache;
		time_t starttime;

		int putline(int echo, const char * str,...);
		int getline(int echo);
		int getreply(int echo);
		int stdgetreply(int echo);
		void doxover(int low, int high);
		void nntp_retrieve(const char *match, int linelimit, int doit,int getflags);
		void nntp_print_retrieving_headers(long lll,long hhh,long ccc,long bbb);
		void nntp_print_retrieving_articles(long nnn, long tot,long done,long btot,long bbb);
		void nntp_group(const char *group, int getheaders);
		void nntp_dogroup(int getheaders);
		void nntp_doarticle(long num,long ltotal,long btotal,char *fn);
		void nntp_open(const char *h);
		void nntp_doopen(void);
		void nntp_close(void);
		void cleanup(void);
		c_prot_nntp();
		~c_prot_nntp();
};
	
#endif
