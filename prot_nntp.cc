#include "prot_nntp.h"
#include "sockstuff.h"
#include "uudeview.h"
#include "misc.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "log.h"
#include "config.h"
extern time_t lasttime;
extern string nghome;

int c_prot_nntp::putline(int echo,const char * str,...){
//	static char fpbuf[255];
	char *fpbuf;
	int i;
	va_list ap;

	va_start(ap,str);
	vasprintf(&fpbuf,str,ap);
	va_end(ap);

	if (echo)
		printf(">%s\n",fpbuf);
	time(&lasttime);
	if (((i=sockprintf(ch,"%s\r\n",fpbuf))<=0)){
//		printf("nntp_putline:%i %s(%i)\n",i,strerror(errno),errno);
		delete(fpbuf);
		throw new c_error(EX_T_ERROR,"nntp_putline:%i %s(%i)",i,strerror(errno),errno);
	}
	delete(fpbuf);
	return i;
}

int c_prot_nntp::getline(int echo){
	int i=sock_gets(ch,cbuf,cbuf_size);
	if (i<=0){
//		printf("nntp_getline:%i %s(%i)\n",i,strerror(errno),errno);
		throw new c_error(EX_T_ERROR,"nntp_getline:%i %s(%i)",i,strerror(errno),errno);
	}else {
		time(&lasttime);
		if (echo)
			printf("%s\n",cbuf);
	}
	return i;

}

int c_prot_nntp::stdgetreply(int echo){
	int i=getreply(echo);
	if (i/100!=2)
		throw new c_error(EX_P_FATAL,"bad reply: %s",cbuf);
	return i;
}

int c_prot_nntp::getreply(int echo){
	int code;
	if ((code=getline(echo))>=0)
		code=atoi(cbuf);
//	if (cbuf[3]=='-')
//		do{
//			ftp_getline(cbuf,cbuf_size);
//		}while((atoi(cbuf)!=code)||(cbuf[3]!=' '));
	return code;
}

inline void c_prot_nntp::nntp_print_retrieving_headers(long lll,long hhh,long ccc,long bbb){
	long tot=hhh-lll+1,done=ccc-lll+1;
	time_t dtime=lasttime-starttime;
	long Bps=(dtime>0)?bbb/dtime:0;
	long Bph=(done>0)?bbb/done:0;
	printf("\rRetrieving headers %li-%li: %li %3li%% %liB/s %lis ",lll,hhh,done,(tot!=0)?(done*100/tot):0,Bps,(Bps>0)?((hhh-ccc)*Bph)/(Bps):0);
	fflush(stdout);//@@@@
}

void c_prot_nntp::doxover(int low, int high){
	char *p;
	time_t curt,lastt=0;
	c_nntp_cache_item*c=NULL;
	char *t[10];
	long bytes=0,realnum=0;
	int i,lowest=-1,dcounter=0,dtrip=10,dtripmax=50,dtripmin=2,dtripdiv=50;
	nntp_doopen();
	nntp_dogroup(0);
	if (!quiet){
		time(&starttime);
		nntp_print_retrieving_headers(low,high,low,0);
	}
	putline(debug,"XOVER %i-%i",low,high);
	time(&starttime);
	time(&curt);
	stdgetreply(debug);
	do {
		bytes+=getline(debug);
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
			realnum++;
			if (lowest==-1){
				lowest=c->num;
				dtrip=(high-lowest)/dtripdiv;
				if (dtrip>dtripmax)
					dtrip=dtripmax;
				else if (dtrip<dtripmin)
					dtrip=dtripmin;
			}
			dcounter++;
			time(&curt);
			if (!quiet && dcounter>=dtrip && curt>lastt){
				nntp_print_retrieving_headers(lowest,high,c->num,bytes);
				dcounter=0;
				time(&lastt);
			}
		}
	}while(1);
	if(!quiet && c){
		nntp_print_retrieving_headers(lowest,high,c->num,bytes);
		printf("(%li)\n",realnum);
	}
}

void c_prot_nntp::nntp_dogroup(int getheaders){
	if(!gcache){
		throw new c_error(EX_U_FATAL,"nntp_dogroup: nogroup selected");
	}
	nntp_doopen();
	if (groupselected) return;
	putline(1,"GROUP %s",group.c_str());
	stdgetreply(1);
	groupselected=1;
	char *p;
	int num,low,high;
	p=strchr(cbuf,' ')+1;
	num=atoi(p);
	p=strchr(p,' ')+1;
	low=atoi(p);
	p=strchr(p,' ')+1;
	high=atoi(p);
//	printf("%i, %i, %i\n",num,low,high);
	if (getheaders){
		if (low>gcache->low)
			gcache->flushlow(low);
		if (gcache->high<high)
			doxover(gcache->high+1,high);
		if (gcache->low>low)
			doxover(low,gcache->low-1);
	}
};

void c_prot_nntp::nntp_group(const char *ngroup, int getheaders){
	group=ngroup;
	if (gcache) delete gcache;
	gcache=new c_cache(nghome,host,group);
	groupselected=0;
	if (getheaders)
		nntp_dogroup(getheaders);
}

inline void c_prot_nntp::nntp_print_retrieving_articles(long nnn, long tot,long done,long btot,long bbb){
	time_t dtime=lasttime-starttime;
	long Bps=(dtime>0)?bbb/dtime:0;
	printf("\rRetrieving article %li: %li/%lil %li/%liB %3li%% %liB/s %lis ",nnn,done,tot,bbb,btot,(tot!=0)?(done*100/tot):0,Bps,(Bps>0)?(btot-bbb)/(Bps):0);
	fflush(stdout);//@@@@
}


void c_prot_nntp::nntp_doarticle(long num,long ltotal,long btotal,char *fn){
	long bytes=0,lines=0;
	int header=1;
	time_t curt,lastt=0;
	c_file_stream f;
	nntp_doopen();
	nntp_dogroup(0);
	putline(debug,"ARTICLE %li",num);
	stdgetreply(debug);
	list<char *> buf;
	list<char *>::iterator curb;
	char *p,*lp;
	time(&starttime);
	curt=starttime;
	do {
		bytes+=getline(debug);
		if (cbuf[0]=='.'){
			if(cbuf[1]==0)break;//should we subtract the 3 extra bytes for ".\r\n"?
			lp=cbuf+1;
		}else
			lp=cbuf;
		lines++;
		if (header && lp[0]==0){
//			printf("\ntoasted header statssssssss\n");
			header=0;
			lines=0;time(&starttime);//bytes=0;
		}
		time(&curt);
		if (!quiet && curt>lastt){
			nntp_print_retrieving_articles(num,ltotal,lines,btotal,bytes);
			lastt=curt;
		}
		p=new char[strlen(lp)+1];
		strcpy(p,lp);
		buf.push_back(p);
	}while(1);
	if (!quiet){
		nntp_print_retrieving_articles(num,ltotal,lines,btotal,bytes);
		printf("\n");
	}
	if (!f.open(fn,"w")){
		for(curb = buf.begin();curb!=buf.end();++curb){
			p=(*curb);
			if (f.putf("%s\n",p)<=0)
				throw new c_error(EX_A_FATAL,"error writing %s: %s",fn,strerror(errno));
			delete p;
		}
		f.close();
	}else
		throw new c_error(EX_A_FATAL,"unable to open %s: %s",fn,strerror(errno));
}

void uu_msg_callback(void *v,char *m,int t){
	if (quiet && (t==UUMSG_MESSAGE || t==UUMSG_NOTE))
		return;
	printf("uu_msg(%i):%s\n",t,m);
};

int uu_busy_callback(void *v,uuprogress *p){
	if (!quiet) printf("uu_busy(%i):%s %i%%\r",p->action,p->curfile,(100*p->partno-p->percent)/p->numparts);
	return 0;
};

void c_prot_nntp::nntp_retrieve(const char *match, int linelimit, int doit, int getflags){
	c_nntp_file_cache *filec;
	if (!(filec=gcache->getfiles(match,linelimit,getflags)))
		throw new c_error(EX_U_FATAL,"nntp_retrieve: no match for %s",match);
	t_nntp_files::iterator curf;
	c_nntp_file *f;
	s_part *p;
	if (doit){
		s_part *bp;
		t_nntp_file_parts::iterator curp;
		char *fn;
		for(curf = filec->files.begin();curf!=filec->files.end();++curf){
			int r;
			if ((r=UUInitialize())!=UURET_OK)
				throw new c_error(EX_A_FATAL,"UUInitialize: %s",UUstrerror(r));
			UUSetOption(UUOPT_DUMBNESS,1,NULL);//we already put the parts in the correct order, so it doesn't need to.
			//UUSetOption(UUOPT_FAST,1,NULL);//we store each message in a seperate file
			//actually, I guess that won't work, since some messages have multiple files in them anyway.
			UUSetOption(UUOPT_OVERWRITE,0,NULL);//no thanks.
			UUSetOption(UUOPT_USETEXT,1,NULL);//######hmmm...
			UUSetMsgCallback(NULL,uu_msg_callback);
			UUSetBusyCallback(NULL,uu_busy_callback,1000);
			f=(*curf);
			bp=f->parts.begin()->second;
			list<char *> fnbuf;
			list<char *>::iterator fncurb;
			for(curp = f->parts.begin();curp!=f->parts.end();++curp){
				//asprintf(&fn,"%s/%s-%s-%li-%li-%li",nghome.c_str(),host.c_str(),group.c_str(),fgnum,part,num);
				p=curp->second;
				asprintf(&fn,"./%s-%s-%li-%03li-%li",host.c_str(),group.c_str(),bp->i->num,(*curp).first,p->i->num);
				if (!fexists(fn))
					nntp_doarticle(p->i->num,p->i->lines,p->i->size,fn);
				UULoadFile(fn,NULL,0);
				//				delete fn;
				fnbuf.push_back(fn);
			}
			uulist * uul;
			int derr=0;
			for (int un=0;;un++){
				if ((uul=UUGetFileListItem(un))==NULL)break;
				if (!(uul->state & UUFILE_OK)){
					printf("%s not ok\n",uul->filename);
					derr++;
					continue;
				}
				r=UUDecodeFile(uul,NULL);
				if (r==UURET_EXISTS){
					char *nfn;
					asprintf(&nfn,"%s.%i",uul->filename,rand());
					UURenameFile(uul,nfn);
					delete nfn;
					r=UUDecodeFile(uul,NULL);
				}
				if (r!=UURET_OK){
					derr++;
					printf("decode(%s): %s\n",uul->filename,UUstrerror(r));
					continue;
				}
			}
			UUCleanUp();
			if (derr)
				printf("%i decoding errors occured, keeping temp files.\n",derr);
			else
				printf("decoded ok, deleting temp files.\n");
			char *p;
			for(fncurb = fnbuf.begin();fncurb!=fnbuf.end();++fncurb){
				p=(*fncurb);
				if (!derr)
					unlink(p);
				delete p;
			}
		}
	}else{    
		for(curf = filec->files.begin();curf!=filec->files.end();++curf){
			f=(*curf);
			p=f->parts.begin()->second;
			//printf("%i/%i\t(%liB,%lil)\t%li\t%s\n",f->have,f->req,f->bytes,f->lines,p->i->date,p->i->subject.c_str());
			printf("%i/%i\t%lil\t%li\t%s\t%s\n",f->have,f->req,f->lines,p->i->date,p->i->subject.c_str(),p->i->email.c_str());
		}
	}
	delete filec;
}

void c_prot_nntp::nntp_open(const char *h){
	nntp_close();
	host=h;
}

void c_prot_nntp::nntp_doopen(void){
	if (ch<0){
		time(&lasttime);
		if((ch=make_connection("nntp",SOCK_STREAM,host.c_str(),NULL,cbuf,cbuf_size))<0){
			//throw -1;
			throw new c_error(EX_T_FATAL,"make_connection:%i %s(%i)",ch,strerror(errno),errno);
		}
		stdgetreply(1);
		putline(debug,"MODE READER");
		getline(debug);
		groupselected=0;
	}
}

void c_prot_nntp::nntp_close(void){
	if(ch>=0){
		close(ch);ch=-1;
	}
}

void c_prot_nntp::cleanup(void){
	if (ch>=0){
		putline(debug,"QUIT");
		nntp_close();
	}
	if (cbuf){delete cbuf;cbuf=NULL;}
	if(gcache){delete gcache;gcache=NULL;}
}

c_prot_nntp::c_prot_nntp(void){
	cbuf=new char[4096];
	cbuf_size=4096;
	gcache=NULL;
	ch=-1;
}
c_prot_nntp::~c_prot_nntp(void){
//	printf("nntp destructing\n");
	cleanup();
}
