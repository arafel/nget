/*
    cache.* - nntp header cache code
    Copyright (C) 1999-2001  Matthew Mueller <donut@azstarnet.com>

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
#include "cache.h"
#include "strreps.h"
#include SLIST_H
#include <memory>
#include <errno.h>
#include <dirent.h>
#include "nget.h"
#include "mylockfile.h"
#include "strtoker.h"
int c_nntp_header::parsepnum(const char *str,const char *soff){
	assert(str);
	assert(soff>=str);
				p++;
				partnum=strtol(p,&erp,10);
				if (*erp!='/' || erp==p) return -1;
				if (partnum>req) return -1;
				//                                      if (partnum==0)
				//                                              partnum=-1;

void c_nntp_header::setfileid(void){
#ifdef CHECKSUM
	fileid=CHECKSUM(0L, Z_NULL, 0);
	fileid=CHECKSUM(fileid,(Byte*)subject.c_str(),subject.size());
	fileid=CHECKSUM(fileid,(Byte*)author.c_str(),author.size());
#else
	hash<char *> H;
	fileid=H(subject.c_str())+H(author.c_str());//prolly not as good as crc32, but oh well.
#endif
}
void c_nntp_header::set(char * str,const char *a,ulong anum,time_t d,ulong b,ulong l,const char *mid){
	assert(str);
	assert(a);
	author=a;articlenum=anum;date=d;bytes=b;lines=l;
	messageid=mid;
	const char *s=str+strlen(str)-3;//-1 for null, -2 for ), -3 for num
	req=0;
			if ((req=parsepnum(str,s))>=0){
				subject="";
				subject.append(str,partoff);
				subject.append("*");
				subject.append(s);
				setfileid();
				return;
			}
//	partnum=0;
	partnum=-1;
	subject=str;
	setfileid();
c_nntp_server_article::c_nntp_server_article(ulong _server,ulong _articlenum,ulong _bytes,ulong _lines):serverid(_server),articlenum(_articlenum),bytes(_bytes),lines(_lines){}
//c_nntp_part::c_nntp_part(c_nntp_header *h):partnum(h->partnum),articlenum(h->articlenum),date(h->date),bytes(h->bytes),lines(h->lines){}
c_nntp_part::c_nntp_part(c_nntp_header *h):partnum(h->partnum),date(h->date),messageid(h->messageid){
	addserverarticle(h);
}
void c_nntp_part::addserverarticle(c_nntp_header *h){
	c_nntp_server_article *sa;
#ifndef NDEBUG
	if (debug>=DEBUG_MIN){
		t_nntp_server_articles::iterator sai=articles.find(h->serverid);
		if (sai!=articles.end()){
			sa=(*sai).second;
			printf("adding server_article we already have %lu %lu %lu %lu(%lu %lu %lu %lu)\n",h->serverid,h->articlenum,h->bytes,h->lines,sa->serverid,sa->articlenum,sa->bytes,sa->lines);
			//		return;//could be useful, lets add it.
		}
	if (h->date!=date)
		printf("date=%li h->date=%li\n",date,h->date);
	sa=new c_nntp_server_article(h->serverid,h->articlenum,h->bytes,h->lines);
	articles.insert(t_nntp_server_articles::value_type(h->serverid,sa));

c_nntp_part::~c_nntp_part(){
	t_nntp_server_articles::iterator i;
	for(i = articles.begin();i!=articles.end();++i){
		assert((*i).second);
		delete (*i).second;


void c_nntp_file::addpart(c_nntp_part *p){
	assert(p);
	//assert((req==-1 && p->partnum<=0) || (p->partnum<=req));//#### req==-1 hack for old version that set non-multipart messages partnum to 0 instead of -1
//	parts[p->partnum]=p;
#ifndef NDEBUG
	t_nntp_file_parts::iterator nfpi=parts.find(p->partnum);
	assert(nfpi==parts.end());
	parts.insert(t_nntp_file_parts::value_type(p->partnum,p));
	if (p->partnum>0 && p->partnum<=req) have++;
//	bytes+=p->apxbytes;lines+=p->apxlines;
c_nntp_file::c_nntp_file(int r,ulong f,t_id fi,const char *s,const char *a,int po,int to):req(r),have(0),flags(f),fileid(fi),subject(s),author(a),partoff(po),tailoff(to){
//	printf("aoeu1.1\n");
}
c_nntp_file::c_nntp_file(c_nntp_header *h):req(h->req),have(0),flags(0),fileid(h->fileid),subject(h->subject),author(h->author),partoff(h->partoff),tailoff(h->tailoff){
//	printf("aoeu1\n");
c_nntp_file::~c_nntp_file(){
	t_nntp_file_parts::iterator i;
	for(i = parts.begin();i!=parts.end();++i){
		assert((*i).second);
		delete (*i).second;

		c_regex_nosub reg;
		ulong size;
void buildflist(const string &path, filematchlist **l){
	DIR *dir=opendir(path.c_str());
	struct dirent *de;
	if (!dir)
		throw new c_error(EX_PATH_FATAL,"opendir: %s(%i)",strerror(errno),errno);
	char buf[1024],*cp;
	int sl;
	while ((de=readdir(dir))) {
		if (strcmp(de->d_name,"..")==0) continue;
		if (strcmp(de->d_name,".")==0) continue;
		sl=0;
		buf[sl++]='\\';
#ifdef HAVE_PCREPOSIX_H
		buf[sl++]='b';//match word boundary
#else
		buf[sl++]='<';//match beginning of word
#endif
		cp=de->d_name;
		while (*cp){
			if (strchr("{}()|[]\\.+*^$",*cp))
				buf[sl++]='\\';//escape special chars
			buf[sl++]=*cp;
			cp++;
		}
		buf[sl++]='\\';
#ifdef HAVE_PCREPOSIX_H
		buf[sl++]='b';//match word boundary
#else
		buf[sl++]='>';//match end of word
#endif
		buf[sl++]=0;
		if (stat((path+'/'+de->d_name).c_str(), &stbuf)==0)
	closedir(dir);
int checkhavefile(filematchlist *fl,const char *f,string messageid,ulong bytes){
			if ((fm->reg.match(f)==0/* || fm->reg.match((messageid+".txt").c_str())==0*/) && fm->size*2>bytes && fm->size<bytes){//TODO: handle text files saved.
c_nntp_files_u* c_nntp_cache::getfiles(const string &path, const string &temppath, c_nntp_files_u * fc,c_mid_info *midinfo,generic_pred *pred,int flags){
//c_nntp_files_u* c_nntp_cache::getfiles(c_nntp_files_u * fc,c_nrange *grange,const char *match, unsigned long linelimit,int flags){
	if (fc==NULL) fc=new c_nntp_files_u;
	//c_regex hreg(match,REG_EXTENDED + ((flags&GETFILES_CASESENSITIVE)?0:REG_ICASE));
	if (!(flags&GETFILES_NODUPEFILECHECK))
		buildflist(path, &flist);

	t_nntp_files::const_iterator fi;
	pair<t_nntp_files_u::const_iterator,t_nntp_files_u::const_iterator> firange;
	c_nntp_file::ptr f;
	for(fi = files.begin();fi!=files.end();++fi){
		f=(*fi).second;
		//if (f->lines>=linelimit && (flags&GETFILES_NODUPECHECK || !(f->flags&FILEFLAG_READ)) && (f->have>=f->req || flags&GETFILES_GETINCOMPLETE) && !hreg.match(f->subject.c_str())){//matches user spec
		//if (f->lines>=linelimit && (flags&GETFILES_NODUPECHECK || !(grange->check(banum))) && (f->have>=f->req || flags&GETFILES_GETINCOMPLETE) && !hreg.match(f->subject.c_str())){//matches user spec
		if ((flags&GETFILES_NODUPEIDCHECK || !(midinfo->check(f->bamid()))) && (f->have>=f->req || flags&GETFILES_GETINCOMPLETE) && (*pred)((ubyte*)f.gimmethepointer())){//matches user spec
//			if (!(flags&GETFILES_NODUPECHECK) && f->isread())
//				continue;

//			if (fc->files.find(banum)!=fc->files.end())
//				continue;
			firange=fc->files.equal_range(f->badate());
			for (;firange.first!=firange.second;++firange.first){
				if ((*firange.first).second->file->bamid()==f->bamid())
//					continue;
					goto file_match_loop_end;//can't continue out of multiple loops
			}

			if (!(flags&GETFILES_NODUPEFILECHECK) && checkhavefile(flist,f->subject.c_str(),f->bamid(),f->bytes())){
				if (flags&GETFILES_DUPEFILEMARK)
					midinfo->insert(f->bamid());
			}
//			f->inc_rcount();
//			fc->files[banum]=f;
			//fc->files.insert(t_nntp_files_u::value_type(f->badate(),f));
			fc->files.insert(t_nntp_files_u::value_type(f->badate(), new c_nntp_file_retr(path,temppath,f)));
			fc->lines+=f->lines();
			fc->bytes+=f->bytes();
file_match_loop_end: ;


c_nntp_server_info* c_nntp_cache::getserverinfo(ulong serverid){
	c_nntp_server_info* servinfo=server_info[serverid];
	if (servinfo==NULL){
		servinfo=new c_nntp_server_info(serverid);
		server_info[serverid]=servinfo;
	return servinfo;
int c_nntp_cache::additem(c_nntp_header *h){
	assert(h);
	c_nntp_file::ptr f;
	t_nntp_files::iterator i;
	pair<t_nntp_files::iterator, t_nntp_files::iterator> irange = files.equal_range(h->fileid);
//	t_nntp_files::const_iterator i;
//	pair<t_nntp_files::const_iterator, t_nntp_files::const_iterator> irange = files.equal_range(h->mid);

	c_nntp_server_info *servinfo=getserverinfo(h->serverid);
	if (h->articlenum > servinfo->high)
		servinfo->high = h->articlenum;
	if (h->articlenum < servinfo->low)
		servinfo->low = h->articlenum;
	servinfo->num++;

//	printf("%lu %s..",h->articlenum,h->subject.c_str());
	for (i=irange.first;i!=irange.second;++i){
		f=(*i).second;
		assert(!f.isnull());
		if (f->req==h->req && f->partoff==h->partoff /*-duh, not good-&& f->tailoff==h->tailoff*/){
			//these two are merely for debugging.. it shouldn't happen (much..? ;)
			if (!(f->author==h->author)){//older (g++) STL versions seem to have a problem with strings and !=
				PDEBUG(DEBUG_MED,"%lu->%s was gonna add, but author is different?\n",h->articlenum,f->bamid().c_str());
				continue;
			}
			if (!(f->subject==h->subject)){
				PDEBUG(DEBUG_MED,"%lu->%s was gonna add, but subject is different?\n",h->articlenum,f->bamid().c_str());
				continue;
			}
			t_nntp_file_parts::iterator op;
			if ((op=f->parts.find(h->partnum))!=f->parts.end()){
				c_nntp_part *matchpart=(*op).second;
				if (matchpart->messageid==h->messageid){
//#ifndef NDEBUG//addserverarticle already has this check in it.
//					t_nntp_server_articles::iterator sai(matchpart->articles.find(h->serverid));
//					if (sai != matchpart->articles.end())
//						printf("adding %s for server %lu again?\n",h->messageid.c_str(),(*sai).second->serverid);
//#endif
					matchpart->addserverarticle(h);
					return 0;
				}
				PDEBUG(DEBUG_MED,"%s was gonna add, but already have this part(sub=%s part=%i omid=%s)?\n",h->messageid.c_str(),f->subject.c_str(),h->partnum,matchpart->messageid.c_str());
				continue;
			}
//			printf("adding\n");
			c_nntp_part *p=new c_nntp_part(h);
			f->addpart(p);
			totalnum++;
			return 0;
		}
	}
//	printf("new\n");
	f=new c_nntp_file(h);
	c_nntp_part *p=new c_nntp_part(h);
	f->addpart(p);
	totalnum++;
	//files[f->subject.c_str()]=f;
	files.insert(t_nntp_files::value_type(f->fileid,f));
	return 1;
void c_nntp_cache::getxrange(c_nntp_server_info *servinfo,ulong newlow,ulong newhigh, c_nrange *range){
	range->clear();
	range->insert(newlow<servinfo->low?newlow:servinfo->low,newhigh);
	t_nntp_files::iterator i;
	c_nntp_file::ptr nf;
	t_nntp_file_parts::iterator pi;
	c_nntp_part *np;
	pair<t_nntp_server_articles::iterator,t_nntp_server_articles::iterator> sarange;
	c_nntp_server_article *sa;
	for(i = files.begin();i!=files.end();++i){
		nf=(*i).second;
		assert(!nf.isnull());
		assert(!nf->parts.empty());
		for(pi = nf->parts.begin();pi!=nf->parts.end();++pi){
			np=(*pi).second;
			assert(np);
			sarange=np->articles.equal_range(servinfo->serverid);
			while (sarange.first!=sarange.second){
				sa=(*sarange.first).second;
				assert(sa);
				range->remove(sa->articlenum);
				++sarange.first;
			}
		}
	}
ulong c_nntp_cache::flushlow(c_nntp_server_info *servinfo, ulong newlow, c_mid_info *midinfo){
	ulong count=0,countp=0,countf=0;
	assert(newlow>0);
	t_nntp_files::iterator i,in;
	c_nntp_file::ptr nf;
	t_nntp_file_parts::iterator pi,pic;
	c_nntp_part *np;
	pair<t_nntp_server_articles::iterator,t_nntp_server_articles::iterator> sarange;
	t_nntp_server_articles::iterator sai;
	c_nntp_server_article *sa;
	c_mid_info rel_midinfo("");
	if (quiet<2) {printf("Flushing headers %lu-%lu(%lu):",servinfo->low,newlow-1,newlow-servinfo->low);fflush(stdout);}
	for(in = files.begin();in!=files.end();){
		i=in;
		++in;
		nf=(*i).second;
		assert(!nf.isnull());
		assert(!nf->parts.empty());
		for(pi = nf->parts.begin();pi!=nf->parts.end();){
			pic=pi;
			++pi;
			np=(*pic).second;
			assert(np);
			sarange=np->articles.equal_range(servinfo->serverid);
			while (sarange.first!=sarange.second){
				sai=sarange.first;
				++sarange.first;
				sa=(*sai).second;
				assert(sa);
				if (sa->articlenum<newlow){
					delete sa;
					np->articles.erase(sai);
					if (np->articles.empty()){
						midinfo->set_delete(np->messageid);
						delete np;
						np=NULL;
						nf->parts.erase(pic);
						countp++;
					}
					count++;
				}
			}
			if (np && midinfo->check(np->messageid)) rel_midinfo.insert(np->messageid);
		}
		if (nf->parts.empty()){
//			nf->dec_rcount();
//			delete nf;
			files.erase(i);
			countf++;
//#ifdef HAVE_HASH_MAP_H
//			in=files.begin();//not needed, apparantly.
//#endif
		}
	}
	servinfo->num-=count;
	totalnum-=countp;
	servinfo->low=newlow;
#ifndef NDEBUG
	for(in = files.begin();in!=files.end();++in){
		nf=(*in).second;
		assert(!nf.isnull());
		assert(!nf->parts.empty());
		for(pi = nf->parts.begin();pi!=nf->parts.end();++pi){
			np=(*pi).second;
			assert(np);
			sai=np->articles.find(servinfo->serverid);
			if (sai!=np->articles.end()){
				sa=(*sai).second;
				assert(sa->articlenum>=newlow);
			}
		}
	}
	if (quiet<2){printf(" %lu (%lu,%lu)\n",count,countp,countf);}
	if (count)saveit=1;
	midinfo->do_delete_fun(rel_midinfo);

	return count;
}
void setfilenamegz(string &file, int gz=-2){
#ifndef HAVE_LIBZ
	gz=0;
#endif
	if (gz==-2)
		gz=nconfig.usegz;
	if (gz)
		file.append(".gz");
}
c_file *dofileopen(string file, string mode, int gz=-2){
	c_file *f=NULL;
#ifndef HAVE_LIBZ
	gz=0;
	if (gz==-2)
		gz=nconfig.usegz;
	if (gz){
		if (gz>0){
			char blah[10];
			sprintf(blah,"%i",gz);
			mode.append(blah);
		}
		c_file_gz *gz=new c_file_gz();
		if (gz->open(file.c_str(),mode.c_str())){
			delete gz;
			return NULL;
		}
		f=gz;
	}
	if (!gz){
		c_file_fd *fd=new c_file_fd();
		if (fd->open(file.c_str(),mode.c_str())){
			delete fd;
			return NULL;
		}
		f=fd;
	}
	if (mode[0]=='r' || mode.find('+')>=0)
		f->initrbuf(2048);
	return f;
}

c_nntp_cache::c_nntp_cache(string path,c_group_info::ptr group_):totalnum(0),group(group_){
	c_file *f=NULL;
	saveit=0;
	//file=nid;
	file=path+"/"+group->group + ",cache";
	setfilenamegz(file,group->usegz);
	if ((f=dofileopen(file.c_str(),"rb",group->usegz))){
		auto_ptr<c_file> fcloser(f);
		char *buf;
		int mode=2;
		c_nntp_file	*nf=NULL;
		c_nntp_part	*np=NULL;
		c_nntp_server_article *sa;
		c_nntp_server_info *si;
		char *t[8];
		int i;
		ulong count=0,counta=0,curline=0;
		while (f->bgets()>0){
			buf=f->rbufp();
			curline++;
			if (mode==2){//start mode
				for(i=0;i<2;i++)
					if((t[i]=goodstrtok(&buf,'\t'))==NULL){
						break;
					}
				if (i>=2 && (strcmp(t[0],CACHE_VERSION))==0){
					totalnum=atoul(t[1]);
				}else{
					if (i>0 && strncmp(t[0],"NGET",4)==0)
						printf("%s is from a different version of nget\n",file.c_str());
					else
						printf("%s does not seem to be an nget cache file\n",file.c_str());
					f->close();fileread=0;
					return;
				}
				mode=4;//go to serverinfo mode.
			}else if (mode==4){//server_info mode
				if (buf[0]=='.'){
					assert(buf[1]==0);
					mode=0;//start new file mode
					continue;
				}else{
					for(i=0;i<4;i++)
						if((t[i]=goodstrtok(&buf,'\t'))==NULL){
							i=-1;break;
						}
					if (i>=4){
						si=new c_nntp_server_info(atoul(t[0]),atoul(t[1]),atoul(t[2]),atoul(t[3]));
						server_info[si->serverid]=si;
					}else
						printf("invalid line %lu mode %i\n",curline,mode);
				}
			else if (mode==3 && np){//new server_article mode
				if (buf[0]=='.'){
					assert(buf[1]==0);
					mode=1;//go back to new part mode
					continue;
				}else{
					for(i=0;i<4;i++)
						if((t[i]=goodstrtok(&buf,'\t'))==NULL){
							i=-1;break;
						}
					if (i>=4){
						assert(i==4);
						sa=new c_nntp_server_article(atoul(t[0]),atoul(t[1]),atoul(t[2]),atoul(t[3]));
						//np->addserverarticle(sa);
						np->articles.insert(t_nntp_server_articles::value_type(sa->serverid,sa));
						counta++;
						printf("invalid line %lu mode %i\n",curline,mode);
				}
			}
			else if (mode==1 && nf){//new part mode
				if (buf[0]=='.'){
					assert(buf[1]==0);
					mode=0;//go back to new file mode
			//		nf->addpart(np);//added here so that addpart will have apxlines/apxbytes to work with (set in mode 3)
					np=NULL;
					continue;
				}else{
					for(i=0;i<3;i++)
						if((t[i]=goodstrtok(&buf,'\t'))==NULL){
							i=-1;break;
						}
					if (i>=3){
						assert(i==3);
						np=new c_nntp_part(atoi(t[0]),atoul(t[1]),t[2]);
						nf->addpart(np);//add at '.' section (above) ... o r not.
						count++;
					}else
						printf("invalid line %lu mode %i\n",curline,mode);
					mode=3;
				}
			}
			else if (mode==0){//new file mode
				for(i=0;i<7;i++)
					if((t[i]=goodstrtok(&buf,'\t'))==NULL){
						i=-1;break;
				if (i>=7){
					assert(i==7);
					nf=new c_nntp_file(atoi(t[0]),atoul(t[1]),atoul(t[2]),t[3],t[4],atoi(t[5]),atoi(t[6]));
					files.insert(t_nntp_files::value_type(nf->fileid,nf));
//					files[nf->subject.c_str()]=nf;
					mode=1;
				}else
					printf("invalid line %lu mode %i\n",curline,mode);
			}else{
				assert(0);//should never get here
		PDEBUG(DEBUG_MIN,"read %lu parts (%lu sa) %i files",count,counta,files.size());
		if (count!=totalnum){
			printf("warning: read %lu parts from cache, expecting %lu\n",count,totalnum);
		}
	}
}
c_nntp_cache::~c_nntp_cache(){
	c_file *f=NULL;
	t_nntp_files::iterator i;
	c_nntp_server_info *si;
	if (saveit && (files.size() || fileread)){
		string tmpfn;
		tmpfn=file+".tmp";
		if((f=dofileopen(tmpfn,"wb",group->usegz))){
			auto_ptr<c_file> fcloser(f);
			if (quiet<2){printf("saving cache: %lu parts, %i files..",totalnum,files.size());fflush(stdout);}
			c_nntp_file::ptr nf;
			t_nntp_file_parts::iterator pi;
			t_nntp_server_articles::iterator sai;
			c_nntp_server_article *sa;
			c_nntp_part *np;
			ulong count=0,counta=0;
			f->putf(CACHE_VERSION"\t%lu\n",totalnum);//mode 2
			//vv mode 4
			while (!server_info.empty()){
				si=server_info.begin()->second;
				assert(si);
				f->putf("%lu\t%lu\t%lu\t%lu\n",si->serverid,si->high,si->low,si->num);//mode 4
				server_info.erase(server_info.begin());
				delete si;
			f->putf(".\n");
			//end mode 4
			//vv mode 0
			for(i = files.begin();i!=files.end();++i){
				nf=(*i).second;
				assert(!nf.isnull());
				assert(!nf->parts.empty());
				f->putf("%i\t%lu\t%lu\t%s\t%s\t%i\t%i\n",nf->req,nf->flags,nf->fileid,nf->subject.c_str(),nf->author.c_str(),nf->partoff,nf->tailoff);//mode 0
				for(pi = nf->parts.begin();pi!=nf->parts.end();++pi){
					np=(*pi).second;
					assert(np);
					f->putf("%i\t%lu\t%s\n",np->partnum,np->date,np->messageid.c_str());//mode 1
					for (sai = np->articles.begin(); sai != np->articles.end(); ++sai){
						sa=(*sai).second;
						assert(sa);
						f->putf("%lu\t%lu\t%lu\t%lu\n",sa->serverid,sa->articlenum,sa->bytes,sa->lines);//mode 3
						counta++;
					f->putf(".\n");//end mode 3
					count++;
				}
				f->putf(".\n");//end mode 1
				(*i).second=NULL; //free cache as we go along instead of at the end, so we don't swap more with low-mem.
				//nf->storef(f);
				//delete nf;
				//nf->dec_rcount();
			}
			f->close();
			if (quiet<2) printf(" done. (%lu sa)\n",counta);
			if (count!=totalnum){
				printf("warning: wrote %lu parts from cache, expecting %lu\n",count,totalnum);
			if (rename(tmpfn.c_str(), file.c_str())){
				printf("error renaming %s > %s: %s(%i)\n",tmpfn.c_str(),file.c_str(),strerror(errno),errno);
			}
			return;
		}else{
			printf("error opening %s: %s(%i)\n",tmpfn.c_str(),strerror(errno),errno);
	if (quiet<2){printf("freeing cache: %lu parts, %i files..\n",totalnum,files.size());}//fflush(stdout);}
	while (!server_info.empty()){
		si=server_info.begin()->second;
		server_info.erase(server_info.begin());
		delete si;
	}
//	for(i = files.begin();i!=files.end();++i){
		//delete (*i).second;
//		(*i).second->dec_rcount();
//	}
//	if (!quiet) printf(" done.\n");
}
c_nntp_files_u::~c_nntp_files_u(){
//	t_nntp_files_u::iterator i;
//	for(i = files.begin();i!=files.end();++i){
//		(*i).second->dec_rcount();
//	}
}



#define MID_INFO_MIN_KEEP (14*24*60*60)
#define MID_INFO_MIN_KEEP_DEL (7*24*60*60)
void c_mid_info::do_delete_fun(c_mid_info &rel_mid){
	t_message_state_list::iterator i=states.begin();
	c_message_state *s;
	int deld=0;
	time_t curtime=time(NULL);
	for (;i!=states.end();++i){
		s=(*i).second;
		if (rel_mid.check(s->messageid))
			continue;
		if ((s->date_removed==TIME_T_MAX1 && s->date_added+MID_INFO_MIN_KEEP<curtime) || (s->date_added+MID_INFO_MIN_KEEP<curtime && s->date_removed+MID_INFO_MIN_KEEP_DEL<curtime)){
//			delete s;
//			states.erase(i);
//			i=states.begin();//urgh.
			s->date_removed=TIME_T_DEAD;//let em just not get saved.
			changed=1;deld++;
	PDEBUG(DEBUG_MIN,"c_mid_info::do_delete_fun: %i killed",deld);
}
c_mid_info::c_mid_info(string path){
	load(path);
}
int c_mid_info::load(string path,bool merge,bool lock){
	if (!merge){
		clear();
		changed=0;
	}
	if (path.empty())
		return 0;
	c_file *f=NULL;
	if (!merge)
		setfilenamegz(path);//ugh, hack.
		file=path;
	int line=0;
	//c_lockfile locker(path,WANT_SH_LOCK);
	auto_ptr<c_lockfile> locker;
	if (lock){
		auto_ptr<c_lockfile> l(new c_lockfile(path,WANT_SH_LOCK));
		locker=l;
		//locker=new c_lockfile(path,WANT_SH_LOCK);//why can't we just do this?  sigh.
	}
//	c_regex_r midre("^(.+) ([0-9]+) ([0-9]+)$");
	strtoker toker(3,' ');
	if ((f=dofileopen(path.c_str(),"rb"))){
		auto_ptr<c_file> fcloser(f);
		while (f->bgets()>0){
			line++;
			if (!toker.tok(f->rbufp()) && toker.numtoks==3)
				insert_full(toker[0],atol(toker[1]),atol(toker[2]));//TODO: shouldn't set changed flag if no new ones are actually merged.
			else
				printf("c_mid_info::load: invalid line %i (%i toks)\n",line,toker.numtoks);
		}
		f->close();
	}else
		return -1;
	PDEBUG(DEBUG_MIN,"c_mid_info::load read %i lines",line);
	if (!merge)
		changed=0;
	return 0;
}
c_mid_info::~c_mid_info(){
	save();
	clear();
}
int c_mid_info::save(void){
	if (!changed)
		return 0;
	if (file.empty())
		return 0;
	c_file *f=NULL;
	c_lockfile locker(file,WANT_EX_LOCK);//lock before we read, so that multiple copies trying to save at once don't lose changes.
	{
		unsigned int count1=states.size();
		load(file,1,0);//merge any changes that might have happened
		if (count1!=states.size()){
			if (debug){printf("saving mid_info: merged something...(%i)\n",states.size()-count1);}
		}
	int nums=0;
	string tmpfn=file+".tmp";
	if((f=dofileopen(tmpfn,"wb"))){
		auto_ptr<c_file> fcloser(f);
		if (debug){printf("saving mid_info: %i infos..",states.size());fflush(stdout);}
		t_message_state_list::iterator sli;
		c_message_state* ms;
		for (sli=states.begin(); sli!=states.end(); ++sli){
			ms=(*sli).second;
			if (ms->date_removed==TIME_T_DEAD)
				continue;
			f->putf("%s %li %li\n",ms->messageid.c_str(),ms->date_added,ms->date_removed);
			nums++;
		}
		if (debug) printf(" (%i) done.\n",nums);
		f->close();
		if (rename(tmpfn.c_str(), file.c_str())){
			printf("error renaming %s > %s: %s(%i)\n",tmpfn.c_str(),file.c_str(),strerror(errno),errno);
			return -1;
		}
	}else
		return -1;
	return 0;
