#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include "file.h"
//#include "cache.h"
#include "nrange.h"
//#include <string>
//#include <map.h>
int debug=1;
int main(void){
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
	return 0;
}
