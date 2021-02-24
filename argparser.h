/*
    argparser.* - parse a string into a list of args
    Copyright (C) 2002  Matthew Mueller <donut AT dakotacom.net>

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
#ifndef __ARGPARSER_H__
#define __ARGPARSER_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <list>
#include <string>

using namespace std;

typedef list<string> arglist_t;
void parseargs(arglist_t &argl, const char *s, bool ignorecomments=false);

#endif
