#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include <stdio.h>
#include "prot_nntp.h"
#include "sockstuff.h"
#include "uudeview.h"
#include "misc.h"
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "log.h"

extern time_t lasttime;
extern string nghome;

int c_prot_nntp::putline(int echo,const char * str,...){
//	static char fpbuf[255];
	char *fpbuf;
	int i;
	va_list ap;

	va_start(ap,str);
	i=vasprintf(&fpbuf,str,ap);
	va_end(ap);
	if (!(fpbuf=(char*)realloc(fpbuf,i+3))){
		delete(fpbuf);
		throw new c_error(EX_T_ERROR,"nntp_putline:realloc(%p,%i) %s(%i)",fpbuf,i+3,strerror(errno),errno);
	}
	if (echo)
		printf(">%s\n",fpbuf);
	fpbuf[i]='\r';fpbuf[i+1]='\n';
	time(&lasttime);
//	if (((i=sockprintf(ch,"%s\r\n",fpbuf))<=0)){
//	if (((i=sock.putf("%s\r\n",fpbuf))<=0)){
//	if (((i=sock.putf("%s\r\n",fpbuf))<=0)){
	if (((i=sock.write(fpbuf,i+2))<=0)){
//		printf("nntp_putline:%i %s(%i)\n",i,strerror(errno),errno);
		delete(fpbuf);
		throw new c_error(EX_T_ERROR,"nntp_putline:%i %s(%i)",i,strerror(errno),errno);
	}
	delete(fpbuf);
	return i;
}

int c_prot_nntp::getline(int echo){
	//int i=sock_gets(ch,cbuf,cbuf_size);
	int i=sock.bgets();
	if (i<=0){
		throw new c_error(EX_T_ERROR,"nntp_getline:%i %s(%i)",i,strerror(errno),errno);
	}else {
		cbuf=sock.rbufp;
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
	if(!gcache && getheaders){
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
//	if (gcache) delete gcache;
	cleanupcache();
	gcache=new c_cache(nghome,host,group);
	groupselected=0;
	if (getheaders)
		nntp_dogroup(getheaders);
}

inline void arinfo::print_retrieving_articles(time_t curtime, arinfo*tot){
	dtime=curtime-starttime;
	Bps=(dtime>0)?bytesdone/dtime:0;
	printf("\rarticle %li: %li/%liL %li/%liB %3li%% %liB/s %lis ",
			anum,linesdone,linestot,bytesdone,bytestot,
			(linestot!=0)?(linesdone*100/linestot):0,Bps,
			(linesdone>=linestot)?dtime:((Bps>0)?(bytestot-bytesdone)/(Bps):-1));
	if (tot)
		printf("%li/%li %lis ",tot->linesdone,tot->linestot,
				//(tot->bytesdone>=tot->bytestot)?curtime-tot->starttime:((Bps>0)?(tot->bytestot-tot->bytesdone)/(Bps):-1));
				(linesdone>=linestot)?curtime-tot->starttime:((Bps>0)?(tot->bytestot-tot->bytesdone)/(Bps):-1));
	fflush(stdout);
}

//inline void c_prot_nntp::nntp_print_retrieving_articles(long nnn, long tot,long done,long btot,long bbb,unsigned long obtot,unsigned long obdone,long ototf,long odonef,time_t tstarttime){
//	time_t dtime=lasttime-starttime;
//	long Bps=(dtime>0)?bbb/dtime:0;
//	printf("\rRetrieving article %li: %li/%liL %li/%liB %3li%% %liB/s %lis %li/%li %lis",nnn,done,tot,bbb,btot,(tot!=0)?(done*100/tot):0,Bps,(done>=tot)?dtime:((Bps>0)?(btot-bbb)/(Bps):-1),odonef,ototf,(obdone>=obtot)?lasttime-tstarttime:((Bps>0)?(botot-obdone)/(Bps):-1));
//	fflush(stdout);//@@@@
//}


void c_prot_nntp::nntp_doarticle(arinfo*ari,arinfo*toti,char *fn){
//	long bytes=0,lines=0;
	int header=1;
	time_t curt,lastt=0;
	c_file_stream f;
	nntp_doopen();
	nntp_dogroup(0);
	putline(debug,"ARTICLE %li",ari->anum);
	stdgetreply(debug);
	list<char *> buf;
	list<char *>::iterator curb;
	char *p,*lp;
	time(&ari->starttime);
	curt=starttime;
	long glr;
	do {
		glr=getline(debug);
		if (cbuf[0]=='.'){
			if(cbuf[1]==0)
				break;
			lp=cbuf+1;
		}else
			lp=cbuf;
		ari->bytesdone+=glr;
		ari->linesdone++;
		if (toti){
			toti->bytesdone+=glr;
//			if (!header)
//				toti->linesdone++;
		}
		if (header && lp[0]==0){
//			printf("\ntoasted header statssssssss\n");
			header=0;
			ari->linesdone=0;
			time(&ari->starttime);//bytes=0;
		}
		time(&curt);
		if (!quiet && curt>lastt){
	//		nntp_print_retrieving_articles(num,ltotal,lines,btotal,bytes);
			ari->print_retrieving_articles(curt,toti);
			lastt=curt;
		}
		p=new char[strlen(lp)+1];
		strcpy(p,lp);
		buf.push_back(p);
	}while(1);
	if (!quiet){
		//nntp_print_retrieving_articles(num,ltotal,lines,btotal,bytes);
		ari->print_retrieving_articles(curt,toti);
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
//	if (!quiet) printf("uu_busy(%i):%s %i%%\r",p->action,p->curfile,(100*p->partno-p->percent)/p->numparts);
	if (!quiet) {printf(".");fflush(stdout);}
	return 0;
};

void c_prot_nntp::nntp_queueretrieve(const char *match, int linelimit, int getflags){
	if (!(filec=gcache->getfiles(filec,match,linelimit,getflags)))
		throw new c_error(EX_U_WARN,"nntp_retrieve: no match for %s",match);
}
void c_prot_nntp::nntp_retrieve(int doit){
	if (!(filec) || filec->files.size()<=0)
		return;
	
	t_nntp_files_u::iterator curf;
	c_nntp_file_u *f;
	if (!doit){
		c_nntp_cache_item *i;
		for(curf = filec->files.begin();curf!=filec->files.end();++curf){
			f=(*curf).second;
//			p=f->parts.begin()->second;
			i=gcache->items[f->banum];
			//printf("%i/%i\t(%liB,%lil)\t%li\t%s\n",f->have,f->req,f->bytes,f->lines,p->i->date,p->i->subject.c_str());
			printf("%i/%i\t%lil\t%li\t%s\t%s\n",f->have,f->req,f->lines,i->date,i->subject.c_str(),i->email.c_str());
		}
	}
	
	delete gcache;gcache=NULL;//we don't need this anymore, so free up some (a lot) of mem

	if (doit){
		arinfo qtotinfo,ainfo;
		time(&qtotinfo.starttime);
		qtotinfo.linesdone=0;
//		qtotinfo.linestot=filec->lines;
		qtotinfo.linestot=filec->files.size();
		qtotinfo.bytesdone=0;
		qtotinfo.bytestot=filec->bytes;
		s_part_u *p;
//		s_part_u *bp;
		t_nntp_file_u_parts::iterator curp;
		t_nntp_files_u::iterator lastf=filec->files.end();
		char *fn;
		for(curf = filec->files.begin();curf!=filec->files.end();++curf){
			int r;
			if (lastf!=filec->files.end()){
				delete (*lastf).second;
				filec->files.erase(lastf);
				qtotinfo.linesdone++;
			}
			lastf=curf;
			f=(*curf).second;
//			bp=f->parts.begin()->second;
			list<char *> fnbuf;
			list<char *>::iterator fncurb;
			for(curp = f->parts.begin();curp!=f->parts.end();++curp){
				//asprintf(&fn,"%s/%s-%s-%li-%li-%li",nghome.c_str(),host.c_str(),group.c_str(),fgnum,part,num);
				p=(*curp).second;
				asprintf(&fn,"./%s-%s-%li-%03li-%li",host.c_str(),group.c_str(),f->banum,(*curp).first,p->anum);
				if (!fexists(fn)){
					ainfo.anum=p->anum;
					ainfo.linesdone=0;
					ainfo.bytesdone=0;
					ainfo.linestot=p->lines;
					ainfo.bytestot=p->size;
					nntp_doarticle(&ainfo,&qtotinfo,fn);
				}else
					qtotinfo.bytestot-=p->size;
//				UULoadFile(fn,NULL,0);//load once they are all d/l'd
				//				delete fn;
				fnbuf.push_back(fn);
			}
			if ((r=UUInitialize())!=UURET_OK)
				throw new c_error(EX_A_FATAL,"UUInitialize: %s",UUstrerror(r));
			UUSetOption(UUOPT_DUMBNESS,1,NULL);//we already put the parts in the correct order, so it doesn't need to.
			//UUSetOption(UUOPT_FAST,1,NULL);//we store each message in a seperate file
			//actually, I guess that won't work, since some messages have multiple files in them anyway.
			UUSetOption(UUOPT_OVERWRITE,0,NULL);//no thanks.
			UUSetOption(UUOPT_USETEXT,1,NULL);//######hmmm...
			UUSetMsgCallback(NULL,uu_msg_callback);
			UUSetBusyCallback(NULL,uu_busy_callback,1000);
			for(fncurb = fnbuf.begin();fncurb!=fnbuf.end();++fncurb){
				UULoadFile((*fncurb),NULL,0);
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
//				printf("\ns:%x d:%x\n",uul->state,uul->uudet);
				if (uul->uudet==PT_ENCODED && strcmp(uul->filename,"0001.txt")==0){
					char *nfn;
					asprintf(&nfn,"%i.txt",f->banum);//give it a (somewhat) more sensible name
					UURenameFile(uul,nfn);
					delete nfn;
				}
				r=UUDecodeFile(uul,NULL);
				if (r==UURET_EXISTS){
					char *nfn;
					asprintf(&nfn,"%s.%i.%i",uul->filename,f->banum,rand());
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
				printf(" %i decoding errors occured, keeping temp files.\n",derr);
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
	}
	delete filec;filec=NULL;
}

void c_prot_nntp::nntp_open(const char *h){
	nntp_close();
	host=h;
}

void c_prot_nntp::nntp_doopen(void){
	//if (ch<0){
	if (!sock.isopen()){
		time(&lasttime);
		int i;
	//	if((ch=make_connection("nntp",SOCK_STREAM,host.c_str(),NULL,cbuf,cbuf_size))<0){
		if((i=sock.open(host.c_str(),"nntp"))<0){
			//throw -1;
			throw new c_error(EX_T_FATAL,"make_connection:%i %s(%i)",i,strerror(errno),errno);
		}
		stdgetreply(1);
		putline(debug,"MODE READER");
		getline(debug);
		groupselected=0;
	}
}

void c_prot_nntp::nntp_close(void){
//	if(ch>=0){
//		close(ch);ch=-1;
//	}
	sock.close();
}

void c_prot_nntp::cleanupcache(void){
	if(gcache){delete gcache;gcache=NULL;}
	if(filec){delete filec;filec=NULL;}
}
void c_prot_nntp::cleanup(void){
//	if (ch>=0){
	if (sock.isopen()){
		putline(debug,"QUIT");
		nntp_close();
	}
//	if (cbuf){delete cbuf;cbuf=NULL;}
	cleanupcache();
}

c_prot_nntp::c_prot_nntp(void){
//	cbuf=new char[4096];
//	cbuf_size=4096;
	gcache=NULL;
//	ch=-1;
	sock.initrbuf(4096);
	filec=NULL;
}
c_prot_nntp::~c_prot_nntp(void){
//	printf("nntp destructing\n");
	cleanup();
}
