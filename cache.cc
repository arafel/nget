#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include "cache.h"
#include "misc.h"
#include "myregex.h"
#include "log.h"
#include <glob.h>
#ifdef HAVE_SLIST_H
#include <slist.h>
#endif
//#include <fnmatch.h>


#ifdef TEXT_CACHE
c_nntp_cache_item* c_nntp_cache_item::loadi(char *s){
	char *t[6], *p;
	int i;
	p=s;
	for(i=0;i<6;i++)
		if((t[i]=goodstrtok(&p,'\t'))==NULL){
			i=-1;return NULL;
		}
//	t[5].erase(t[5].size-1,t[5].size);//err, its not a basic_string, is it? heh.
	if ((p=strpbrk(t[5],"\n\r")))
		*p=0;
	c_nntp_cache_item *c=new c_nntp_cache_item(atol(t[0]),atol(t[1]),atol(t[2]),atol(t[3]),t[4],t[5]);
	return c;
};
#else
c_nntp_cache_item* c_nntp_cache_item::loadi(long l1, c_file *f, char *buf){
	long l[4];
	if (f->read(l,sizeof(long)*4)<=0)return NULL;
	if (f->read(buf,l[3])<=0)return NULL;
	buf[l[3]]=0;
	char *s2=buf+l[3]+2;
	if (f->read(&l[3],sizeof(long))<=0)return NULL;
	if (f->read(s2,l[3])<=0)return NULL;
	s2[l[3]]=0;
	c_nntp_cache_item *c=new c_nntp_cache_item(l1,l[0],l[1],l[2],buf,s2);
//	delete s;
//	delete s2;
	return c;
}
#endif

#ifdef DEBUG_CACHE_DESTRUCTION
static int dbgcount5=0;
#endif

c_nntp_cache_item::c_nntp_cache_item(long n,time_t d,long s, long l, char * a,char * e):subject(a), email(e){
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount5++;
	printf("c_nntp_cache_item::c_nntp_cache_item dbgcount=%i\n",dbgcount5);
#endif
	num=n;date=d;size=s;lines=l;
//	printf("new c_nntp_cache_item(%li,%li,%li,%li,%s,%s\n",n,d,s,l,a,e);
}
c_nntp_cache_item::~c_nntp_cache_item(){
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount5--;
//	subject.delete_c_str();email.delete_c_str();
	printf("c_nntp_cache_item::~c_nntp_cache_item dbgcount=%i\n",dbgcount5);
#endif
}

int c_nntp_cache_item::storei(c_file *f){
#ifdef TEXT_CACHE
	return f->putf("%li\t%li\t%li\t%li\t%s\t%s\n",num,date,size,lines,subject.c_str(),email.c_str());
#else
	f->write(&num,sizeof(num));
	f->write(&date,sizeof(date));
	f->write(&size,sizeof(size));
	f->write(&lines,sizeof(lines));
	long blag=subject.size();
	f->write(&blag,sizeof(blag));
	f->write(subject.c_str(),blag);
	blag=email.size();
	f->write(&blag,sizeof(blag));
	f->write(email.c_str(),blag);
	return 0;
#endif
};

#ifdef TEXT_CACHE
int c_nntp_file::loadf(char *str,t_cache_items *itp){
	char *s,*s2,*s3;
	if (!(s=strchr(str,'\t')))
		return -1;
	s++;
	if (!(s2=strchr(s,'\t')))
		return -1;
	s2++;
	if (!(s3=strchr(s2,'\t')))
		return -1;
	s3++;
	long num=atol(s3);
	t_cache_items::iterator itpc=itp->find(num);
	if (itpc==itp->end()){
		return -2;
	}
	s_part *p=new s_part;
	p->partnum=atoi(str);
	p->partoff=atoi(s);
	p->tailoff=atoi(s2);
	p->i=(*itpc).second;
//	parts[p->partnum]=p;
//	have++;
	addpart(p);
	return 0;
}
#else
int c_nntp_file::loadf(c_file *f,t_cache_items *itp){
	s_part *p=new s_part;
	long num;
	if(f->read(&p->partnum,sizeof(p->partnum))<=0)return -1;
	if(f->read(&p->partoff,sizeof(p->partoff))<=0)return -1;
	if(f->read(&p->tailoff,sizeof(p->tailoff))<=0)return -1;
	if(f->read(&num,sizeof(num))<=0)return -1;
	t_cache_items::iterator itpc=itp->find(num);
	if (itpc==itp->end()){
//		printf("not found c_nntp_file::load %li %i %i %li\n",p->partnum,p->partoff,p->tailoff,num);
		delete p;
		return -2;
	}
	p->i=(*itpc).second;
//	printf("c_nntp_file::load %li %i %i %li\n",p->partnum,p->partoff,p->tailoff,p->i->num);
//	parts[p->partnum]=p;
//	have++;
	addpart(p);
	return 0;
}
#endif
int c_nntp_file::storef(c_file *f){
#ifdef TEXT_CACHE
	f->putf("%i\t%i\n",req,have);
#else
	f->write(&req,sizeof(req));
	long blag=parts.size();
	f->write(&blag,sizeof(blag));
#endif
	t_nntp_file_parts::iterator curf;
	s_part *p;
	for(curf = parts.begin();curf!=parts.end();++curf){
		p=(*curf).second;
#ifdef TEXT_CACHE
		f->putf("%li\t%i\t%i\t%li\n",p->partnum,p->partoff,p->tailoff,p->i->num);
#else
		f->write(&p->partnum,sizeof(p->partnum));
		f->write(&p->partoff,sizeof(p->partoff));
		f->write(&p->tailoff,sizeof(p->tailoff));
		f->write(&p->i->num,sizeof(p->i->num));
#endif
	}
#ifdef TEXT_CACHE
	f->putf(".\n");
#endif
	return 0;
};


#ifdef DEBUG_CACHE_DESTRUCTION
static int dbgcount3=0;
#endif
c_nntp_file::c_nntp_file(c_nntp_file &f):req(f.req){
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount3++;
	printf("c_nntp_file::c_nntp_file dbgcount=%i\n",dbgcount3);
#endif
	t_nntp_file_parts::iterator curf;
	s_part *p;
	have=0;bytes=0;lines=0;
	for(curf = f.parts.begin();curf!=f.parts.end();++curf){
		p=new s_part(*(*curf).second);
//		parts[p->partnum]=p;		
		addpart(p);
	}
}
c_nntp_file::c_nntp_file(){
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount3++;
	printf("c_nntp_file::c_nntp_file dbgcount=%i\n",dbgcount3);
#endif
	have=0;bytes=0;lines=0;
}
c_nntp_file::~c_nntp_file(){
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount3--;
	printf("c_nntp_file::~c_nntp_file dbgcount=%i\n",dbgcount3);
#endif
	t_nntp_file_parts::iterator curf;
	for(curf = parts.begin();curf!=parts.end();++curf){
		delete (*curf).second;
	}
}

void c_nntp_file::addpart(s_part * p){
	parts[p->partnum]=p;
	if (p->partnum>0 && p->partnum<=req) have++;
	bytes+=p->i->size;
	lines+=p->i->lines;
}

c_nntp_file_u::c_nntp_file_u(c_nntp_file &f,const char *fn):req(f.req){
	t_nntp_file_parts::iterator curf;
	s_part_u *p;
	if (fn)
		filename=fn;
	else{
		s_part *q=(*f.parts.begin()).second;
		if (q->partoff>3)
			filename=q->i->subject.substr(0,q->partoff-1);
		else
			filename=q->i->subject;
	}
	have=f.have;bytes=f.bytes;lines=f.lines;
	banum=(*f.parts.begin()).second->i->num;
	for(curf = f.parts.begin();curf!=f.parts.end();++curf){
		p=new s_part_u(*(*curf).second);
		parts[p->partnum]=p;
	}
}
c_nntp_file_u::c_nntp_file_u(){
	have=0;bytes=0;lines=0;
}
c_nntp_file_u::~c_nntp_file_u(){
	t_nntp_file_u_parts::iterator curf;
	for(curf = parts.begin();curf!=parts.end();++curf){
		delete (*curf).second;
	}
}

s_part_u::s_part_u (s_part &p){
	partnum=p.partnum;
	anum=p.i->num;
	size=p.i->size;
	lines=p.i->lines;
}
/*int s_part::parseitem(c_nntp_cache_item *ni){
	const char *str=ni->subject.c_str();
	char *s=strrchr(str,'/');
	partoff=-1;tailoff=-1;
	i=ni;partnum=0;
	int req=0;
	if (s) {
		char *p;
		if ((p=strpbrk(s,")]"))){
			char m;
			if (*p==')') m='(';
			else m='[';
			tailoff=p-str;
			p=s;
			for(p=s;p>str;p--)
				if (*p==m){
					partoff=p-str;
					partnum=atoi(p+1);
					req=atoi(s+1);
//					if (partnum==0)
//						partnum=-1;
					break;
				}
		}
	}
	
	return req;
}*/

int s_part::parsepnum(const char *str,const char *soff){
	const char *p;
	if ((p=strpbrk(soff+1,")]"))){
		char m,m2=*p;
		if (m2==')') m='(';
		else m='[';
		tailoff=p-str;
		for(p=soff;p>str;p--)
			if (*p==m){
				char *erp;
				partoff=p-str;
				partnum=strtol(p+1,&erp,10);
				if (*erp!='/' || erp==p+1) return -1;
				int req=strtol(soff+1,&erp,10);
				if (*erp!=m2 || erp==soff+1) return -1;
				//					if (partnum==0)
				//						partnum=-1;
				return req;
			}
	}
	return -1;
}
int s_part::parseitem(c_nntp_cache_item *ni){
	const char *str=ni->subject.c_str();
//	char *s=strrchr(str,'/');
	const char *s=str+ni->subject.size()-3;//-1 for null, -2 for ), -3 for num
	int req=0;
	i=ni;
	for (;s>str;s--) {
		if (*s=='/')
			if ((req=parsepnum(str,s))>=0)return req;
	}
	partoff=-1;tailoff=-1;
	partnum=0;
	
	return 0;
}

c_nntp_file_cache_u::c_nntp_file_cache_u(){lines=0;bytes=0;};//:freg("[[(]([0-9]+)/([0-9]+)[])]"){};
c_nntp_file_cache_u::~c_nntp_file_cache_u(){
	t_nntp_files_u::iterator curf;
	for(curf = files.begin();curf!=files.end();++curf){
		delete (*curf).second;
	}
}


#ifdef DEBUG_CACHE_DESTRUCTION
static int dbgcount2=0;
#endif
c_nntp_file_cache::c_nntp_file_cache(){
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount2++;
	printf("c_nntp_file_cache::c_nntp_file_cache dbgcount=%i\n",dbgcount2);
#endif
}//:freg("[[(]([0-9]+)/([0-9]+)[])]"){};
c_nntp_file_cache::~c_nntp_file_cache(){
	t_nntp_files::iterator curf;
	for(curf = files.begin();curf!=files.end();++curf){
		delete (*curf);
	}
	//files.erase(files.begin(),files.end());
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount2--;
	printf("c_nntp_file_cache::~c_nntp_file_cache dbgcount=%i\n",dbgcount2);
#endif
}
int c_nntp_file_cache::store(c_file *f){
	t_nntp_files::iterator curf;
	for(curf = files.begin();curf!=files.end();++curf){
		(*curf)->storef(f);
	}
	return 0;
}
#ifdef TEXT_CACHE
c_nntp_file* c_nntp_file_cache::load(char * str){
	c_nntp_file *c;
	char *p=strchr(str,'\t');
	if (!p)return NULL;
	p++;
	c=new c_nntp_file;
	c->have=0;
	c->req=atoi(str);
//	c.have=atoi(p);//####hmmm... don't really want this
	files.push_back(c);
	return c;
}
#else
void c_nntp_file_cache::load(long l, c_file *f,t_cache_items *itp){
	c_nntp_file *c;
	long blag;
	c=new c_nntp_file;
	c->have=0;
	c->req=l;
	if (f->read(&blag,sizeof(blag))<=0) {delete c;return;}
//	printf("c_nntp_file_cache::load %li %li\n",l,blag);
	for (long i=0;i<blag;i++)
		c->load(f,itp);
	files.push_back(c);
}
#endif
int c_nntp_file_cache::delitems(long lownum){
	t_nntp_files::iterator curf,nextf;
	c_nntp_file*f;
	t_nntp_file_parts::iterator curp;
	s_part *p;
	int t=0;
	for(nextf = curf = files.begin();curf!=files.end();curf=nextf){
		++nextf;//more iterator invalidation carp
		f=(*curf);
//		t+=f->delitems(lownum);
		for(curp = f->parts.begin();curp!=f->parts.end();){
			p=(*curp).second;
			if (p->i->num<lownum){
				t++;
				f->bytes-=p->i->size;
				f->lines-=p->i->lines;
				if (p->partnum>0 && p->partnum<=f->req) f->have--;
				delete p;
				f->parts.erase(curp);
				curp=f->parts.begin();//iterators are invalidated by erase, hopefully this will be good.
				continue;
			}
			++curp;
		}
		if (f->parts.size()<=0){
			delete f;
			files.erase(curf);
		}
	}
	return t;
}

#ifdef DEBUG_CACHE
typedef map<long,int,less<long> > t_consistancy_count;
void c_nntp_file_cache::check_consistancy(t_cache_items *itp){

	t_consistancy_count count;
	t_consistancy_count::iterator curc;
	t_nntp_files::iterator cur;
	c_nntp_file *f;
	t_nntp_file_parts::iterator curp;
	c_nntp_cache_item *i;
	s_part *p;
	long tparts=0,tpu=0,atotal=0,dupes=0,notfound=0;

	t_cache_items::iterator curi;
	for(curi = itp->begin();curi!=itp->end();++curi){
		count[(*curi).first]=0;
		atotal++;
	}
	
	for(cur = files.begin();cur!=files.end();++cur){
		f=(*cur);
		for(curp = f->parts.begin();curp!=f->parts.end();++curp){
			p=(*curp).second;
			i=p->i;
			if (p->partnum>f->req)
				printf("c_nntp_file_cache: partnum %li>%i : %s\n",p->partnum,f->req,i->subject.c_str());
			count[i->num]++;
			if (count[i->num]==1)
				tpu++;
			tparts++;
		}
	}
	long ac,an;
	for(curc = count.begin();curc!=count.end();++curc){
		an=(*curc).first;
		ac=(*curc).second;
		if(ac<=0){
			notfound++;
			printf("article %li was not in file cache, adding\n",an);
			additem((*itp)[an]);
		}else if (ac>1){
			dupes+=ac-1;
			printf("article %li was in file cache %li times\n",an,ac);
		}
	}
	printf("c_nntp_file_cache: %li parts(%li unique) (out of %li articles). %li dupes, %li not found.\n",tparts,tpu,atotal,dupes,notfound);
}
#endif

void c_nntp_file_cache::additem(c_nntp_cache_item* i){
	t_nntp_files::iterator curf;
	s_part *p, *tp;
	p=new s_part;
	c_nntp_file *nf;
	int req;
	if ((req=p->parseitem(i))>=0){
//		if (p->partnum==0)
//			return;
		for(curf = files.begin();curf!=files.end();++curf){
			nf=(*curf);
			if (nf->parts.find(p->partnum)!=nf->parts.end())
				continue;
			tp=(*nf->parts.begin()).second;
			if (req==nf->req && tp->i->email==i->email && p->partoff==tp->partoff && strncmp(i->subject.c_str(),tp->i->subject.c_str(),p->partoff)==0){
//				nf->parts[p->partnum]=p;
//				nf->have++;
				nf->addpart(p);
				return;
			}
		}
	}
	nf=new c_nntp_file;
	nf->req=req;
//	nf->have=1;
//	nf->parts[p->partnum]=p;
	nf->addpart(p);
	files.push_front(nf);
//	items.push_front(s_part(i,pn));
}

//shouldn't be needed anymore, since filecache is saved
/*void c_cache::initfiles(void){
	t_cache_items::iterator cur;
	c_nntp_cache_item *i;
	printf("initializing file lookup cache..");fflush(stdout);
	if (files)delete files;
	files=new c_nntp_file_cache;
	for(cur = items.begin();cur!=items.end();++cur){
		i=(*cur).second;
		files->additem(i);
	}
	printf(" done (%i->%i)\n",items.size(),files->files.size());
}*/

// old method, goes through and initializes a cache for each match
/*c_nntp_file_cache * c_cache::getfiles(const char *match){
	int nm=0;
	c_nntp_file_cache *fc=new c_nntp_file_cache;
	c_regex hreg(match);

	t_cache_items::iterator cur;
	c_nntp_cache_item *i;
	for(cur = items.begin();cur!=items.end();++cur){
		i=(*cur).second;
		if (!hreg.match(i->subject.c_str())){//matches user spec
			fc->additem(i);
			nm++;
//			if (!freg.match(i->subject.c_str()+hreg.subeo(0))){//has part nums
//			}
		}
	}
	if (!nm){
		delete fc;fc=NULL;
	}
	return fc;
}*/
class file_match {
	public:
		c_regex reg;
		long size;
		file_match(const char *m,int a):reg(m,a){};
};
typedef slist<file_match *> filematchlist;
//typedef slist<char *> filematchlist;
typedef slist<long> longlist;

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
	char buf[1024];
	for (int i=0;i<globbuf.gl_pathc;i++){
/*		asprintf(&s,"*[!"ALNUM"]%s[!"ALNUM"]*",globbuf.gl_pathv[i]);
		(*l)->push_front(s);
		asprintf(&s,"*[!"ALNUM"]%s",globbuf.gl_pathv[i]);
		(*l)->push_front(s);
		asprintf(&s,"%s[!"ALNUM"]*",globbuf.gl_pathv[i]);*/
		//no point in using fnmatch.. need to do this gross multi string per file kludge, and...
		//sprintf(buf,"(^|[^[:alnum:]]+)%s([^[:alnum:]]+|$)",globbuf.gl_pathv[i]);//this is about the same speed as the 3 fnmatchs
		sprintf(buf,"\\<%s\\>",globbuf.gl_pathv[i]);//this is much faster
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
			(*a)->push_front(atol(globbuf.gl_pathv[i]));
		}
	}
	globfree(&globbuf);
//	return l;
}

//gnu extension.. if its not available, just compare normally.
//#ifndef FNM_CASEFOLD
//#define FNM_CASEFOLD 0
//#endif

int checkhavefile(filematchlist *fl,longlist *al,const char *f,long anum,long bytes){
	filematchlist::iterator curl;
	longlist::iterator cura;
	if (al)
		for (cura=al->begin();cura!=al->end();++cura){
			if (*cura==anum){
				printf("already have %s\n",f);
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

//new method, initializes the cache once, then searches that
c_nntp_file_cache_u * c_cache::getfiles(c_nntp_file_cache_u * fc,const char *match, int linelimit,int flags){
	//,int getincomplete, int casesensitive){
	//int nm=0;
//	c_nntp_file_cache *fc=new c_nntp_file_cache;
	if (fc==NULL) fc=new c_nntp_file_cache_u;
	c_regex hreg(match,REG_EXTENDED + ((flags&GETFILES_CASESENSITIVE)?0:REG_ICASE));

	if (!files) throw new c_error(EX_A_FATAL,"no file cache??");

	filematchlist *flist=NULL;
	longlist *alist=NULL;
	if (!(flags&GETFILES_NODUPECHECK))
		buildflist(&flist,&alist);
	t_nntp_files::iterator cur;
	c_nntp_file *f;
	c_nntp_file_u *nf;
	c_nntp_cache_item *i;
	for(cur = files->files.begin();cur!=files->files.end();++cur){
		f=(*cur);
		i=(*f->parts.begin()).second->i;
		if (f->lines>=linelimit && (f->have>=f->req || flags&GETFILES_GETINCOMPLETE) && !hreg.match(i->subject.c_str())){//matches user spec
//			fc->additem(i);
			if (fc->files.find(i->num)!=fc->files.end())
				continue;
			if (!(flags&GETFILES_NODUPECHECK) && checkhavefile(flist,alist,i->subject.c_str(),i->num,f->bytes))
				continue;
			nf=new c_nntp_file_u(*f,NULL);
			//fc->files.push_front(nf);
			//fc->files.insert(nf->parts.begin()->second->anum)=nf;
			fc->files[nf->banum]=nf;
			fc->lines+=nf->lines;
			fc->bytes+=nf->bytes;
//			nm++;
//			if (!freg.match(i->subject.c_str()+hreg.subeo(0))){//has part nums
//			}
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
void c_cache::additem(c_nntp_cache_item* i, int do_file_cache){
	t_cache_items::iterator cur=items.find(i->num);
	if(cur != items.end()){
		delete (*cur).second;
		(*cur).second=i;
	}else
		items[i->num]=i;
	//items.push_back(i);
	if (i->num>high)
		high=i->num;
	if (i->num<low)
		low=i->num;
	if (do_file_cache){
		files->additem(i);
		saveit=1;
	}
}
int c_cache::flushlow(int newlow){
	int count=0,num1=items.size();
	t_cache_items::iterator cur;
	c_nntp_cache_item*i;
	if (!quiet) {printf("Flushing headers %i-%i(%i):",low,newlow-1,newlow-low);fflush(stdout);}
	int fi=files->delitems(newlow);
	cur=items.begin();
	while (cur!=items.end()){
		i=(*cur).second;
		if (i->num<newlow){
			count++;
			delete i;
		}else
			break;
		++cur;
	}
	if(cur!=items.begin()){
		items.erase(items.begin(),cur);
	}
	if (!quiet){printf(" %i(%i,%i)\n",count,num1-items.size(),fi);}
	low=newlow;
	saveit=1;
	return count;
}

#ifdef DEBUG_CACHE
void c_cache::check_consistancy(const char *s){
		printf("%s: check_consistancy: %i headers (%i-%i (%i)) \n",s,items.size(),low,high,high-low+1);
		files->check_consistancy(&items);
//		printf("DEBUG_CACHE: headers:%li files:%li parts:%li pdupes:%li",items.size(),files->size(),0,0);
}
#endif

#ifdef DEBUG_CACHE_DESTRUCTION
static int dbgcount1=0;
#endif
c_cache::c_cache(string path,string hid,string nid):file(path),high(0),low(INT_MAX){
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount1++;
	printf("c_cache::c_cache dbgcount=%i\n",dbgcount1);
#endif
#ifdef HAVE_LIBZ
	c_file_gz f;
#else
	c_file_stream f;
#endif
	int mode=0;
#ifdef TEXT_CACHE
	f.initrbuf(2048);
	//f.initrbuf(256);
	char *buf;
#else
	char buf[2048];
#endif
	saveit=0;//if we are interupted during the loading process, we wouldn't want to overwrite the full file with only the part we have read.
	c_nntp_cache_item*i;
	testmkdir(file.c_str(),S_IRWXU);
	file.append(hid);
	testmkdir(file.c_str(),S_IRWXU);
	file.append("/");
	file.append(nid);
#ifndef TEXT_CACHE
	file.append("_b");
#endif
#ifdef HAVE_LIBZ
	file.append(".gz");
#endif
	fileread=0;
	files=new c_nntp_file_cache;
	if (!f.open(file.c_str(),"r")){
		fileread=1;
#ifdef TEXT_CACHE
		c_nntp_file *fp=NULL;
		while (f.bgets()>0){
		//while (f.gets(buf,2048)){
			buf=f.rbufp;
			if(buf[0]=='.'){
				mode=1;
				continue;
			}
			switch (mode){
				case 0:
					if((i=c_nntp_cache_item::loadi(buf))){
						additem(i,0);
					}else
						printf("read invalid cache line(mode %i): %s\n",mode,buf);
					break;
				case 1:
					if ((fp=files->load(buf)))
						mode=2;
					else
						printf("read invalid cache line(mode %i): %s\n",mode,buf);
					break;
				case 2:
					if(fp){
						if (fp->loadf(buf,&items))
							printf("read invalid cache line(mode %i): %s\n",mode,buf);
					}
					break;
				default:
					break;
			}
			
		}
#else
		long blag;
		while (f.read(&blag,sizeof(blag))==sizeof(blag)){
			if(mode==0 && blag==0){
				mode=1;
//				printf("mode=1\n");
				continue;
			}
			switch (mode){
				case 0:
					if((i=c_nntp_cache_item::loadi(blag,&f,buf))){
						additem(i,0);
					}
					break;
				case 1:
					files->load(blag,&f,&items);
//						mode=2;
					break;
//				case 2:
//					if(p)
//						p->load(buf,&items);
//					break;
				default:
					break;
			}
			
		}
#endif
		f.close();
#ifdef DEBUG_CACHE
		check_consistancy("c_cache");
#endif
	}
#ifdef DEBUG_CACHE
		else printf("couldn't open %s: %s\n",file.c_str(),strerror(errno));
#endif
//	saveit=1;//actually, we should only save it again if its changed..
};
c_cache::~c_cache(){
#ifdef HAVE_LIBZ
	c_file_gz f;
#else
	c_file_stream f;
#endif

#ifdef DEBUG_CACHE
	check_consistancy("~c_cache");
#endif

	int writeit=(saveit && (items.size() || fileread) && !f.open(file.c_str(),"w"));
	//	if ((items.size() || fileread) && !f.open(file.c_str(),"w")){

	t_cache_items::iterator cur;
	c_nntp_cache_item *i;
	if(writeit){
		if (!quiet){printf("saving cache: %i headers, %i files..",items.size(),files->files.size());fflush(stdout);}
		for(cur = items.begin();cur!=items.end();++cur){
			i=(*cur).second;
			if (!i){
				printf("item %li == NULL?\n",(*cur).first);continue;
			}
			i->storei(&f);
			//	delete i;
		}
#ifdef TEXT_CACHE
		f.putf(".\n");
#else
		long blag=0;
		f.write(&blag,sizeof(blag));
#endif
		if (files) files->store(&f);
#ifdef TEXT_CACHE
		f.putf(".\n");
#endif
		f.close();
		if (!quiet) printf(" done.\n");
	}
	for(cur = items.begin();cur!=items.end();++cur){//wasteful.. but what can ya do?  need to have them around for the files->store, but have to write these first so that they are around when loading files
		i=(*cur).second;
		delete i;
	}
//	items.erase(items.begin(),items.end());
	//	}
	if (files) delete files;
#ifdef DEBUG_CACHE_DESTRUCTION
	dbgcount1--;
//	printf("c_cache::c_cache dbgcount=%i\n",dbgcount1);
	cache_dbginfo();
#endif
};

#ifdef DEBUG_CACHE_DESTRUCTION
void cache_dbginfo(void){
	printf("--\nc_cache::c_cache dbgcount=%i\n",dbgcount1);
	printf("c_nntp_file_cache::c_nntp_file_cache dbgcount=%i\n",dbgcount2);
	printf("c_nntp_file::c_nntp_file dbgcount=%i\n",dbgcount3);
	printf("c_nntp_cache_item::c_nntp_cache_item dbgcount=%i\n",dbgcount5);
}
#endif
