/*
    nget - command line nntp client
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
#ifndef _NGET_H_
#define _NGET_H_
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include "server.h"
//#include "datfile.h"

//extern c_data_file cfg;
extern time_t lasttime;
extern string nghome;
/*extern c_server_list servers;
extern c_group_info_list groups;
extern c_server_priority_grouping_list priogroups;*/

#define PRIVMODE (S_IRUSR | S_IWUSR)
#define PUBMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
//#define PUBXMODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define PUBXMODE (S_IRWXU|S_IRWXG|S_IRWXO)

#endif
