#include "nrange.h"
#include "file.h"
#include "stlhelp.h"

int debug=5;
#define DOCHK(i) printf("chk %i=%i\n",i,range.check(i))
int testremv(int il,int ih,int rl,int rh){
	c_nrange range("");
	c_file_fd f;
	f.dup(STDOUT_FILENO);
	printf("testremv %i-%i, %i-%i\n",il,ih,rl,rh);
	range.insert(10,30);
	range.insert(110,200);
	range.insert(il,ih);
//	range.insert(il,ih);
	range.remove(rl,rh);
	range.print(&f);
	f.close();
	return 0;
}
int testins(int il,int ih,int rl,int rh){
	c_nrange range("");
	c_file_fd f;
	f.dup(STDOUT_FILENO);
	printf("testins %i-%i, %i-%i\n",il,ih,rl,rh);
	range.insert(10,30);
	range.insert(110,200);
	range.insert(il,ih);
	range.insert(rl,rh);
	range.print(&f);
	f.close();
	return 0;
}
class tester{
	public:
		int aoeu;
		tester(int i){
			printf("tester ctor %i\n",i);
			aoeu=i;
		}
		~tester(){printf("tester dtor %i\n",aoeu);}
};
int main (void){
	tester a(1);
	printf("hi\n");
	tester b(2);
	printf("hi2\n");
	safestring hi(NULL);
	c_nrange range("");
	range.insert(3,100);
	DOCHK(2);
	DOCHK(3);
	DOCHK(4);
	DOCHK(40);
	DOCHK(99);
	DOCHK(100);
	DOCHK(101);
	testremv(50,100,25,75);
	testremv(50,100,60,75);
	testremv(50,100,60,125);
	testremv(50,100,25,125);
	testremv(50,100,49,125);
	testremv(50,100,50,125);
	testremv(50,100,51,125);
	testremv(50,100,75,99);
	testremv(50,100,75,100);
	testremv(50,100,75,101);
	printf("ins\n");
	testins(35,40,50,60);
	testins(0,300,1,2);
	testins(0,150,1,2);
	testins(50,90,15,150);
	testins(31,60,61,109);
	testins(31,109,50,51);
	testins(31,110,50,51);
	testins(30,109,50,51);
	testins(30,108,50,51);
	testins(32,108,50,51);
	testins(32,109,50,60);
	return 0;
}
