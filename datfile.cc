/*
    datfile.cc - easy config/etc file handling
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
#include "datfile.h"

#define df_BUFSIZE 1024
void c_data_item::set(const char *val){
	char *err;
	str=val;
	i=strtol(val,&err,0);
	if (err==val) ierr=-2;
	else if ((err-val)<(signed)str.size()) ierr=-1;
	else ierr=0;
};



c_data_item *c_data_section::rgetitem(const char *name){
	data_list::iterator i=data.find(name);
	if (i!=data.end()) return (*i).second;
	return NULL;
};
int c_data_section::delitem(data_list::iterator i){
	if (i!=data.end()){
		delete (*i).second;
		data.erase(i);
		return 1;
	}
	return 0;
}
c_data_item *c_data_section::additem(c_data_item *item){
	data_list::iterator i=data.find(item->key.c_str());
	if (i!=data.end()){
		delete (*i).second;
		data.erase(i);
	}
	data.insert(data_list::value_type(item->key.c_str(),item));
	return item;
};
void c_data_section::cleanup(void){
	data_list::iterator i;
	i=data.begin();
	while ((i!=data.end())){
		delete (*i).second;
		++i;
	}
	//	    delete data;
	//	    data=new data_list;

	//	    erase(begin(),end());
	//	    while (begin()!=end())
	//		{
	//		    printf("%x %x\n",begin(),end());
	//		    if ((*begin()).second)
	//			  delete((*begin()).second);
	//		    erase((*begin()).first);
	//		    erase(begin());
	//		}
};



void c_data_file::setfilename(const char * f){
	filename=f;
};

void c_data_file::read_list(char *buf,int len,FILE *f, c_data_section *c){
	char *n,*v;
	int slen,r;
	while (fgets(buf,len,f)){
		slen=strlen(buf);
		while (slen>0 && (buf[slen-1]=='\n' || buf[slen-1]=='\r')){
			buf[--slen]=0;
		}
		if (slen<1)
			continue;
		r=strspn(buf," \t");
	    if (buf[r]=='}') {
//			printf("end section\n");
			break;
		}
		if (!(slen>r))
			continue;
	    if (buf[r]=='{')
		{
			c_data_section *d=new c_data_section(buf+r+1);
			//		c->data[buf+r+1]=d;
			c->additem(d);
//			printf("new section: %s\n",buf+r+1);
			read_list(buf,len,f,d);
		}
		else if (buf[0]!='/')
		{
		    n=buf+r;
			if((v=strchr(n,'='))){
				*v=0;
				v++;
				c->additem(new c_data_item(n,v));
//				printf("new item: %s=%s\n",n,v);
			}
		}
	}
}

void c_data_file::read(void){
	if (filename.size()==0)
	     return;
	char buf[df_BUFSIZE];
	FILE *f=fopen(filename.c_str(),"r");
	if (f){
		read_list(buf,df_BUFSIZE,f,&data);
		changed=0;
		fclose(f);
	}
};
void c_data_file::save_list(int &r,FILE *f, c_data_section *c,const char *cname){
	data_list::iterator i;
	c_data_item *item;
	if (cname)
		fprintf(f,"%*s{%s\n",r,"",cname);
	r++;
	i=c->data.begin();
//	printf("saving %i items\n",c->data.size());
	while (i!=c->data.end()){
		item=(*i).second;
//		item->feel();
		if (item->type==1)
		     save_list(r,f,(c_data_section*)item,(*i).first);
		else{
		     fprintf(f,"%*s%s=%s\n",r,"",(*i).first,item->str.c_str());
//			 printf("saving %*s%s=%s\n",r,"",(*i).first,item->str.c_str());
		}
		++i;
	}
	r--;
	if (cname)
		fprintf(f,"%*s}\n",r,"");
};
void c_data_file::save(void){
	if (filename.size()==0)
	     return;
//	dobackup(filename.str());
	int r=-1;
	FILE *f=fopen(filename.c_str(),"w");
	if (f){
		save_list(r,f,&data,NULL);
		fclose(f);
		changed=0;
	}
};

