#include "prot_nntp.h"
#include "sockstuff.h"
#include "misc.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
extern int quiet;
extern time_t lasttime;
int c_prot_nntp::putline(const char * str,...){
//	static char fpbuf[255];
	char *fpbuf;
	int i;
	va_list ap;

	va_start(ap,str);
	vasprintf(&fpbuf,str,ap);
	va_end(ap);

	if (!quiet)
		printf(">%s\n",fpbuf);
	time(&lasttime);
	if (((i=sockprintf(ch,"%s\r\n",fpbuf))<=0)){
		printf("nntp_putline:%i %s(%i)\n",i,strerror(errno),errno);
		delete(fpbuf);
		throw -2;
	}
	delete(fpbuf);
	return i;
}

int c_prot_nntp::getline(void){
	int i=sock_gets(ch,cbuf,cbuf_size);
	if (i<=0){
		printf("nntp_getline:%i %s(%i)\n",i,strerror(errno),errno);
		throw -2;
	}else {
		time(&lasttime);
		if (!quiet)
			printf("%s\n",cbuf);
	}
	return i;

}
int c_prot_nntp::getreply(void){
	int code;
	if ((code=getline())>=0)
		code=atoi(cbuf);
//	if (cbuf[3]=='-')
//		do{
//			ftp_getline(cbuf,cbuf_size);
//		}while((atoi(cbuf)!=code)||(cbuf[3]!=' '));
	return code;
}
void c_prot_nntp::doxover(int low, int high){
	char *p;
	c_nntp_cache_item*c;
	char *t[10];
	int i;
	putline("XOVER %i-%i",low,high);
	if (getreply()/100!=2)
		throw -3;
	do {
		getline();
		if (cbuf[0]=='.')break;
		p=cbuf;
		for(i=0;i<10;i++){
			if((t[i]=goodstrtok(&p,'\t'))==NULL){
				break;
			}
			//				printf("%i:%s\n",i,t[i]);
		}
		if (i>=7){
			c=new c_nntp_cache_item(atol(t[0]),	decode_textdate(t[3]), atol(t[6]), atol(t[7]),t[1],t[2]);
			gcache->additem(c);
		}
	}while(1);
}
void c_prot_nntp::nntp_group(const char *group){
	int num,low,high;
	char *p;
	putline("GROUP %s",group);
	if (getreply()/100!=2)
		throw -3;
	p=strchr(cbuf,' ')+1;
	num=atoi(p);
	p=strchr(p,' ')+1;
	low=atoi(p);
	p=strchr(p,' ')+1;
	high=atoi(p);
//	printf("%i, %i, %i\n",num,low,high);
	gcache=new c_cache("/home/donut/.nget/",group);
	if (gcache->high<high)
		doxover(gcache->high+1,high);
	if (gcache->low>low)
		doxover(low,gcache->low-1);
}
void c_prot_nntp::nntp_open(const char *host){
	if((ch=make_connection("nntp",SOCK_STREAM,host,NULL,cbuf,cbuf_size))<0){
		throw -1;
	}
	if (getreply()!=200)
		throw -1;
	putline("MODE READER");
	getline();

}
void c_prot_nntp::nntp_close(void){
	if(ch>=0){
		close(ch);ch=-1;
	}
}
c_prot_nntp::c_prot_nntp(void){
	cbuf=new char[4096];
	cbuf_size=4096;
	gcache=NULL;
}
c_prot_nntp::~c_prot_nntp(void){
	if (ch>=0){
		putline("QUIT");
		nntp_close();
	}
	delete cbuf;
	if(gcache)delete gcache;
}
