    Copyright (C) 1999-2002  Matthew Mueller <donut@azstarnet.com>
#include "dupe_file.h"
void c_nntp_header::setfileid(char *refstr, unsigned int refstrlen){
	if (refstrlen)
		fileid=CHECKSUM(fileid,(Byte*)refstr,refstrlen);
	if (refstrlen)
		fileid+=H(refstr, refstrlen);
void c_nntp_header::set(char * str,const char *a,ulong anum,time_t d,ulong b,ulong l,const char *mid, char *refstr){
	int refstrlen=refstr?strlen(refstr):0;

	references.clear();
	if (refstr && *refstr) {
		char *ref, *refstr_copy=refstr;
		while ((ref = goodstrtok(&refstr_copy,' '))) {
			references.push_back(ref);
		}
	}

				setfileid(refstr, refstrlen);
	setfileid(refstr, refstrlen);
c_nntp_file::c_nntp_file(c_nntp_header *h):req(h->req),have(0),flags(0),fileid(h->fileid),subject(h->subject),author(h->author),partoff(h->partoff),tailoff(h->tailoff),references(h->references){
	dupe_file_checker flist;
		flist.addfrompath(path);
		if ((flags&GETFILES_NODUPEIDCHECK || !(midinfo->check(f->bamid()))) && (flags&GETFILES_GETINCOMPLETE || f->iscomplete()) && (*pred)((ubyte*)f.gimmethepointer())){//matches user spec
			if (!(flags&GETFILES_NODUPEFILECHECK) && flist.checkhavefile(f->subject.c_str(),f->bamid(),f->bytes())){
			fc->addfile(f,path,temppath);
			if (!(f->references==h->references)){
				PDEBUG(DEBUG_MED,"%lu->%s was gonna add, but references are different?\n",h->articlenum,f->bamid().c_str());
				continue;
			}
enum {
	START_MODE=2,
	SERVERINFO_MODE=4,
	FILE_MODE=0,
	PART_MODE=1,
	SERVER_ARTICLE_MODE=3,
	REFERENCES_MODE=5,
};

		int mode=START_MODE;
			//printf("line %i mode %i: %s\n",curline,mode,buf);
			if (mode==START_MODE){
				mode=SERVERINFO_MODE;//go to serverinfo mode.
			}else if (mode==SERVERINFO_MODE){
					mode=FILE_MODE;//start new file mode
			else if (mode==SERVER_ARTICLE_MODE && np){//new server_article mode
					mode=PART_MODE;//go back to new part mode
			else if (mode==PART_MODE && nf){//new part mode
					mode=FILE_MODE;//go back to new file mode
					mode=SERVER_ARTICLE_MODE;//start adding server_articles
			else if (mode==FILE_MODE){//new file mode
					mode=REFERENCES_MODE;
			}
			else if (mode==REFERENCES_MODE && nf){//adding references on new file
				if (buf[0]=='.'){
					assert(buf[1]==0);
					mode=PART_MODE;
					np=NULL;
					continue;
				}else{
					nf->references.push_back(buf);
				}
			try {
				auto_ptr<c_file> fcloser(f);
				if (quiet<2){printf("saving cache: %lu parts, %i files..",totalnum,files.size());fflush(stdout);}
				c_nntp_file::ptr nf;
				t_references::iterator ri;
				t_nntp_file_parts::iterator pi;
				t_nntp_server_articles::iterator sai;
				c_nntp_server_article *sa;
				c_nntp_part *np;
				f->putf(CACHE_VERSION"\t%lu\n",totalnum);//START_MODE
				//vv SERVERINFO_MODE
				while (!server_info.empty()){
					si=server_info.begin()->second;
					assert(si);
					f->putf("%lu\t%lu\t%lu\t%lu\n",si->serverid,si->high,si->low,si->num);//mode 4
					server_info.erase(server_info.begin());
					delete si;
				}
				f->putf(".\n");
				//end SERVERINFO_MODE
				//vv FILE_MODE
				for(i = files.begin();i!=files.end();++i){
					nf=(*i).second;
					assert(!nf.isnull());
					assert(!nf->parts.empty());
					f->putf("%i\t%lu\t%lu\t%s\t%s\t%i\t%i\n",nf->req,nf->flags,nf->fileid,nf->subject.c_str(),nf->author.c_str(),nf->partoff,nf->tailoff);//FILE_MODE
					for(ri = nf->references.begin();ri!=nf->references.end();++ri){
						f->putf("%s\n",ri->c_str());//REFERENCES_MODE
					f->putf(".\n");//end REFERENCES_MODE
					for(pi = nf->parts.begin();pi!=nf->parts.end();++pi){
						np=(*pi).second;
						assert(np);
						f->putf("%i\t%lu\t%s\n",np->partnum,np->date,np->messageid.c_str());//PART_MODE
						for (sai = np->articles.begin(); sai != np->articles.end(); ++sai){
							sa=(*sai).second;
							assert(sa);
							f->putf("%lu\t%lu\t%lu\t%lu\n",sa->serverid,sa->articlenum,sa->bytes,sa->lines);//SERVER_ARTICLE_MODE
							counta++;
						}
						f->putf(".\n");//end SERVER_ARTICLE_MODE
						count++;
					}
					f->putf(".\n");//end PART_MODE
					(*i).second=NULL; //free cache as we go along instead of at the end, so we don't swap more with low-mem.
					//nf->storef(f);
					//delete nf;
					//nf->dec_rcount();
				f->close();
			}catch(FileEx &e){
				printCaughtEx(e);
				if (unlink(tmpfn.c_str()))
					perror("unlink:");
				fatal_exit();
				fatal_exit();
			fatal_exit();
	c_message_state::ptr s;
	if (save())
		fatal_exit();
		try {
			auto_ptr<c_file> fcloser(f);
			if (debug){printf("saving mid_info: %i infos..",states.size());fflush(stdout);}
			t_message_state_list::iterator sli;
			c_message_state::ptr ms;
			for (sli=states.begin(); sli!=states.end(); ++sli){
				ms=(*sli).second;
				if (ms->date_removed==TIME_T_DEAD)
					continue;
				f->putf("%s %li %li\n",ms->messageid.c_str(),ms->date_added,ms->date_removed);
				nums++;
			}
			if (debug) printf(" (%i) done.\n",nums);
			f->close();
		}catch(FileEx &e){
			printCaughtEx(e);
			if (unlink(tmpfn.c_str()))
				perror("unlink:");
			return -1;
	}else {
		printf("error opening %s: %s(%i)\n",tmpfn.c_str(),strerror(errno),errno);
	}