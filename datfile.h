/*
    datfile.h - easy config/etc file handling
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
#ifndef _DATFILE_H_
#define _DATFILE_H_
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include <string>
#ifdef HAVE_HASH_MAP_H
#include <hash_map.h>
#else
#include <map.h>
#endif
#include <stdio.h>

#ifdef HAVE_HASH_MAP_H
struct eqstr 
{
    bool operator()(const char* s1, const char* s2) const 
	{
	    return strcmp(s1, s2) == 0;
	}
};
#else
struct ltstr
{
        bool operator()(const char* s1, const char* s2) const
	  {
                  return strcmp(s1, s2) < 0;
	  }
};
#endif

class c_data_item
{
	public:
		string key;
		string str; 
		long i;
		int ierr;//0=no error;-1=error after converting some;-2=not at all;-3=never been set
		int type;
		c_data_item(const char *k):key(k)
		{
			ierr=-3;i=0;
			type=0;
		};
		c_data_item(const char *k, const char *s):key(k)
		{
			set(s);
			type=0;
		};    
		c_data_item(const char *k, long l):key(k)
		{
			set(l);
			type=0;
		};    
		void set(long l)
		{
			str="";
			i=l;
			ierr=0;
		};
		void set(const char *val);
};

#ifdef HAVE_HASH_MAP_H
typedef hash_map<const char*, c_data_item*, hash<const char*>, eqstr> data_list ;
#else
typedef map<const char*, c_data_item*, ltstr> data_list ;
#endif
//typedef map<const char*, T_Data_Item_List, ltstr> T_Data_List ;

class c_data_section: public c_data_item
{
  public:
    data_list data;
/*	int getiteml(const char *name,long *l){
		c_data_item *i=rgetitem(name);
		if (i && i->type==0 && i->ierr==0) {*l=i->i;return 0;}
		return -1;
	};*/
	int getitemi(const char *name,int *l){
		c_data_item *i=rgetitem(name);
		if (i && i->type==0 && i->ierr==0) {*l=i->i;return 0;}
		return -1;
	};
	const char *getitema(const char *name){
		c_data_item *i=rgetitem(name);
		if (i && i->type==0) return i->str.c_str();
		return NULL;
	};
	c_data_item *getitem(const char *name){
		c_data_item *i=rgetitem(name);
		if (i && i->type==0) return i;
		return NULL;
	};
	c_data_section *getsection(const char *name){
		c_data_item *i=rgetitem(name);
		if (i && i->type==1) return (c_data_section*) i;
		return NULL;
	};
	c_data_item *rgetitem(const char *name);
	c_data_item *additem(c_data_item *item);
	int delitem(data_list::iterator i);
	int delitem(const char *name){
		return delitem(data.find(name));
	}
	
    void cleanup(void);
    c_data_section(const char *k):c_data_item(k)
	{
//	    data=new data_list;
//	    data["default"]=new c_data_item("test");
	    //ierr=-3;
		type=1;
	};
    virtual ~c_data_section()
	{
	    cleanup();
	};
};
//typedef hash_map<const char*, data_section*, hash<const char*>, eqstr> t_data_list ;

//typedef map<const char*, T_Data_Item_List, ltstr> T_Data_File ;
class c_data_file
{
//    FILE *f;
//    t_data_file data;
    int changed;
  public:
    string filename;
    c_data_section data;

    void setfilename(const char * f);

    void read_list(char *buf,int len,FILE *f, c_data_section *c);
    void read(void);
    void save_list(int &r,FILE *f, c_data_section *c,const char *cname);
    void save(void);

    
//    c_data_section* find (char*section);
//    c_data_section* find (char*format,...);

	c_data_file(void):data(""){}
//    c_data_section* findif(Predicate Pred)
//	{
//	    data_list::iterator i=find_if(data.begin(),data.end(),Pred);
//	    if (i==data.end())return NULL;
//	    else return (*i).second;	    
//	};
    
//    ~c_data_file()
//	{
//	    if (changed) save();
//	    data.cleanup();
//	};
    
//    data_section* findif(char *name, char*val);
//    data_section* findifa(char *name, char*val,char *name2, char*val2);
};
#endif
