#include "cache.h"
class c_prot_nntp /*: public c_transfer_protocol */{
	public:
		int ch;
		char *cbuf;
		int cbuf_size;
		c_cache *gcache;
		
		int putline(const char * str,...);
		int getline(void);
		int getreply(void);
		void doxover(int low, int high);
		void nntp_group(const char *group);
		void nntp_open(const char *host);
		void nntp_close(void);
		c_prot_nntp();
		~c_prot_nntp();
};
	

