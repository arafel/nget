#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include <stdlib.h>
#include "myregex.h"

int c_regex::match(const char *str){
	int i;
	if ((i=regexec(regex,str,nregmatch,regmatch,0)))
	     return i;
	freesub();
	rnsub=regex->re_nsub;
	if (rnsub>=0 && nregmatch){
		rsub=new (string*)[rnsub+1];
		for (i=0;i<=rnsub;i++){
//			rsub[i].spf("%.*s",regmatch[i].rm_eo-regmatch[i].rm_so,str+regmatch[i].rm_so);
//			rsub[i].append(str+regmatch[i].rm_so,regmatch[i].rm_eo-regmatch[i].rm_so);
			rsub[i]=new string(str+regmatch[i].rm_so,regmatch[i].rm_eo-regmatch[i].rm_so);
		}
	}
	return 0;
}
c_regex::c_regex(const char * pattern,int cflags){
	int i;
	if (!pattern)
		pattern="";
	regex=new regex_t;
	i=regcomp(regex,pattern,cflags);
	rnsub=-1;
	if (i){
		nregmatch=-1;
		return;
	}
	if (cflags&REG_NOSUB){
		nregmatch=0;
		regmatch=NULL;
	}else{
		nregmatch=1;
		for (;*pattern!=0;pattern++){
			if (*pattern=='(')//this could give more regmatches than we need, considering 
				nregmatch++;//escaping and such, but its a lot better than a static number
		}
		//	if (nregmatch)
		regmatch=new regmatch_t[nregmatch+2];
		//	regmatch= (regmatch_t*) malloc(sizeof(regmatch_t)*(nregmatch+2));
	}
};
c_regex::~c_regex(){
	freesub();
	regfree(regex);
	if (regmatch){
		delete regmatch;
//		free(regmatch);
	}
};
void c_regex::freesub(void){
	if (rnsub>=0 && nregmatch){
		for (int i=0;i<=rnsub;i++){
			delete rsub[i];
		}
		delete rsub;
	}
	rsub=NULL;
	rnsub=-1;
};
