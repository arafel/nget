/*
    prot_nntp.* - nntp protocol handler
    Copyright (C) 1999  Matthew Mueller <donut@azstarnet.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include "prot_nntp.h"
#include <list.h>
#include <stdio.h>
#include "sockstuff.h"
#ifdef HAVE_UUDEVIEW_H
#include <uudeview.h>
#else
#include "uudeview.h"
#endif
#include "misc.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "log.h"
#include "file.h"
#include "nget.h"

int c_prot_nntp::putline(int echo,const char * str,...){
	va_list ap;
	va_start(ap,str);
	int i=doputline(echo,str,ap);
	va_end(ap);
	return i;
}
int c_prot_nntp::stdputline(int echo,const char * str,...){
	va_list ap;
	int i;
	va_start(ap,str);
	doputline(echo,str,ap);
	va_end(ap);
	i=getreply(echo);
	if (i==450 || i==480) {
		nntp_auth();
		va_start(ap,str);
		doputline(echo,str,ap);
		va_end(ap);
		i=getreply(echo);
	}
	return i;
}
int c_prot_nntp::doputline(int echo,const char * str,va_list ap){
//	static char fpbuf[255];
	char *fpbuf;
	int i;
//	va_list ap;

//	va_start(ap,str);
	i=vasprintf(&fpbuf,str,ap);
//	va_end(ap);
	if (!(fpbuf=(char*)realloc(fpbuf,i+3))){
		delete(fpbuf);
		nntp_close();
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
		nntp_close(1);
		throw new c_error(EX_T_ERROR,"nntp_putline:%i %s(%i)",i,strerror(errno),errno);
	}
	delete(fpbuf);
	return i;
}

int c_prot_nntp::getline(int echo){
	//int i=sock_gets(ch,cbuf,cbuf_size);
	int i=sock.bgets();
	if (i<=0){
		nntp_close(1);
		throw new c_error(EX_T_ERROR,"nntp_getline:%i %s(%i)",i,strerror(errno),errno);
	}else {
		cbuf=sock.rbufp;
		time(&lasttime);
		if (echo)
			printf("%s\n",cbuf);
	}
	return i;
}

int c_prot_nntp::chkreply(int reply){
//	int i=getreply(echo);
	if (reply/100!=2)
		throw new c_error(EX_P_FATAL,"bad reply %i: %s",reply,cbuf);
	return reply;
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
	char *t[10];
	long bytes=0,realnum=0;
	int i,lowest=-1,dcounter=0,dtrip=10,dtripmax=50,dtripmin=2,dtripdiv=50;
	nntp_doopen();
	nntp_dogroup(0);
	if (!quiet){
		time(&starttime);
		nntp_print_retrieving_headers(low,high,low,0);
	}
	chkreply(stdputline(debug>=DEBUG_MED,"XOVER %i-%i",low,high));
	time(&starttime);
	time(&curt);
	//stdgetreply(debug);
	unsigned long an=0;
	c_nntp_header nh;
	char * tp;
	do {
		bytes+=getline(debug>=DEBUG_ALL);
		if (cbuf[0]=='.')break;
		p=cbuf;
		for(i=0;i<10;i++){
			if((t[i]=goodstrtok(&p,'\t'))==NULL){
				break;
			}
			// fields 0, 6, 7 must all be numeric
			// otherwise restart
			if (i==0 || i==6 || i==7){
				tp=t[i];
				while (*tp){
					if (!isdigit(*tp))
						break;
					tp++;
				}
				// did the test finish, and/or was it a blank field?
				if (*tp && tp!=t[i]){
					// no - get out and read the next line
					// Is this how we want to handle it? SMW
					printf("error retrieving xover (%i non-numeric): ",i);
					for (int j=0;j<i;j++)
						printf("%i:%s ",j,t[j]);
					printf("*:%s\n",p);
					break;
				}
			}
		}
		if (i>7){
		//	c=new c_nntp_cache_item(atol(t[0]),	decode_textdate(t[3]), atol(t[6]), atol(t[7]),t[1],t[2]);
			//gcache->additem(c);
			an=atoul(t[0]);
			nh.set(t[1],t[2],an,decode_textdate(t[3]),atoul(t[6]),atoul(t[7]));
			//gcache->additem(an, decode_textdate(t[3]), atol(t[6]), atol(t[7]),t[1],t[2]);
			gcache->additem(&nh);
			//delete nh;
			realnum++;
			if (lowest==-1){
				lowest=an;
				dtrip=(high-lowest)/dtripdiv;
				if (dtrip>dtripmax)
					dtrip=dtripmax;
				else if (dtrip<dtripmin)
					dtrip=dtripmin;
			}
			dcounter++;
			time(&curt);
			if (!quiet && dcounter>=dtrip && curt>lastt){
				nntp_print_retrieving_headers(lowest,high,an,bytes);
				dcounter=0;
				time(&lastt);
			}
		}else{
			printf("error retrieving xover (%i<=7): ",i);
			for (int j=0;j<i;j++)
				printf("%i:%s ",j,t[j]);
			printf("*:%s\n",p);
			break;
		}
	}while(1);
	if(!quiet && an){
		nntp_print_retrieving_headers(lowest,high,an,bytes);
		printf("(%li)\n",realnum);
	}
}

void c_prot_nntp::nntp_dogroup(int getheaders){
	if(!gcache && getheaders){
		throw new c_error(EX_U_FATAL,"nntp_dogroup: nogroup selected");
	}
	nntp_doopen();
	if (groupselected) return;
	chkreply(stdputline(1,"GROUP %s",group.c_str()));
//	stdgetreply(1);
	groupselected=1;
	char *p;
	ulong num,low,high;
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
		if (low>0){
			ulong l=(high-low)/4;//keep 1/4 the current number of headers worth of old read info.. make configureable?
			if (low>l){
				l=low-l-1;
				grange->remove(0,l);
			}
//			else
//				l=0;
		}
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
	grange=new c_nrange(nghome + "/" + host + "/" + group + "_r");
	gcache=new c_nntp_cache(nghome,host,group);
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


int c_prot_nntp::nntp_doarticle(arinfo*ari,arinfo*toti,char *fn){
//	long bytes=0,lines=0;
	int header=1;
	time_t curt,lastt=0;
	//c_file_stream f;
	c_file_fd f;
	nntp_doopen();
	nntp_dogroup(0);
	if (stdputline(debug>=DEBUG_MED,"ARTICLE %li",ari->anum)/100!=2){
		printf("error retrieving article %li: %s\n",ari->anum,cbuf);
		return -1;
	}
	list<char *> buf;
	list<char *>::iterator curb;
	char *p,*lp;
	time(&ari->starttime);
	curt=starttime;
	long glr;
	do {
		glr=getline(debug>=DEBUG_ALL);
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
	if ((ari->bytesdone!=ari->bytestot && ari->bytesdone+1!=ari->bytestot) || ari->linesdone!=ari->linestot){//I seem to get a lot of times where I retrieve one byte less than it says there is.
		printf("doarticle %lu: %lu!=%lu || %lu!=%lu\n",ari->anum,ari->bytesdone,ari->bytestot,ari->linesdone,ari->linestot);
	}
	if (!f.open(fn,O_CREAT|O_WRONLY|O_TRUNC,PRIVMODE)){
		for(curb = buf.begin();curb!=buf.end();++curb){
			p=(*curb);
			if (f.putf("%s\n",p)<=0)
				throw new c_error(EX_A_FATAL,"error writing %s: %s",fn,strerror(errno));
			delete p;
		}
		f.close();
	}else
		throw new c_error(EX_A_FATAL,"unable to open %s: %s",fn,strerror(errno));
	return 0;
}

int uu_info_callback(void *v,char *i){
	printf("info:%s",i);
	return 0;
};
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
	if (!(filec=gcache->getfiles(filec,grange,match,linelimit,getflags)))
		throw new c_error(EX_U_WARN,"nntp_retrieve: no match for %s",match);
}

void c_prot_nntp::nntp_retrieve(int doit){
	if (!(filec) || filec->files.empty())
		return;
	
	t_nntp_files_u::iterator curf;
	c_nntp_file *f;

	//hmm, maybe not now. //well, now we need to keep it around again, so we can set the read flag.  At least with the new cache implementation its not quite such a memory hog.
	if (gcache){
		delete gcache;gcache=NULL;//we don't need this anymore, so free up some (a lot) of mem
	}

	if (!doit){
		c_nntp_part *p;
		for(curf = filec->files.begin();curf!=filec->files.end();++curf){
			f=(*curf).second;
			p=(*f->parts.begin()).second;
			//printf("%i/%i\t(%liB,%lil)\t%li\t%s\n",f->have,f->req,f->bytes,f->lines,p->i->date,p->i->subject.c_str());
			printf("%li\t%i/%i\t%lil\t%li\t%s\t%s\n",p->articlenum,f->have,f->req,f->lines,p->date,f->subject.c_str(),f->author.c_str());
		}
	}

	if (doit){
#ifdef SHORT_TEMPNAMES
		t_id group_id;
# ifdef CHECKSUM
		group_id=CHECKSUM(0L, Z_NULL, 0);
		//group_id=CHECKSUM(group_id,host.c_str(),host.size());
		group_id=CHECKSUM(group_id,(Byte*)group.c_str(),group.size());
# else
		hash<const char*> H;
		group_id=H(group.c_str());
# endif
#endif
		arinfo qtotinfo,ainfo;
		time(&qtotinfo.starttime);
		qtotinfo.linesdone=0;
//		qtotinfo.linestot=filec->lines;
		qtotinfo.linestot=filec->files.size();
		qtotinfo.bytesdone=0;
		qtotinfo.bytestot=filec->bytes;
		c_nntp_part *p;
//		s_part_u *bp;
		t_nntp_file_parts::iterator curp;
		t_nntp_files_u::iterator lastf=filec->files.end();
		char *fn;
		for(curf = filec->files.begin();curf!=filec->files.end();++curf){
			int r;
			if (lastf!=filec->files.end()){
//				delete (*lastf).second;//new cache implementation uses pointers to the same data
				filec->files.erase(lastf);
				qtotinfo.linesdone++;
			}
			lastf=curf;
			f=(*curf).second;
//			bp=f->parts.begin()->second;
			list<char *> fnbuf;
			list<char *>::iterator fncurb;
			int derr=0;
			int un=0;
			for(curp = f->parts.begin();curp!=f->parts.end();++curp){
				//asprintf(&fn,"%s/%s-%s-%li-%li-%li",nghome.c_str(),host.c_str(),group.c_str(),fgnum,part,num);
				p=(*curp).second;
				if (derr){
					qtotinfo.bytestot-=p->bytes;
					continue;
				}
#ifdef SHORT_TEMPNAMES
				//asprintf(&fn,"./%lx-%lx.%03li-%li",group_id,f->sub_id,(*curp).first,p->anum);
				//asprintf(&fn,"./%lx-%lx.%03li",group_id,H(f->subject.c_str()),(*curp).first);
				asprintf(&fn,"./%lx-%lx.%03li",group_id,f->mid,(*curp).first);
#else
				asprintf(&fn,"./%s-%s-%li-%03li-%li",host.c_str(),group.c_str(),f->banum(),(*curp).first,p->articlenum);
#endif
				if (!fexists(fn)){
					ainfo.anum=p->articlenum;
					ainfo.linesdone=0;
					ainfo.bytesdone=0;
					ainfo.linestot=p->lines;
					ainfo.bytestot=p->bytes;
					if (nntp_doarticle(&ainfo,&qtotinfo,fn)){
						delete fn;
						qtotinfo.bytestot-=p->bytes;
						derr=-1;//skip this file..
						continue;
					}
				}else{
					qtotinfo.bytestot-=p->bytes;
					if (!quiet) printf("already have article %li\n",p->articlenum);
				}
//				UULoadFile(fn,NULL,0);//load once they are all d/l'd
				//				delete fn;
				fnbuf.push_back(fn);
			}
			if (!derr){
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
				for (un=0;;un++){
					if ((uul=UUGetFileListItem(un))==NULL)break;
					if (!(uul->state & UUFILE_OK)){
						printf("%s not ok\n",uul->filename);
						derr++;
						continue;
					}
					//				printf("\ns:%x d:%x\n",uul->state,uul->uudet);
					if (/*uul->uudet==PT_ENCODED &&*/ strcmp(uul->filename,"0001.txt")==0){
						char *nfn;
						asprintf(&nfn,"%i.txt",f->banum());//give it a (somewhat) more sensible name
						UURenameFile(uul,nfn);
						delete nfn;
					}else
						r=UUInfoFile(uul,NULL,uu_info_callback);
					r=UUDecodeFile(uul,NULL);
					if (r==UURET_EXISTS){
						char *nfn;
						asprintf(&nfn,"%s.%i.%i",uul->filename,f->banum(),rand());
#ifdef	USE_FILECOMPARE	// We have a duplicate file name
						// memorize the old file name
						char	*old_fn;
						asprintf(&old_fn,"%s",uul->filename);
						// Generate a new filename to use
						UURenameFile(uul,nfn);
						r=UUDecodeFile(uul,NULL);
						// did it decode correctly?
						if (r == UURET_OK){
							// if identical, delete the one we just downloaded
							if (filecompare(old_fn,nfn) > 0){
								printf("Duplicate File Removed %s\n", nfn);
								unlink(nfn);
							}
						}
						// cleanup
						delete	old_fn;

#else	/* USE_FILECOMPARE */				// the orginal code
						UURenameFile(uul,nfn);
						r=UUDecodeFile(uul,NULL);
#endif	/* USE_FILECOMPARE */
						delete nfn;
					}
					if (r!=UURET_OK){
						derr++;
						printf("decode(%s): %s\n",uul->filename,UUstrerror(r));
						continue;
					}else{
//						printf("flags %lx",f->flags);
//						if (!(f->flags&FILEFLAG_READ)){//if the flag is already set, don't force a save
//							printf("&=%i",FILEFLAG_READ);
//							f->flags|=FILEFLAG_READ;
							grange->insert(f->banum());// should insert numbers for all parts, or not?  If all parts are sequential, it could actually reduce space, however if they aren't it would just waste a bunch of space.  The first num is the only one thats currently checked, so I guess I'll just leave it at that for now.
//							gcache->saveit=1;//make sure the flags get saved, even if the header file hasn't changed otherwise
//						}
//						printf("->%lx\n",f->flags);
					}
				}
				UUCleanUp();
			}
			if (derr>0)
				printf(" %i decoding errors occured, keeping temp files.\n",derr);
			else if (derr<0 && fnbuf.size())
				printf("download error occured, keeping temp files.\n");
			else if (un==0){
				printf("hm.. nothing decoded.. keeping temp files\n");
				derr=-2;
			}else if (derr==0 && !quiet)
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
	//delete filec;filec=NULL;
	cleanupcache();
}
void c_prot_nntp::nntp_auth(void){
	if (user.size()<=0){
		string fn=nghome+"/"+host+"/.authinfo";
		c_file_fd f;
		f.initrbuf(256);
		if (!f.open(fn.c_str(),O_RDONLY)){
//			char buf[256];
			if (f.bgets()){
				user=f.rbufp;
				if (f.bgets()){
//					buf[strlen(buf)]=0;
					pass=f.rbufp;
				}
			}
			f.close();
		}
	}
	nntp_doauth(user.c_str(),pass.c_str());
}
void c_prot_nntp::nntp_doauth(const char *user, const char *pass){
	int i;
	
	if(!user){
		nntp_close();
		throw new c_error(EX_U_FATAL,"nntp_doauth: no authorization info known");
	}
	putline(1,"AUTHINFO USER %s",user);
	i=getreply(1);
	if (i==350 || i==381){ 
		if(!pass){
			nntp_close();
			throw new c_error(EX_U_FATAL,"nntp_doauth: no password known");
		}
		putline(0,"AUTHINFO PASS %s",pass);
		printf(">AUTHINFO PASS *\n");
		i=getreply(1);
	}
	chkreply(i);
	authed=1;
}

void c_prot_nntp::nntp_open(const char *h,const char *u,const char *p){
	nntp_close();
	host=h;
	if (u)
		user=u;
	if (p)
		pass=p;
}

void c_prot_nntp::nntp_doopen(void){
	//if (ch<0){
	if (!sock.isopen()){
		if(host.size()<=0){
			throw new c_error(EX_U_FATAL,"nntp_doopen: no host selected");
		}
		time(&lasttime);
		int i;
	//	if((ch=make_connection("nntp",SOCK_STREAM,host.c_str(),NULL,cbuf,cbuf_size))<0){
		if((i=sock.open(host.c_str(),"nntp"))<0){
			//throw -1;
			//throw new c_error(EX_T_FATAL,"make_connection:%i %s(%i)",i,strerror(errno),errno);
			throw new c_error(EX_T_ERROR,"make_connection:%i %s(%i)",i,strerror(errno),errno);
		}
		chkreply(getreply(1));
		putline(debug>=DEBUG_MED,"MODE READER");
		getline(debug>=DEBUG_MED);
		groupselected=0;
		authed=0;
	}
}

void c_prot_nntp::nntp_close(int fast){
//	if(ch>=0){
//		close(ch);ch=-1;
//	}
	if (sock.isopen()){
		if (!fast)
			putline(1,"QUIT");
			//putline(debug,"QUIT");
		sock.close();
	}
}

void c_prot_nntp::cleanupcache(void){
	if(grange){delete grange;grange=NULL;}
	if(gcache){delete gcache;gcache=NULL;}
	if(filec){delete filec;filec=NULL;}
}
void c_prot_nntp::cleanup(void){
//	if (ch>=0){
	nntp_close();
//	if (sock.isopen()){
//		putline(debug,"QUIT");
//		nntp_close();
//	}
//	if (cbuf){delete cbuf;cbuf=NULL;}
	cleanupcache();
}

c_prot_nntp::c_prot_nntp(void){
//	cbuf=new char[4096];
//	cbuf_size=4096;
	grange=NULL;
	gcache=NULL;
//	ch=-1;
	sock.initrbuf(4096);
	filec=NULL;
	authed=0;
}
c_prot_nntp::~c_prot_nntp(void){
//	printf("nntp destructing\n");
	cleanup();
}
