/*
    sockstuff.* - socket handling code
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
#ifndef _SOCKSTUFF_H_
#define _SOCKSTUFF_H_
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
//#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 

int atoport(const char *service,const char *proto,char * buf, int buflen);
//int make_connection(char *service, int type, char *netaddress,char * buf, int buflen);
int make_connection(const char *defservice, int type,const char *netaddress,const char *service,char * buf, int buflen);

int getsocketaddress(int s, struct sockaddr_in *addr);
int get_connection1(int socket_type, u_short port);
int get_connection2(int socket_type,int gc_listening_socket);
//int get_connection2(int socket_type);
int sock_write(int sockfd, const char *buf, size_t count);
int sock_read(int sockfd, void *buf, size_t count);
int sock_gets(int sockfd, char *str, size_t count);
int sock_puts(int sockfd, const char *str);

int sockprintf(int sockfd, const char *str, ...)
        __attribute__ ((format (printf, 2, 3)));
#endif
