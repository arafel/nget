/*
    cache.* - nntp header cache code
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
#include "myregex.h"
#include "cache.h"
#include "misc.h"
#include "log.h"
#include SLIST_H
#include <glob.h>
#include <errno.h>
#include "nget.h"

int c_nntp_header::parsepnum(const char *str,const char *soff){
	const char *p;
	if ((p=strpbrk(soff+1,")]"))){
		char m,m2=*p;
		if (m2==')') m='(';
		else m='[';
		tailoff=p-str;
		for(p=soff;p>str;p--)
			if (*p==m){
				p++;
				char *erp;
				partoff=p-str;
				partnum=strtol(p,&erp,10);
				if (*erp!='/' || erp==p) return -1;
				int req=strtol(soff+1,&erp,10);
				if (*erp!=m2 || erp==soff+1) return -1;
				//                                      if (partnum==0)
				//                                              partnum=-1;
				return req;
			}
	}
	return -1;
}

void c_nntp_header::setmid(void){
#ifdef CHECKSUM
	mid=CHECKSUM(0L, Z_NULL, 0);
	mid=CHECKSUM(mid,(Byte*)subject.c_str(),subject.size());
	mid=CHECKSUM(mid,(Byte*)author.c_str(),author.size());
#else
	hash<char *> H;
	mid=H(subject.c_str())+H(author.c_str());//prolly not as good as crc32, but oh well.
#endif
}
void c_nntp_header::set(char * str,const char *a,ulong anum,time_t d,ulong b,ulong l){
	author=a;articlenum=anum;date=d;bytes=b;lines=l;
	const char *s=str+strlen(str)-3;//-1 for null, -2 for ), -3 for num
	req=0;
	for (;s>str;s--) {
		if (*s=='/')
			if ((req=parsepnum(str,s))>=0){
				subject="";
				subject.append(str,partoff);
				subject.append("*");
				subject.append(s);
				setmid();
				return;
			}
	}
	partoff=-1;tailoff=-1;
	partnum=0;
	subject=str;
	setmid();
}



c_nntp_part::c_nntp_part(c_nntp_header *h):partnum(h->partnum),articlenum(h->articlenum),date(h->date),bytes(h->bytes),lines(h->lines){}



void c_nntp_file::addpart(c_nntp_part *p){
	parts[p->partnum]=p;
	if (p->partnum>0 && p->partnum<=req) have++;
	bytes+=p->bytes;lines+=p->lines;
}

c_nntp_file::c_nntp_file(int r,ulong f,t_id mi,const char *s,const char *a,int po,int to):req(r),have(0),bytes(0),lines(0),flags(f),mid(mi),subject(s),author(a),partoff(po),tailoff(to){
}
c_nntp_file::c_nntp_file(c_nntp_header *h):req(h->req),have(0),bytes(0),lines(0),flags(0),mid(h->mid),subject(h->subject),author(h->author),partoff(h->partoff),tailoff(h->tailoff){
}

c_nntp_file::~c_nntp_file(){
	t_nntp_file_parts::iterator i;
	for(i = parts.begin();i!=parts.end();++i){
		delete (*i).second;
	}
}


class file_match {
	public:
		c_regex reg;
		ulong size;
		file_match(const char *m,int a):reg(m,a){};
};
typedef slist<file_match *> filematchlist;
//typedef slist<char *> filematchlist;
typedef slist<ulong> longlist;

#define ALNUM "a-zA-Z0-9"
void buildflist(filematchlist **l,longlist **a){
	glob_t globbuf;
	globbuf.gl_offs = 0;
	glob("*",0,NULL,&globbuf);
	*l=NULL;
	*a=NULL;
	if (globbuf.gl_pathc<=0)return;
	*l=new filematchlist;
//	char *s;
	struct stat stbuf;
//	c_regex *s;
	file_match *fm;
	c_regex amatch("^[0-9]+\\.txt",REG_EXTENDED|REG_ICASE|REG_NOSUB);
	char buf[1024],*cp;
	int sl;
	for (int i=0;i<globbuf.gl_pathc;i++){
/*		asprintf(&s,"*[!"ALNUM"]%s[!"ALNUM"]*",globbuf.gl_pathv[i]);
		(*l)->push_front(s);
		asprintf(&s,"*[!"ALNUM"]%s",globbuf.gl_pathv[i]);
		(*l)->push_front(s);
		asprintf(&s,"%s[!"ALNUM"]*",globbuf.gl_pathv[i]);*/
		//no point in using fnmatch.. need to do this gross multi string per file kludge, and...
		//sprintf(buf,"(^|[^[:alnum:]]+)%s([^[:alnum:]]+|$)",globbuf.gl_pathv[i]);//this is about the same speed as the 3 fnmatchs
//		sl=sprintf(buf,"\\<%s\\>",globbuf.gl_pathv[i]);//this is much faster
//		cp=buf+2;
//		while ((cp=strpbrk(cp,"()|[]"))&&cp-buf<sl-2)
//			*cp='.';//filter out some special chars.. really should just escape them, but thats a bit harder
		sl=0;
		buf[sl++]='\\';buf[sl++]='<';//match beginning of word
		cp=globbuf.gl_pathv[i];
		while (*cp){
			if (strchr("()|[]\\.+*^$",*cp))
				buf[sl++]='\\';//escape special chars
			buf[sl++]=*cp;
			cp++;
		}
		buf[sl++]='\\';buf[sl++]='>';//match end of word
		buf[sl++]=0;
//		printf("%s\n",buf);//this is much faster
//		s=new c_regex(buf,REG_EXTENDED|REG_ICASE|REG_NOSUB);
		fm=new file_match(buf,REG_EXTENDED|REG_ICASE|REG_NOSUB);
		if(stat(globbuf.gl_pathv[i],&stbuf)==0)
			fm->size=stbuf.st_size;
		else
			fm->size=0;
		(*l)->push_front(fm);
		if (!amatch.match(globbuf.gl_pathv[i])){
			if (*a==NULL)
				*a=new longlist;
			(*a)->push_front(atoul(globbuf.gl_pathv[i]));
		}
	}
	globfree(&globbuf);
//	return l;
}

//gnu extension.. if its not available, just compare normally.
//#ifndef FNM_CASEFOLD
//#define FNM_CASEFOLD 0
//#endif

int checkhavefile(filematchlist *fl,longlist *al,const char *f,ulong anum,ulong bytes){
	filematchlist::iterator curl;
	longlist::iterator cura;
	if (al)
		for (cura=al->begin();cura!=al->end();++cura){
			if (*cura==anum){
				printf("already have(%lu) %s\n",anum,f);
				return 1;
			}
		}
	if (fl){
		file_match *fm;
		for (curl=fl->begin();curl!=fl->end();++curl){
			//		printf("fnmatch(%s,%s,%i)=",*curl,f,FNM_CASEFOLD);
//			if (fnmatch(*curl,f,FNM_CASEFOLD)==0){
			fm=*curl;
			if (fm->reg.match(f)==0 && fm->size*2>bytes && fm->size<bytes){
				//			printf("0\n");
				printf("already have %s\n",f);
				return 1;
			}
			//		printf("1\n");
		}
	}
	return 0;
}


c_nntp_files_u* c_nntp_cache::getfiles(c_nntp_files_u * fc,c_nrange *grange,const char *match, unsigned long linelimit,int flags){
	if (fc==NULL) fc=new c_nntp_files_u;
	c_regex hreg(match,REG_EXTENDED + ((flags&GETFILES_CASESENSITIVE)?0:REG_ICASE));

	filematchlist *flist=NULL;
	longlist *alist=NULL;
	if (!(flags&GETFILES_NODUPECHECK))
		buildflist(&flist,&alist);

	t_nntp_files::const_iterator fi;
	c_nntp_file *f;
	ulong banum;
	for(fi = files.begin();fi!=files.end();++fi){
		f=(*fi).second;
		banum=f->banum();
		//if (f->lines>=linelimit && (flags&GETFILES_NODUPECHECK || !(f->flags&FILEFLAG_READ)) && (f->have>=f->req || flags&GETFILES_GETINCOMPLETE) && !hreg.match(f->subject.c_str())){//matches user spec
		if (f->lines>=linelimit && (flags&GETFILES_NODUPECHECK || !(grange->check(banum))) && (f->have>=f->req || flags&GETFILES_GETINCOMPLETE) && !hreg.match(f->subject.c_str())){//matches user spec
//			fc->additem(i);
//			if (!(flags&GETFILES_NODUPECHECK) && f->isread())
//				continue;
//			banum=f->banum();
			if (fc->files.find(banum)!=fc->files.end())
				continue;
			if (!(flags&GETFILES_NODUPECHECK) && checkhavefile(flist,alist,f->subject.c_str(),banum,f->bytes))
				continue;
			f->inc_rcount();
			fc->files[banum]=f;
			fc->lines+=f->lines;
			fc->bytes+=f->bytes;
		}
	}
	if (flist){
		filematchlist::iterator curl;
		for (curl=flist->begin();curl!=flist->end();++curl)
			delete *curl;
		delete flist;
	}
	if (alist)delete alist;
//	if (!nm){
//		delete fc;fc=NULL;
//	}

	return fc;
}

int c_nntp_cache::additem(c_nntp_header *h){
	c_nntp_part *p=new c_nntp_part(h);
	c_nntp_file *f;
	t_nntp_files::iterator i;
	pair<t_nntp_files::iterator, t_nntp_files::iterator> irange = files.equal_range(h->mid);
//	t_nntp_files::const_iterator i;
//	pair<t_nntp_files::const_iterator, t_nntp_files::const_iterator> irange = files.equal_range(h->mid);
	if (h->articlenum>high)
		high=h->articlenum;
	if (h->articlenum<low)
		low=h->articlenum;
	saveit=1;
	num++;
//	printf("%lu %s..",h->articlenum,h->subject.c_str());
	for (i=irange.first;i!=irange.second;++i){
		f=(*i).second;
		if (f->req==h->req && f->partoff==h->partoff && f->tailoff==h->tailoff){
			//these two are merely for debugging.. it shouldn't happen (much..? ;)
			if (!(f->author==h->author)){//older (g++) STL versions seem to have a problem with strings and !=
				printf("%lu->%lu was gonna add, but author is different?\n",h->articlenum,f->banum());
				continue;
			}
			if (!(f->subject==h->subject)){
				printf("%lu->%lu was gonna add, but subject is different?\n",h->articlenum,f->banum());
				continue;
			}
			if (f->parts.find(p->partnum)!=f->parts.end()){
//				printf("%lu was gonna add, but already have this part?\n",h->articlenum);
				continue;
			}
//			printf("adding\n");
			f->addpart(p);
			return 0;
		}
	}
//	printf("new\n");
	f=new c_nntp_file(h);
	f->addpart(p);
	//files[f->subject.c_str()]=f;
	files.insert(t_nntp_files::value_type(f->mid,f));
	return 1;
}

ulong c_nntp_cache::flushlow(ulong newlow){
	ulong count=0;
	t_nntp_files::iterator i,in;
	c_nntp_file *nf;
	t_nntp_file_parts::iterator pi,pic;
	c_nntp_part *np;
	if (!quiet) {printf("Flushing headers %lu-%lu(%lu):",low,newlow-1,newlow-low);fflush(stdout);}
	for(in = files.begin();in!=files.end();){
		i=in;
		++in;
		nf=(*i).second;
		for(pi = nf->parts.begin();pi!=nf->parts.end();){
			pic=pi;
			++pi;
			np=(*pic).second;
			if (np->articlenum<newlow){
				delete np;
				nf->parts.erase(pic);
				count++;
			}
		}
		if (nf->parts.empty()){
			nf->dec_rcount();
//			delete nf;
			files.erase(i);
		}
	}
	num-=count;
	low=newlow;
	if (!quiet){printf(" %lu\n",count);}

	return count;
}

c_nntp_cache::c_nntp_cache(string path,string hid,string nid):cdir(path),high(0),low(ULONG_MAX),num(0){
#ifdef HAVE_LIBZ
	c_file_gz f;
#else
	c_file_fd f;
#endif
	f.initrbuf(2048);
	saveit=0;
	int r=testmkdir(cdir.c_str(),S_IRWXU);
	if (r) throw new c_error(EX_A_FATAL,"error creating dir %s: %s(%i)",cdir.c_str(),strerror(r==-1?errno:r),r==-1?errno:r);
	cdir.append(hid);
	cdir.append("/");
	file=nid;
#ifdef HAVE_LIBZ
	file.append(".gz");
#endif
	fileread=0;
	if (!f.open((cdir + file).c_str(),
#ifdef HAVE_LIBZ
				"r"
#else
				O_RDONLY
#endif
				)){
		char *buf;
		int mode=2;
		c_nntp_file	*nf=NULL;
		c_nntp_part	*np;
		char *t[8];
		int i;
		fileread=1;
		ulong count=0,curline=0;
		while (f.bgets()>0){
			buf=f.rbufp;
			curline++;
			if (mode==2){
				for(i=0;i<4;i++)
					if((t[i]=goodstrtok(&buf,'\t'))==NULL){
						break;
					}
				if (i>=4 && (strcmp(t[0],CACHE_VERSION))==0){
					low=atoul(t[1]);high=atoul(t[2]);num=atoul(t[3]);
				}else{
					if (i>0 && strncmp(t[0],"NGET",4)==0)
						printf("%s is from a different version of nget\n",file.c_str());
					else
						printf("%s does not seem to be an nget cache file\n",file.c_str());
					f.close();fileread=0;
					return;
				}
				mode=0;
			}
			else if (mode==1 && nf){
				if (buf[0]=='.'){
					mode=0;
					continue;
				}else{
					for(i=0;i<5;i++)
						if((t[i]=goodstrtok(&buf,'\t'))==NULL){
							i=-1;break;
						}
					if (i>=5){
						np=new c_nntp_part(atoi(t[0]),atoul(t[1]),atoul(t[2]),atoul(t[3]),atoul(t[4]));
						nf->addpart(np);
						count++;
					}else
						printf("invalid line %lu mode %i\n",curline,mode);
				}
			}
			else if (mode==0){
				for(i=0;i<7;i++)
					if((t[i]=goodstrtok(&buf,'\t'))==NULL){
						i=-1;break;
					}
				if (i>=7){
					nf=new c_nntp_file(atoi(t[0]),atoul(t[1]),atoul(t[2]),t[3],t[4],atoi(t[5]),atoi(t[6]));
					files.insert(t_nntp_files::value_type(nf->mid,nf));
//					files[nf->subject.c_str()]=nf;
					mode=1;
				}else
					printf("invalid line %lu mode %i\n",curline,mode);
			}
		}
		if (count<num){
			printf("warning: read %lu parts from cache, expecting %lu\n",count,num);
		}
	}
}
c_nntp_cache::~c_nntp_cache(){
#ifdef HAVE_LIBZ
	c_file_gz f;
#else
	c_file_fd f;
#endif
	t_nntp_files::iterator i;
	if (saveit && (files.size() || fileread)){
		int r=testmkdir(cdir.c_str(),S_IRWXU);
		if (r) printf("error creating dir %s: %s(%i)\n",cdir.c_str(),strerror(r==-1?errno:r),r==-1?errno:r);
		else if(!(r=f.open((cdir + file).c_str(),
#ifdef HAVE_LIBZ
						"w"
#else
						O_CREAT|O_WRONLY|O_TRUNC,PUBMODE
#endif
						))){
			if (!quiet){printf("saving cache: %lu headers, %i files..",num,files.size());fflush(stdout);}
			c_nntp_file *nf;
			t_nntp_file_parts::iterator pi;
			c_nntp_part *np;
			ulong count=0;
			f.putf(CACHE_VERSION"\t%lu\t%lu\t%lu\n",low,high,num);
			for(i = files.begin();i!=files.end();++i){
				nf=(*i).second;
				f.putf("%i\t%lu\t%lu\t%s\t%s\t%i\t%i\n",nf->req,nf->flags,nf->mid,nf->subject.c_str(),nf->author.c_str(),nf->partoff,nf->tailoff);
				for(pi = nf->parts.begin();pi!=nf->parts.end();++pi){
					np=(*pi).second;
					f.putf("%i\t%lu\t%lu\t%lu\t%lu\n",np->partnum,np->articlenum,np->date,np->bytes,np->lines);
					count++;
				}
				f.putf(".\n");
				//nf->storef(f);
				//delete nf;
				nf->dec_rcount();
			}
			f.close();
			if (!quiet) printf(" done.\n");
			if (count<num){
				printf("warning: wrote %lu parts from cache, expecting %lu\n",count,num);
			}
			return;
		}else{
			printf("error opening %s: %s(%i)\n",(cdir + file).c_str(),strerror(errno),errno);
		}
	}

	if (!quiet){printf("freeing cache: %lu headers, %i files..",num,files.size());fflush(stdout);}
	for(i = files.begin();i!=files.end();++i){
		//delete (*i).second;
		(*i).second->dec_rcount();
	}
	if (!quiet) printf(" done.\n");
}
c_nntp_files_u::~c_nntp_files_u(){
	t_nntp_files_u::iterator i;
	for(i = files.begin();i!=files.end();++i){
		(*i).second->dec_rcount();
	}
}
