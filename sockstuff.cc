#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include "sockstuff.h"
//#include <netdb.h>
//#include <netinet/in.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>

#include <sys/time.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "log.h"
#ifdef WRFTP
#include "main.h"
#endif
#include "misc.h"

int sock_timeout=3*60;

/* Take a service name, and a service type, and return a port number.  If the
service name is not found, it tries it as a decimal number.  The number
returned is byte ordered for the network. */

int atoport(const char *service, const char *proto,char * buf, int buflen) {
	int port;
	long int lport;
	struct servent servb, *serv;
	char *errpos;
	/* First try to read it from /etc/services */
#ifdef HAVE_GETSERVBYNAME_R
	getservbyname_r(service,proto,&servb,buf,buflen,&serv);
#else
	serv = getservbyname(service, proto);
#endif
	if (serv != NULL)
		port = serv->s_port;
	else { /* Not in services, maybe a number? */
		lport = strtol(service,&errpos,0);
		if ( (errpos[0] != 0) || (lport < 1) || (lport > 65535) )
			return -1; /* Invalid port address */
		port = htons(lport);
	}
	return port;
}


/* This is a generic function to make a connection to a given server/port.
service is the port name/number,
type is either SOCK_STREAM or SOCK_DGRAM, and
netaddress is the host name to connect to.
The function returns the socket, ready for action.*/
int make_connection(const char *defservice, int type,const char *netaddress,const char *service,char * buf, int buflen){
	PERROR("make_connection(%s,%i,%s,%s,%p,%i)",defservice,type, netaddress, service,buf, buflen);
	/* First convert service from a string, to a number... */
	int port = -1;
	struct in_addr *addr;
	int sock, connected;
	struct sockaddr_in address;
	//	char *tmp;
	struct hostent hostb,*host;
	static struct in_addr saddr;
	int err;

	if (!(service && *service))
		service=defservice;

	//	if ((tmp=strrchr(netaddress,':')))
	//	     service=tmp+1;
	if (type == SOCK_STREAM)
		port = atoport(service, "tcp",buf,buflen);
	else if (type == SOCK_DGRAM)
		port = atoport(service, "udp",buf,buflen);
	if (port == -1) {
		PERROR("make_connection:  Invalid socket type %s",service);
		return -1;
	}
	//	if (tmp)
	//	     *tmp=0;
	if (inet_aton(netaddress,&saddr)) {
		addr=&saddr;
	} else {
#ifdef HAVE_GETHOSTBYNAME_R
		gethostbyname_r(netaddress,&hostb,buf,buflen,&host,&err);
#else
		host = gethostbyname(netaddress);
#endif
		if (host != NULL) {
			addr=(struct in_addr *)*host->h_addr_list;
		}
		else addr=NULL;
	}
	if (addr == NULL) {
		PERROR("make_connection:  Invalid network address %s (%i %i)",netaddress,err,errno);
		//		if (tmp) *tmp=':';
		return -1;
	}
	//	if (tmp) *tmp=':';

	unsigned char *i;
	i=(unsigned char *)&addr->s_addr;
	PMSG("Connecting to %i.%i.%i.%i:%i",i[0],i[1],i[2],i[3],ntohs(port));

	memset((char *) &address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = (port);
	address.sin_addr.s_addr = addr->s_addr;

	sock = socket(AF_INET, type, 0);


	//  printf("Connecting to %s on port %d.\n",inet_ntoa(*addr),htons(port));

	if (type == SOCK_STREAM) {
#ifdef HAVE_SELECT
		fcntl(sock,F_SETFL,O_NONBLOCK);//we can set the sock to nonblocking, and use select to enforce a timeout.
#endif
		connected = connect(sock, (struct sockaddr *) &address,
				sizeof(address));
#ifdef HAVE_SELECT
		if (connected<0 && errno==EINPROGRESS){
			fd_set w;
			struct timeval tv;
			int i;
			tv.tv_sec=sock_timeout;
			tv.tv_usec=0;
			FD_ZERO(&w);
			FD_SET(sock,&w);
			if ((i=select(sock+1,NULL,&w,NULL,&tv))>0){
				int erp;
				socklen_t erl=sizeof(erp);
				if (getsockopt(sock,SOL_SOCKET,SO_ERROR,&erp,&erl)){
					PERROR("getsockopt: %s",strerror(errno));
					return -1;
				}
				if (erp) {
					PERROR("connect: %s",strerror(erp));
					return -1;
				}
			}else{
				PERROR("connection timed out\n");
				close(sock);
				return -1;
			}
			fcntl(sock,F_SETFL,0);//set to blocking again (I hope)
			connected=0;
		}
#endif
		if (connected < 0) {
			PERROR("connect: %s",strerror(errno));
			return -1;
		}
		return sock;
	}
	/* Otherwise, must be for udp, so bind to address. */
	if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
		PERROR("bind: %s",strerror(errno));
		return -1;
	}
	return sock;
}





int getsocketaddress(int s, struct sockaddr_in *addr){
	socklen_t len = sizeof(struct sockaddr_in);

	if (getsockname(s, (struct sockaddr *)addr, &len) < 0) {
		PERROR("getsockname: %s",strerror(errno));
		return -1;
	}
	return 0;
}       /* GetSocketAddress */






/* This function listens on a port, and returns connections. 
* 
*  The parameters are as follows:
*    socket_type: SOCK_STREAM or SOCK_DGRAM (TCP or UDP sockets)
*    port: The port to listen on.  Remember that ports < 1000 are
*	reserved for the root user.  Must be passed in network byte
*	order (see "man htons").
*/

int get_connection1(int socket_type, u_short port) {

	int gc_listening_socket;
	struct sockaddr_in gc_address;
	int gc_reuse_addr = 1;


	/* Setup internet address information.  
	   This is used with the bind() call */
	memset((char *) &gc_address, 0, sizeof(gc_address));
	gc_address.sin_family = AF_INET;
	gc_address.sin_port = port;
	gc_address.sin_addr.s_addr = htonl(INADDR_ANY);

	gc_listening_socket = socket(AF_INET, socket_type, 0);
	if (gc_listening_socket < 0) {
		PERROR("socket: %s",strerror(errno));
		return(-1);
	}

	setsockopt(gc_listening_socket, SOL_SOCKET, SO_REUSEADDR, &gc_reuse_addr,
			sizeof(gc_reuse_addr));

	if (bind(gc_listening_socket, (struct sockaddr *) &gc_address, 
				sizeof(gc_address)) < 0) {
		PERROR("bind: %s",strerror(errno));
		close(gc_listening_socket);
		return (-2);
	}
	if (socket_type == SOCK_STREAM)
		listen(gc_listening_socket, 1);
	return gc_listening_socket;
}
int get_connection2(int socket_type,int gc_listening_socket)
{
	int connected_socket = -1;
	if (socket_type == SOCK_STREAM) {

		while(connected_socket < 0) {
			connected_socket = accept(gc_listening_socket, NULL, NULL);
			if (connected_socket < 0) {
				/* Either a real error occured, or blocking was interrupted for
				   some reason.  Only abort execution if a real error occured. */
				if (errno != EINTR) {
					PERROR("accept: %s",strerror(errno));
					close(gc_listening_socket);
					return (-3);
				} else {
					PERROR("accept1: %s",strerror(errno));
					continue;    /* don't fork - do the accept again */
				}
			}

			//	close(gc_listening_socket); /* Close our copy of this socket */

		}
		close(gc_listening_socket);
		return connected_socket;
	}
	else
		return gc_listening_socket;
}




/* This is just like the write() system call, accept that it will
make sure that all data is transmitted. */
int sock_write(int sockfd, const char *buf, size_t count) {
	size_t bytes_sent = 0;
	int this_write;

	while (bytes_sent < count) {
		//    do
		this_write = write(sockfd, buf, count - bytes_sent);
		//    while ( (this_write < 0) && (errno == EINTR) );
		if (this_write <= 0)
			return this_write;
		bytes_sent += this_write;
		buf += this_write;
	}
	return count;
}


//like read(), but uses select() to enforce a timeout.  read() blocks forever if the connection is lost unexpectadly (modem disconnect, etc)
int sock_read(int sockfd, void *buf, size_t count){
#ifdef HAVE_SELECT
	fd_set r;
	struct timeval tv;
	int i;
	tv.tv_sec=sock_timeout;
	tv.tv_usec=0;
	FD_ZERO(&r);
	FD_SET(sockfd,&r);
	if ((i=select(sockfd+1,&r,NULL,NULL,&tv))>0){
#endif
		return read(sockfd,buf,count);
#ifdef HAVE_SELECT
	}
	return i;
#endif
}

/* This function reads from a socket, until it recieves a linefeed
   character.  It fills the buffer "str" up to the maximum size "count".

   This function will return -1 if the socket is closed during the read
   operation.

   Note that if a single line exceeds the length of count, the extra data
   will be read and discarded!  You have been warned. */
int sock_gets(int sockfd, char *str, size_t count) {
//  int bytes_read;
  size_t total_count = 0, real_total=0;
  char *current_position;
  char last_read = 0;

  current_position = str;
  while (last_read != 10) {
//    bytes_read = read(sockfd, &last_read, 1);
//    if (bytes_read <= 0) {
    //if (read(sockfd, &last_read, 1) <= 0) {
    if (sock_read(sockfd, &last_read, 1) <= 0) {
      /* The other side may have closed unexpectedly */
      return -1; /* Is this effective on other platforms than linux? */
    }
     real_total++;
    if ( (total_count < count) && (last_read != 10) && (last_read !=13) ) {
      current_position[0] = last_read;
      current_position++;
      total_count++;
    }
  }
  if (count > 0)
    current_position[0] = 0;
//  return total_count;
  return real_total;
}

/* This function writes a character string out to a socket.  It will 
   return -1 if the connection is closed while it is trying to write. */
int sock_puts(int sockfd, const char *str) {
	return sock_write(sockfd, str, strlen(str));
}


int sockprintf(int sockfd, const char *str, ...) {
#ifndef HAVE_VDPRINTF
	static char fpbuf[255];
#endif
	va_list ap;

	va_start(ap,str);
#ifdef HAVE_VDPRINTF
	int i=vdprintf(sockfd,str,ap);
	va_end(ap);
	return i;
#else
	vsprintf(fpbuf,str,ap);
	va_end(ap);
	return sock_write(sockfd, fpbuf, strlen(fpbuf));
#endif
}
