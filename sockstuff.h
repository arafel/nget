#ifndef _SOCKSTUFF_H_
#define _SOCKSTUFF_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/socket.h>
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
int sock_read(int sockfd, char *buf, size_t count);
int sock_gets(int sockfd, char *str, size_t count);
int sock_puts(int sockfd, const char *str);

int sockprintf(int sockfd, const char *str, ...)
        __attribute__ ((format (printf, 2, 3)));
#endif
