#include "cache.h"
#include "misc.h"
c_nntp_cache_item* c_nntp_cache_item::load(char *s){
	char *t[6], *p;
	int i;
	p=s;
	s[strlen(s)-1]=0;//get rid of \n
	for(i=0;i<6;i++)
		if((t[i]=goodstrtok(&p,'\t'))==NULL){
			i=-1;return NULL;
		}
	c_nntp_cache_item *c=new c_nntp_cache_item(atol(t[0]),atol(t[1]),atol(t[2]),atol(t[3]),t[4],t[5]);
	return c;
};
c_nntp_cache_item::c_nntp_cache_item(long n,time_t d,long s, long l, string a, string e):subject(a), email(e){
	num=n;date=d;size=s;lines=l;
};
int c_nntp_cache_item::store(c_file *f){
	return f->putf("%i\t%i\t%i\t%i\t%s\t%s\n",num,date,size,lines,subject.c_str(),email.c_str());
};
void c_cache::additem(c_nntp_cache_item* i){
	items[i->num]=i;
	//items.push_back(i);
	if (i->num>high)
		high=i->num;
	if (i->num<low)
		low=i->num;
}
c_cache::c_cache(string path,string nid):file(path),high(0),low(INT_MAX){
	c_file_gz f;
	char buf[2048];
	c_nntp_cache_item*i;
	file.append(nid);
	file.append(".gz");
	if (!f.open(file.c_str(),"r")){
		while (f.gets(buf,2048)){
			if((i=c_nntp_cache_item::load(buf))){
				additem(i);
			}
		}
		f.close();
	}
};
c_cache::~c_cache(){
	c_file_gz f;
	if (!f.open(file.c_str(),"w")){
		t_cache_items::iterator cur;
		c_nntp_cache_item *i;
		for(cur = items.begin();cur!=items.end();++cur){
			i=(*cur).second;
			i->store(&f);
			delete i;
		}
		f.close();
	}
};
