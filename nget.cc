#include <signal.h>
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
#include "log.h"

//####these should prolly go in some other file....
c_error::c_error(int n, const char * s, ...){
	va_list ap;
	num=n;
	va_start(ap,s);
	vasprintf(&str,s,ap);
	va_end(ap);
	//printf("c_error %i constructed\n",num);
}
c_error::~c_error(){
	if (str) delete str;
	//printf("c_error %i deconstructed\n",num);
};
int quiet=0,debug=0;
time_t lasttime;
#define NUM_OPTIONS 13
struct option long_options[NUM_OPTIONS+1];
/*={
	{"quiet",1,0,'q'},
	{"host",1,0,'h'},
	{"group",1,0,'g'},
	{"quickgroup",1,0,'G'},
	{"retrieve",1,0,'r'},
	{"testretrieve",1,0,'R'},
	{"tries",1,0,'t'},
	{"limit",1,0,'l'},
	{"help",0,0,'?'},
	{0,0,0,0}
};*/
struct opt_help {
	int namelen;
	char *arg;
	char *desc;
};
//char * opt_help[NUM_OPTIONS+1];
//char * opt_ahelp[NUM_OPTIONS+1];
opt_help ohelp[NUM_OPTIONS+1];
int olongestlen=0;

void addoption(char *longo,int needarg,char shorto,char *adesc,char *desc){
	static int cur=0;
	long_options[cur].name=longo;
	long_options[cur].has_arg=needarg;
	long_options[cur].flag=0;
	long_options[cur].val=shorto;
	ohelp[cur].namelen=longo?strlen(longo):0;
	ohelp[cur].arg=adesc;
	ohelp[cur].desc=desc;
	int l=(adesc?strlen(adesc):0)+ohelp[cur].namelen+3;
	if (l>olongestlen)
		olongestlen=l;
	cur++;
}
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
//      printf("\nUSAGE: nget [-q] [-h host] [-g group] [-r subject regex] ?\n\n"
//	       "-q	quiet mode.  Use twice for total silence\n"
//);
	printf("\nUSAGE: nget -g group [-r file [-r file] [-g group [-r ...]]]\n");
	  for (int i=0;long_options[i].name;i++){
		  if (long_options[i].has_arg)
			  printf("-%c  --%s=%-*s  %s\n",long_options[i].val,long_options[i].name,olongestlen-ohelp[i].namelen-1,ohelp[i].arg,ohelp[i].desc);
		  else
			  printf("-%c  --%-*s  %s\n",long_options[i].val,olongestlen,long_options[i].name,ohelp[i].desc);
	  }
}


c_prot_nntp nntp;

void term_handler(int s){
	printf("\nterm_handler: signal %i, shutting down.\n",s);
	nntp.cleanup();
	exit(0);
}


string nghome;

int main(int argc,char ** argv){
	int c=0,opt_idx;
	char * group=NULL;
	nntp.nntp_open(getenv("NNTPSERVER")?:"localhost");
	addoption("quiet",0,'q',0,"supress extra info");
	addoption("host",1,'h',"HOSTNAME","nntp host (default $NNTPSERVER)");
	addoption("group",1,'g',"GROUPNAME","newsgroup");
	addoption("quickgroup",1,'G',"GROUPNAME","use group without checking for new headers");
	addoption("retrieve",1,'r',"REGEX","retrieve files matching regex");
	addoption("testretrieve",1,'R',"REGEX","test what would have been retrieved");
	addoption("tries",1,'t',"INT","set max retries (-1 unlimits, default 10)");
	addoption("limit",1,'l',"INT","min # of lines a 'file' must have(default 3)");
	addoption("incomplete",0,'i',0,"retrieve files with missing parts");
	addoption("complete",0,'I',0,"retrieve only files with all parts(default)");
	addoption("case",0,'c',0,"match casesensitively");
	addoption("nocase",0,'C',0,"match iscasesensitively(default)");
	addoption("help",0,'?',0,"this help");
	addoption(NULL,0,0,0,NULL);
	signal(SIGTERM,term_handler);
	signal(SIGHUP,term_handler);
	signal(SIGINT,term_handler);
	signal(SIGQUIT,term_handler);
	nghome=getenv("HOME");
	nghome.append("/.nget/");
	time(&lasttime);
	srand(lasttime);
	if (argc<2){
		print_help();
	}
	else {
		int redo=0,redone=0,maxretry=10,badhost=0,linelimit=3,gflags=0;
		while (1){
			if (redo){
				redo=0;
			}else {
				if ((c=getopt_long(argc,argv,"qh:g:G:r:R:t:l:iIcC?",long_options,&opt_idx))<0)
					break;
				redone=0;
			}
//#define GETFILES_CASESENSITIVE 1
//#define GETFILES_GETINCOMPLETE 2
			try {
				switch (c){
					case 'R':
						if(!group)
							perror("no group specified\n");
						nntp.nntp_retrieve(optarg,linelimit,0,gflags);
						break;
					case 'r':
						if (!badhost){
							if(!group)
								perror("no group specified\n");
							nntp.nntp_retrieve(optarg,linelimit,1,gflags);
						}
						break;
					case 'g':
						if (!badhost){
							group=optarg;
							nntp.nntp_group(optarg,1);
						}
						break;
					case 'G':
						group=optarg;
						nntp.nntp_group(optarg,0);
						break;
					case 'h':
						badhost=0;
						nntp.nntp_open(optarg);
						break;
					case 't':
						maxretry=atoi(optarg);
						if (maxretry==-1)
							maxretry=INT_MAX;
						printf("max retries set to %i\n",maxretry);
						break;
					case 'l':
						linelimit=atoi(optarg);
						if (linelimit<0)
							linelimit=0;
						printf("minimum line limit set to %i\n",linelimit);
						break;
					case 'i':
						gflags|= GETFILES_GETINCOMPLETE;
						break;
					case 'I':
						gflags&= ~GETFILES_GETINCOMPLETE;
						break;
					case 'c':
						gflags|= GETFILES_CASESENSITIVE;
						break;
					case 'C':
						gflags&= ~GETFILES_CASESENSITIVE;
						break;
					case 'q':
						quiet++;
						break;
//					case ':':
//					case '?':
					default:
						print_help();
						return 1;
				}
			}catch(c_error *e){
				int n=e->num;
				printf("caught exception %i: %s",n,e->str);
				delete e;
				if (n<EX_FATALS && redone<maxretry){
					redo=1;redone++;
					printf(" (trying again. %i)\n",redone);
				}else{
					if (n==EX_A_FATAL){
						printf(" (fatal application error, exiting..)\n");
						return -1;
					}else{
						printf(" (fatal, aborting..)\n");
						if (n==EX_T_FATAL)
							badhost=1;
					}	
				}
			}
		}
			//		printf("quiet=%i host=%s\n",quiet,nntp_server);
			//		for (int i=optind;i<argc;i++)
			//			switch (mode){
			//				default:
			//					printf("%s\n",argv[i]);
			//					nntp_get(nntp_server,group,argv[i]);
			//			}

	}
	return 0;
}

