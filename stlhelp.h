/*
    stlhelp.* - common stl helper structs
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
#ifndef _STLHELP_H_
#define _STLHELP_H_

struct eqstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) == 0;
	}
};
struct ltstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

//string class that won't segv on NULL to constructor.  todo: also protect assign() ?
#include <string>
template <class basestring,class charT>
class safe_string : public basestring {
	public:
		safe_string(const charT *s):basestring(){
			if (s) assign(s);
		}
		safe_string(const charT *s,int l):basestring(){
			if (s) assign(s,l);
		}
		safe_string(int l,charT c):basestring(l,c){}
		safe_string():basestring(){}
};
typedef safe_string<string,char> safestring;


#endif