/*
    log.* - debug/error logging and exception defines
    Copyright (C) 1999-2000  Matthew Mueller <donut@azstarnet.com>

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
#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include "misc.h"

int quiet=0,debug=0;
c_error::c_error(int n, const char * s, ...){
	va_list ap;
	num=n;
	va_start(ap,s);
	vasprintf(&str,s,ap);
	va_end(ap);
//	printf("c_error %i constructed: %s\n",num,str);
}
c_error::~c_error(){
	if (str) free(str);
	//printf("c_error %i deconstructed\n",num);
};
