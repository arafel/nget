#ifndef _MYREGEX_H_
#define _MYREGEX_H_

#include <regex.h>
#include <assert.h>
//#include "mystring.h"
#include <string>

class c_regex{
//	regex_t regex;
	regex_t *regex;
	regmatch_t *regmatch;
	int nregmatch, rnsub;
	string **rsub;
  public:
	void freesub(void);
	int nsub(void) const {return rnsub;}
	const string &operator [](int i) const {return *rsub[i];}
	const string &sub(int i) const {return *rsub[i];}
	const char * subs(int i) const {return rsub[i]->c_str();}
	const int sublen(int i) const {return regmatch[i].rm_eo-regmatch[i].rm_so;}
	const int subeo(int i) const {return regmatch[i].rm_eo;}
	int match(const char *str);
	c_regex(const char * pattern,int cflags=REG_EXTENDED);
	~c_regex();
};

/*
//idea: make c_regex return a new c_regsub or something.
//pro: thread saftey/flexibility
//con: have to remember to delete it.
class c_regsub{
	int rnsub;
  public:
	int nsub(void) const {return rnsub;}
	mystring &operator [](int i) {return rsub[i];}
	mystring *rsub;
	c_regsub(int num);
	~c_regsub();
};
class c_regex{
	regex_t regex;
	regmatch_t *regmatch;
	int nregmatch
	void freesub(void);
  public:

	int match(const char *str);
	c_regex(const char * pattern);
	~c_regex();
};
*/

#endif
