#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "file.h"

void testf(char *filen){
	c_file_fd fb;
	c_file_fd f;
	fb.open(filen,O_RDONLY);
//	fb.open(filen,"r");
	fb.initrbuf(2048);
	f.open(filen,O_RDONLY);
//	f.open(filen,"r");
	char buf[2048];
	int lb,l;//,lrb,lr;
	int line=0;
	while (1){
		line++;
		fb.bgets();
		if (!f.gets(buf,2048))return;
		l=strlen(buf);
/*		if (l>0 && (buf[l-1]=='\n' || buf[l-1]=='\r')){
			buf[--l]=0;
			if (l>0 && (buf[l-1]=='\n' || buf[l-1]=='\r'))
				buf[--l]=0;
		}*/
		lb=strlen(fb.rbufp());
		if (l!=lb || strcmp(fb.rbufp(),buf)!=0){
			printf("line %i %i/%i\n\"%s\"\n\"%s\"\n",line,lb,l,fb.rbufp(),buf);
			sleep(10000);
		}
	}
}

int main(int argc,char **argv){
	testf(argv[1]);
}
