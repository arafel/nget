#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "misc.h"
#include "sockstuff.h"
#include "prot_nntp.h"

void print_help(void){
      printf("nget v0.1 - nntp command line fetcher\n");
      printf("Copyright 1999 Matt Mueller <donut@azstarnet.com>\n");
      printf("\n\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n");	
      printf("\nUSAGE: nget [-q] [-h host] ?\n\n"
	       "-a	quiet mode.  Use twice for total silence\n"
);
}

char * nntp_server;
int quiet=0;
time_t lasttime;
struct option long_options[]={
	{"quiet",1,0,'q'},
	{"host",1,0,'h'},
	{"group",1,0,'g'},
	{"help",0,0,'?'},
	{0,0,0,0}
};


int nntp_get(char *host, char * group, char* file);
int main(int argc,char ** argv){
	int c,mode=0,opt_idx;
	char * group=NULL;
	nntp_server=getenv("NNTPSERVER");
	if (argc<2){
		print_help();
	}
	else {
		while ((c=getopt_long(argc,argv,"qh:g:?",long_options,&opt_idx))>=0){
			switch (c){
				case 'g':
					group=optarg;
					break;
				case 'h':
					nntp_server=optarg;
					break;
				case 'q':
					quiet++;
					break;
				case ':':
				case '?':
					print_help();
					return 1;
			}
		}
		printf("quiet=%i host=%s\n",quiet,nntp_server);
		for (int i=optind;i<argc;i++)
			switch (mode){
				default:
					printf("%s\n",argv[i]);
					nntp_get(nntp_server,group,argv[i]);
			}
	}
	return 0;
}

int nntp_get(char *host, char * group, char* file){
	c_prot_nntp nntp;
	try {
		nntp.nntp_open(host);
		nntp.nntp_group(group);
	}catch(int exerr){
		printf("caught exception %i\n",exerr);
	}
	nntp.putline("QUIT");
	nntp.nntp_close();
	
	return 0;
};

