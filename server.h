/*
    server.* - nget configuration handling
    Copyright (C) 2000  Matthew Mueller <donut@azstarnet.com>

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
#ifndef _SERVER_H_
#define _SERVER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "datfile.h"
#include "misc.h"
#include "rcount.h"


class c_server {
	public:
		ulong serverid;
		string alias;
		string addr;
		string user,pass;
		bool fullxover;
		c_server(string id, string alia, string add, string use,string pas,const char *fullxove):alias(alia),addr(add),user(use),pass(pas){
			serverid=atoul(id.c_str());
			if (fullxove)
				fullxover=atoi(fullxove);
			else
				fullxover=0;
		}
};
typedef map<ulong,c_server*,less<ulong> > t_server_list;
//typedef map<ulong,c_data_section*,less<ulong> > t_server_list;


class c_server_priority {
	public:
		c_server *server;
		float baseprio;
		c_server_priority(c_server *serv,float basep):server(serv),baseprio(basep){}
};
typedef map<ulong,c_server_priority*, less<ulong> > t_server_priority_grouping;

class c_server_priority_grouping {
	public:
		string alias;
		float defglevel;
		float deflevel;
//		vector<c_server *> servers;
		t_server_priority_grouping priorities;
		float getserverpriority(ulong serverid){
			t_server_priority_grouping::iterator spgi=priorities.find(serverid);
			if (spgi!=priorities.end())
				return (*spgi).second->baseprio;
			else
				return deflevel;
		}
		c_server_priority_grouping(string alia):alias(alia){defglevel=1.0;deflevel=1.0;}
		~c_server_priority_grouping(){
			t_server_priority_grouping::iterator i;
			for (i=priorities.begin(); i!=priorities.end(); ++i)
				delete (*i).second;
		}
};
typedef map<const char *,c_server_priority_grouping*, ltstr> t_server_priority_grouping_list;

class c_group_info : public c_refcounted<c_group_info>{
	public:
		string alias;
		string group;
//		string priogrouping;
		c_server_priority_grouping *priogrouping;
		c_group_info(string alia, string name, c_server_priority_grouping *prio):alias(alia),group(name),priogrouping(prio){}
};
typedef map<const char *,c_group_info::ptr, ltstr> t_group_info_list;

class c_nget_config {
	private:
			struct serv_match_by_name {
				const char *mname;
				bool operator() (pair<ulong,c_server*> server) const{
					return (server.second->alias==mname || server.second->addr==mname);
				}
			};
	public:
		t_server_list serv;
		t_server_priority_grouping_list prioritygroupings;
		c_server_priority_grouping *trustsizes;
		t_group_info_list groups;
		float curservmult;

		c_server* getserver(string name){
			serv_match_by_name name_matcher;
			name_matcher.mname=name.c_str();
			t_server_list::iterator sli=find_if(serv.begin(),serv.end(),name_matcher);
			if (sli!=serv.end())
				return (*sli).second;
			return NULL;
		}
		c_server_priority_grouping* getpriogrouping(string groupname){
			t_server_priority_grouping_list::iterator spgli = prioritygroupings.find(groupname.c_str());
			if (spgli != prioritygroupings.end())
				return (*spgli).second;
			return NULL;
		}
		c_group_info::ptr addgroup(string alias, string name, string prio){
			if (name.empty())return NULL;
			if (prio.empty())prio="default";
			c_server_priority_grouping *priog=getpriogrouping(prio);
			assert(priog);
			c_group_info::ptr group(new c_group_info(alias,name,priog));
			if (!group.isnull()){
				if (!alias.empty())
					groups.insert(t_group_info_list::value_type(group->alias.c_str(),group));
				groups.insert(t_group_info_list::value_type(group->group.c_str(),group));
			}
			return group;
		}
		c_group_info::ptr getgroup(const char * groupname){
			t_group_info_list::iterator gili=groups.find(groupname);
			if (gili!=groups.end())
				return (*gili).second;
			return addgroup("",groupname,"");//do we even need to addgroup?  since groups are refcounted, could just return a new one.. nothing really needs it to be in the list..
		}
		void setlist(c_data_section *hinfo,c_data_section *pinfo,c_data_section *ginfo){
			c_server *server;
			data_list::iterator dli,dlj;
			c_data_section *ds;
			c_data_item *di;
			const char *sida;
			//halias
			assert(hinfo);
			for (dli=hinfo->data.begin();dli!=hinfo->data.end();++dli){
				di=(*dli).second;
				if (di->type!=1){
					printf("h not a section\n");
					continue;
				}
				//				ds=dynamic_cast<c_data_section*>(di);
				ds=(c_data_section*)(di);//TODO: ok, this is bad, but dynamic_cast doesn't work. ??
				if (!ds){
					printf("h !ds\n");continue;
				}
				if (!(sida=ds->getitema("id"))){
					printf("%s no id\n",ds->key.c_str());
					continue;
				}
				server=new c_server(sida,ds->key,ds->getitems("addr"),ds->getitems("user"),ds->getitems("pass"),ds->getitema("fullxover"));
				serv[server->serverid]=server;
			}
			//hpriority
			if (pinfo)
				for (dli=pinfo->data.begin();dli!=pinfo->data.end();++dli){
					di=(*dli).second;
					if (di->type!=1){
						printf("p not a section\n");
						continue;
					}
					//				ds=dynamic_cast<c_data_section*>(di);
					ds=(c_data_section*)(di);//TODO: ok, this is bad, but dynamic_cast doesn't work. ??
					if (!ds){
						printf("p !ds\n");continue;
					}
					c_server_priority_grouping *pgrouping=new c_server_priority_grouping(ds->key);
					for (dlj=ds->data.begin();dlj!=ds->data.end();++dlj){
						di=(*dlj).second;
						if (di->key=="_level"){
							pgrouping->deflevel=atof(di->str.c_str());
						}else if (di->key=="_glevel"){
							pgrouping->defglevel=atof(di->str.c_str());
						}else{
							c_server_priority *sprio=new c_server_priority(getserver(di->key),atof(di->str.c_str()));
							pgrouping->priorities.insert(t_server_priority_grouping::value_type(sprio->server->serverid,sprio));
						}
					}
					if (pgrouping->alias=="trustsizes")
						trustsizes=pgrouping;
					else
						prioritygroupings.insert(t_server_priority_grouping_list::value_type(pgrouping->alias.c_str(),pgrouping));
				}
			if (getpriogrouping("default")==NULL){
				c_server_priority_grouping *pgrouping=new c_server_priority_grouping("default");
				prioritygroupings.insert(t_server_priority_grouping_list::value_type(pgrouping->alias.c_str(),pgrouping));
			}
			if (trustsizes==NULL){
				trustsizes=new c_server_priority_grouping("trustsizes");
			}
			//galias
			if (ginfo)
				for (dli=ginfo->data.begin();dli!=ginfo->data.end();++dli){
					di=(*dli).second;
					if (di->type!=1){
						addgroup(di->key,di->str,"");
					}else{
						//				ds=dynamic_cast<c_data_section*>(di);
						ds=(c_data_section*)(di);//TODO: ok, this is bad, but dynamic_cast doesn't work. ??
						if (!ds){
							printf("g !ds\n");continue;
						}
						addgroup(ds->key,ds->getitems("group"),ds->getitems("prio"));
					}
				}
		}
		c_nget_config(){
			curservmult=2.0;
		}
		~c_nget_config(){
			t_server_list::iterator i;
			for (i=serv.begin(); i!=serv.end(); ++i)
				delete (*i).second;
			delete trustsizes;
		}
};
class c_group_info_list {
	public:
};

extern c_nget_config nconfig;

#endif
