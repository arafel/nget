#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include "file.h"
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sockstuff.h"

//c_buffer::c_buffer(void){
//	buf=NULL;bufsize=0;bufused=0;bufdef=0;bufstep=4096;bufhigh=0;
//}
//c_buffer::~c_buffer(void){
//	if (buf) free(buf);
//}
//int c_buffer::bufput(void *data, size_t len){
//	if (bufused+len>bufhigh){
//		return 1;
//	}else if (bufused+len>bufsize){
//		size_t i;
//		i=((bufused+len)/bufstep + 1)*bufstep;
//		if (!resize(i))
//		     return -1;
//	}
//	memcpy(buf+bufused,data,len);
//	bufused+=len;
//	return 0;
//}
//int bufget(void * data, size_t len){
//	if (bufused<len)
//	     return 1;
//	memcpy(data,buf+bufoff,len);
//	bufused-=len;
//	bufoff+=len;
//}
//int c_buffer::resize(int size){
//	void *v;
//	if ((v=realloc(buf,size))){
//		buf=v;
//		bufsize=size;
//		return 0;
//	}else
//	     return -1;
//}
//void c_buffer::empty(void){
//	bufused=0;bufoff=0;
//	if (bufsize>bufhigh)
//	     resize(bufhigh);
//}

c_file::c_file(void){
//	ftype=-1;
//	fh=-1;sh=NULL;
//#ifdef HAVE_LIBZ
//	gzh=NULL;
//#endif 
	rbuf=NULL;
	rbufsize=0;
	rbufstate=0;
	resetrbuf();
}

c_file::~c_file(){
	if (rbuf)free(rbuf);
//	close();//cant call it here, since doclose is already gone
}

size_t c_file::putf(const char *data,...){
	char *fpbuf;
	int i,l;
	va_list ap;

	va_start(ap,data);
	l=vasprintf(&fpbuf,data,ap);
	va_end(ap);

	//	if (wbuf.bufput(data,len)){
	//		dowrite(wbuf.buf,wbuf.bufused);
	//		dowrite(data,len);
	//		wbuf.empty();
	//	}
	//	return len;
	i=dowrite(fpbuf,l);
	delete(fpbuf);
	return i;
}
char * c_file::gets(char *data,size_t len){
//	if (wbuf.used>len)
	return dogets(data,len);
}
size_t c_file::write(const void *data,size_t len){
//	if (wbuf.bufput(data,len)){
//		dowrite(wbuf.buf,wbuf.bufused);
//		dowrite(data,len);
//		wbuf.empty();
//	}
//	return len;
	return dowrite(data,len);
}
size_t c_file::read(void *data,size_t len){
//	if (wbuf.used>len)
	return doread(data,len);
}

//int c_file::open(const char *name,const char * mode){
//	close();
//	return doopen(name,mode);
//}
int c_file::flush(int local=0){
//	int i=0;
//###########3 dowrite(buffers..)
	if (!local)
	     return doflush();
	return 0;
}
int c_file::close(void){
	if (isopen()){
		flush(1);
		return doclose();
	}
	resetrbuf();rbufstate=0;
	return 0;
}
unsigned int c_file::initrbuf(unsigned int s){
	int p=rbufp-rbuf;
	if (s<rbufused)
		s=rbufused;
	if ((rbuf=(char*)realloc(rbuf,s))){//If ptr is NULL, equivalent to malloc()
		rbufp=rbuf+p;
		rbufsize=s;
	}
	return rbufsize;
};
size_t c_file::bread(size_t len){
	if (len>rbufsize)len=rbufsize;
	size_t temp=doread(rbuf+rbufused,len-rbufused)+rbufused-rbufoff;
	rbufp=rbuf+rbufoff;
	resetrbuf();rbufstate=0;
	return temp;
}
int c_file::bgets_sub(void){
//	int rbufstate=0;//state needs to be persistant, in order to compensate for a buffer that ends with \r, but the next
	//char happens to be \n.  This would previously return an erroneous blank line.
	unsigned int i;
	int istate=rbufstate;
	if (rbufstate==2)rbufstate=0;
//	else if (rbufstate==3)rbufstate=4;
	for (i=rbufoff;i<rbufused;i++){
		if (rbufstate==0 && rbuf[i]=='\r'){
			rbuf[i]=0;rbufstate=1;rbufoff=i+1;
		}
		else if (rbuf[i]=='\n'){
			rbuf[i]=0;rbufoff=i+1;
			if (rbufstate<3){
				rbufstate=2;break;
			}else{
				rbufstate=0;
				//rbufofp++;
				rbufp++;
			}
		}else if(rbufstate){//line is \r terminated only. odd, but ok.
			if (rbufstate>=3)
				rbufstate=0;
			else{
				rbufstate=2;
				break;
			}
		}
	}
	if (rbufstate==0)
		rbufoff=i;
	else if (rbufstate==1)
		rbufstate=3;
	else if (istate==3 &&rbufstate==3)//if there were no chars in the buffer, don't return that we have another line
		rbufstate=4;
//	else
//		if(rbufignorenext)rbufstate=0;
	return (rbufstate>0 && rbufstate<4);
}
int c_file::bgets(void){
	//unsigned int rbos=rbufoff;
	rbufofp=rbufoff;
	rbufp=rbuf+rbufoff;
	size_t t;
	while (!bgets_sub()){
		if (rbufused<rbufsize-1){
			if ((t=doread(rbuf+rbufused,rbufsize-rbufused-1))<=0){
				if(rbufofp<rbufused){//if we have some chars still, return em.
					rbuf[rbufused]=0;
					break;
				}
//				return NULL;//otherwise, signal an error.
				return -1;
			}
			rbufused+=t;
		}else{
			if (rbufofp>0){//we are at the end, but have some empty space at the start.. use it.
				memmove(rbuf,rbufp,rbufsize-rbufofp);
				rbufp=rbuf;
				rbufused-=rbufofp;
				rbufoff-=rbufofp;
				rbufofp=0;
			}else{
				rbuf[rbufused]=0;//welp, looks like we got to the end of the buffer, but not the end of the line.  oh well.
//				rbufignorenext=1;
				break;
			}
		}
	}
	int l=rbufoff-rbufofp;
	if (rbufoff>=rbufused)
		resetrbuf();
	return l;
}

#ifdef HAVE_LIBZ
int c_file_gz::open(const char *name,const char * mode){
	close();
	return (!(gzh=gzopen(name,mode)));
}
int c_file_gz::doflush(void){
	if (gzh)
	     return gzflush(gzh,Z_SYNC_FLUSH);
	return 0;
}
int c_file_gz::doclose(void){
	int i=0;
	i=gzclose(gzh);
	gzh=NULL;
	return i;
}
inline int c_file_gz::isopen(void){
	return (gzh!=0);
}
inline size_t c_file_gz::dowrite(const void *data,size_t len){
	return gzwrite(gzh,(void*)data,len);
}
inline size_t c_file_gz::doread(void *data,size_t len){
	return gzread(gzh,data,len);
}
inline char * c_file_gz::dogets(char *data,size_t len){
	return gzgets(gzh,data,len);
}
#endif

int c_file_stream::open(const char *name,const char * mode){
	close();
	return (!(fs=fopen(name,mode)));
}
int c_file_stream::doflush(void){
	if (fs)
	     return fflush(fs);
	return 0;
}
int c_file_stream::doclose(void){
	int i=0;
	i=fclose(fs);
	fs=NULL;
	return i;
}
inline int c_file_stream::isopen(void){
	return (fs!=0);
}
inline size_t c_file_stream::dowrite(const void *data,size_t len){
	return fwrite(data,1,len,fs);
}
inline size_t c_file_stream::doread(void *data,size_t len){
	return fread(data,1,len,fs);
}
inline char * c_file_stream::dogets(char *data,size_t len){
	return fgets(data,len,fs);
}


int c_file_tcp::open(const char *host,const char * port){
	close();
//	return (!(fs=fopen(name,mode)));
	if (rbuf && rbufsize>512){
		sock=make_connection(port,SOCK_STREAM,host,NULL,rbuf,rbufsize);
	}
	else{
		char buf[512];
		sock=make_connection(port,SOCK_STREAM,host,NULL,buf,512);
	}
	if (sock<0)return sock;
	else return 0;
}
int c_file_tcp::doflush(void){
	if (sock>=0)
	     return fsync(sock);
	return 0;
}
int c_file_tcp::doclose(void){
	int i=0;
	i=::close(sock);
	sock=-1;
	return i;
}
inline int c_file_tcp::isopen(void){
	return (sock>=0);
}
inline size_t c_file_tcp::dowrite(const void *data,size_t len){
	return sock_write(sock,(char*)data,len);
}
inline size_t c_file_tcp::doread(void *data,size_t len){
	return sock_read(sock,data,len);
}
inline char * c_file_tcp::dogets(char *data,size_t len){
	int i=sock_gets(sock,data,len);
	if (i<=0)return NULL;
	return data;
	//return sock_gets(sock,data,len);
}
