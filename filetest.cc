#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include "file.h"
//#include "cache.h"
//#include "nrange.h"
#include "myregex.h"
//#include <string>
//#include <map.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "etree.h"
//#include <algo.h>
int debug=1;

#if 0

#define NTEST 41
char *testt[NTEST]={
	"Path: rQdQ!remarQ73!remarQ.com!supernews.com!dispose.news.demon.net!demon!diablo.theplanet.net!dca1-hub1.news.digex.net!intermedia!feed1.news.rcn.net!rcn!nntp.abs.net!newshub2.home.com!news.home.com!news1.frmt1.sfba.home.com.POSTED!not-for-mail",
	"a",
	"ab",
	"aa.",
	"a3hb",
	"aaoeu",
	"aaaaaa",
	"blah: a",
	"aaaaaaaa",
	"Lines: 7501",
	"aaaaaaaaaauuuu",
	"iaaaaaaaaaaaaaaaa",
	"ioaaaaaaaaaaaaaaaa",
	"iaaaaaaaaaaaaaaaaaa",
	"iaaaaaaaaaaaaaaaaaaa",
	"aaaaaiaaaaaaaaaaaaaaaa",
	"222222222iaaaaaaaaaaaaaaaa",
	"From: ayanami@v-wave.com (Neo)",
	"X-Complaints-To: abuse@home.net",
	"NNTP-Posting-Host: 24.108.12.114",
	"Organization: @Home Network Canada",
	"Date: Mon, 08 Nov 1999 09:27:54 GMT",
	"Newsgroups: alt.binaries.sounds.jpop",
	"Xref: rQdQ alt.binaries.sounds.jpop:73684",
	"X-Newsreader: Forte Free Agent 1.11/32.235",
	"aX-Newsreader: Forte Free Agent 1.11/32.235",
	"NNTP-Posting-Date: Mon, 08 Nov 1999 01:27:54 PST",
	"blarge: ta te te etette te tete te tet ete tete tet e",
	"Message-ID: <3826956b.131054626@news.rdc1.sfba.home.com>",
	"Subject: [album] Trinity - DOH-Yo - 03 - spasm mix.mp3 (01/25)",
	"Seeeeeeeeubject: [album] Trinity - DOH-Yo - 03 - spasm mix.mp3 (01/25)",
	"anotemun: nettttttttttttttttttttttttttttttttttttteaaaaaaaaaaaaaaaaaaaaaaaaa",
	"anotemun: nettttttttttttttttttttttttttttttttttttteaaaaaaaaaaaaaaaaaaaaaaaaab",
	"anotemun: nettttttttttttttttttttttttttttttttttttteaaaaaaaaaaaaaaaaaaaaaaaaabc",
	"anotemun: netttttttttttttttttttttttttttt3ttttttttteaaaaaaaaaaaaaaaaaaaaaaaaabc",
	"anotemun: netttttttttttttttttttttttttttt3ttttttttteaaaaaaaaaaaaaaaaaaaaaaaaabca",
	"3anotemun: netttttttttttttttttttttttttttt3ttttttttteaaaaaaaaaaaaaaaaaaaaaaaaabca",
	"X-Trace: news1.frmt1.sfba.home.com 942053274 24.108.12.114 (Mon, 08 Nov 1999 01:27:54 PST)",
	"",
	"begin 644 03 - spasm mix.mp3",
	"M__NP1```\"```2P````````E@```````!+````````\"6`````____________"
};
/*#define T2L 2000
char testt2[T2L];
int ol=0;*/
char * gettext(int i){
/*	if (i==0){
		for (int j=0;j<T2L;j++)
			testt2[j]=rand()%90+35;
	}
	int l=rand()%T2L;
	if (l==0)
		l=rand()%T2L;
	else
		l=rand()%128;
	testt2[ol]=rand()%90+35;
	testt2[l]=0;
	ol=l;
	return testt2;*/
/*	if ((rand()%100)==1)
		return testt[i%NTEST];
	if (i<NTEST) return testt[i];
	else return testt[NTEST-1];*/
	return testt[rand()%NTEST];
}
#endif

#define NBUFPS 70
//#define PREBUFPS 15
#define PREBUFPS 58
//#define TMIN 12
#define TMIN 1024
#define TNUM 4
#define TLEN 5000000
#define TMAX TMIN*((2<<(TNUM-1))>>1)+1
//char *bufps[NBUFPS];
char bufps[NBUFPS][256];
int bufls[NBUFPS];
int wbufps=0;
int rbufps=0;

c_file_testpipe f;

int wlen;
char *wbuf;

int fl;
int fi;
char fc;

int fillb(char *b){
	fl=rand()%130;
	fc=rand()%26+'a';
	for (fi=0;fi<fl;fi++){
		b[fi]=fc;
	}
	b[fl]=0;
	return fl;
}

void adowrite(char *b){
	strcpy(bufps[wbufps],b);
	wlen=strlen(b);
	f.write(b,wlen);
	bufls[wbufps]=wlen+2;
	wbufps=(wbufps+1)%NBUFPS;
	f.write("\r\n",2);
}
void adowrite(void){
//		wbuf=gettext(i);
	wbuf=bufps[wbufps];
	wlen=fillb(wbuf);
//		bufps[wbufps]=wbuf;
//	f.write(wbuf,wlen=strlen(wbuf));
	f.write(wbuf,wlen);
	bufls[wbufps]=wlen+2;
	wbufps=(wbufps+1)%NBUFPS;
	f.write("\r\n",2);
}
//942378700
int main(int argc, char **argv){
	int r,i,t,seed;
	char *buf;
	if (argc<=1){
		seed=time(NULL);
	}else{
		seed=atoi(argv[1]);
	}
	srand(seed);
	printf("seed=%i\n",seed);
	f.open();
	adowrite("1234");
	adowrite("abcd");
	adowrite("E");
	adowrite("F");
	for (i=0;i<PREBUFPS;i++){
		adowrite();
	}
	for (t=TMIN;t<TMAX;t*=2){
		i=0;
		printf("trying %i\n",t);
		f.initrbuf(t);
		while (i<TLEN && (r=f.bgets())>0) {
	//		if (i>0 && i%10000==0)
				printf("on %i dsz=%i\n",i,f.datasize());
			buf=f.rbufp();
			//		printf("r=%i s=%s\n",r,buf);
			if ((r!=bufls[rbufps]) || (strcmp(buf,bufps[rbufps])!=0)){
				printf("t=%i i=%i r=%i/%i b=%s/%s\n",t,i,r,bufls[rbufps],buf,bufps[rbufps]);
				printf("\a\asleeping  (seed=%i)\n",seed);
				sleep(10000);
			}
			rbufps=(rbufps+1)%NBUFPS;
			adowrite();
			i++;
		}
	}
}
#if 0
struct stest {
	int a;
	string b;
};
template <class Op>
struct e_stest : public e_binary_function<stest *, typename Op::arg2_type, typename Op::ret_type> {
	typedef typename Op::arg2_type arg2_type;
//	arg2_type val2;
	Op O;
	
//	e_binder2nd(const arg2_type v):val2(v){};
	virtual ret_type operator()(const arg1_type v,const arg2_type val2) const=0;
};
template <class Op>
struct e_stesta : public e_stest<Op> {
	ret_type operator()(const arg1_type v,const arg2_type val2) const {return O(v->a,val2);}
};
template <class Op>
struct e_stestb : public e_stest<Op> {
	ret_type operator()(const arg1_type v,const arg2_type val2) const {return O(v->b,val2);}
};
int main(void){
//	c_etree tree;
//	c_exp exp;
//	int i=0;
//	exp.set(EXP_GE,0,i);
//	printf("%i %i %i\n",exp.compare(i-1),exp.compare(i),exp.compare(i+1));
	//Predicate *c;
	//typedef unary_function<int,bool> ipred;
	//typedef e_unary_function<int,int> ipred;
	stest sa,sb,sc;
	sa.a=1;sa.b="3a";
	sb.a=2;sb.b="hi";
	sc.a=3;sc.b="aoeuaou";
	//typedef e_unary_function<int,bool> ipred;
	typedef e_unary_function<stest*,bool> ipred;
//	typedef int(*ipred)(int);
//	ipred *cp;
	//ipred *c;
	ipred *c;
	//c=new binder2nd<binary_function<int,int,bool> >(greater_equal<int>(), 3);
	//c=new e_lnot<int>();
	//c=new e_binder2nd<e_stesta<e_gt<int> > >(1);
	c=ecompose2(new e_land<bool>,
			new e_binder2nd<e_stesta<e_gt<int> > >(1),
			new e_binder2nd_p<e_stestb<e_eq<string,c_regex*> > >(new c_regex("a"))
			);
/*	c=ecompose2(new e_lor<bool>,
			c,
			new e_binder2nd<e_stestb<e_eq<int> > >(3)
			);*/
//	c=new e_binder2nd<e_gt<int> >(3);
//	c=bind2nd(greater_equal<int>(), 3);
	//c=(ipred)bind2nd(greater_equal<int>(), 3);
	//unary_function<int,bool> *d=new logical_not<int>();
//	printf("%i\n",(*c)(4));
//	printf("%i\n",(*c)(3));
//	printf("%i\n",(*c)(0));
	printf("%i\n",(*c)(&sa));
	printf("%i\n",(*c)(&sb));
	printf("%i\n",(*c)(&sc));
	delete c;
//	printf("%i\n",sizeof(bool));
//	printf("%i\n",bind2nd(greater_equal<int>(), 3));
//	printf("%i\n",bind2nd(greater_equal<int>(), 3)(2));
	//printf("%i\n",greater_equal<int>()(2,3));
#if 0
//	c_file_gz f;
//	if(!f.open("./test.txt.gz","w")){
//		f.putf("aoeuaoeu%s\t%i\n","hi",3);
//		f.close();
//	}
//	string s="aoeuaoeu";
//	printf("%s\n",s.c_str());
//	s.erase(s.size()-1,s.size());
//	printf("%s\n",s.c_str());
	//c_nrange s("filetest.dat");
	c_nrange s("");
	c_file_fd f;
	f.dup(STDOUT_FILENO);
//	set <ulong> s;
	ulong l,j;
//	getchar();
//	for (j=0;j<1024;j++)
//		for (l=0;l<1024;l+=1)
//			s.insert(l+j*2048);
//			s.insert(j*2048);
//		s.addn(l);
	/*s.insert(100,200);
	s.insert(300,400);
	s.insert(5,50);
	s.insert(450,550);
	s.insert(10);
	s.insert(10);
//	s.insert(1);
	s.insert(14);*/
	s.insert(698902);
//	for (j=0;j<15;j++)
//		printf("%lu=%i\n",j,s.check(j));
	//getchar();
	s.print(&f);
	//s.remove(103,103);
	s.remove(0,100000);
	s.print(&f);
	f.close();
	printf("test\n");
/*	map<ulong,ulong> s;
	map<ulong,ulong>::iterator i;
	s[3]=3;
	s[10]=10;
	s[1]=1;
	s[12]=12;
//	lower_bound finds the first item that is >= arg
//	upper_bound finds the first item that is > arg
#define test(a) printf("%lu %lu %lu\n",a,(*s.lower_bound(a)).second,(*s.upper_bound(a)).second);
//	printf("%lu\n",(*s.upper_bound(3)).second);
//printf("%lu\n",(*s.upper_bound(2)).second);
	test(2);
	test(3);
	test(4);
	test(9);
	test(10);
	test(11);*/
#endif
	return 0;
}
#endif
