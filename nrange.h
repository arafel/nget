/*
    nrange.* - stores a set of numbers in a non-memory-hogging way (and speedy too)
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
#ifndef _NRANGE_H_
#define _NRANGE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string>
#include <sys/types.h>
#include <map.h>
#include <stdio.h>
#include "file.h"


typedef map<ulong, ulong, less<ulong> > t_rlist;//we are going to use high as the key, and low as the value
class c_nrange{
	public:
//		s_lrange water;
		int changed;
		string file;
		t_rlist rlist;
		int check(ulong n){
			t_rlist::iterator i=rlist.lower_bound(n);
			if (i!=rlist.end() && (*i).second<=n)
				return 1;
			return 0;
		}
		void insert(ulong n);
		//void insert(ulong l,ulong h);
		void insert(ulong l,ulong h){
			for (ulong i=l;i<=h;i++)
				insert(i);//ok, this could really be optomized a lot with a custom func, but I don't feel like doing that now, and inserting this little func here will let me do it later if I want to.
		}
//		void remove(ulong n){remove(n,n);}
		void remove(ulong l, ulong h);
		void print(c_file *f){
			t_rlist::iterator i;
			//int first=1;
			for (i=rlist.begin();i!=rlist.end();++i){
//				if (first)first=0;
//				else printf(",");
//				printf("(%lu)",(*i).first);
				if ((*i).second==(*i).first)
					f->putf("%lu\n",(*i).second);
				else
					f->putf("%lu-%lu\n",(*i).second,(*i).first);
//				f.putf("\n");
			}
//			printf("\n");
		}
		c_nrange(string f);
		~c_nrange();
};
#endif
