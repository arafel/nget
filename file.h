#ifndef _FILE_H_
#define _FILE_H_
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
//#include <unistd.h>
#include <stdio.h>

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif 


//class c_buffer {
//  private:
//	int resize(int size);
//  public:
//	char *buf;
//	int bufsize, bufused, bufdef, bufstep, bufhigh,bufoff;
	
//	int bufput(size_t len);
//	int bufget(void * data, size_t len);
//	void empty(void);
//	c_buffer(void);
//	~c_buffer();
//}

class c_file {
//	unsigned long size;
  private:
//	c_buffer rbuf,wbuf;
//	int ftype;
//	int fh;
//	FILE *sh;
//#ifdef HAVE_LIBZ
//	gzFile gzh;
//#endif 
	
	virtual char * dogets(char *data,size_t len)=0;
	virtual size_t dowrite(const void *data,size_t len)=0;
	virtual size_t doread(void *data,size_t len)=0;
//	virtual int doopen(const char *name,const char * mode)=0;
	virtual int doflush(void)=0;
	virtual int doclose(void)=0;
	int bgets_sub(void);
	void resetrbuf(void){rbufused=0;rbufoff=0;};//rbufstate=0;};//rbufignorenext=0;};
  protected:
	unsigned int rbufsize,rbufused,rbufofp,rbufoff;//,rbufignorenext;
	int rbufstate;
	char *rbuf;
	
  public:
	char *rbufp;
	
	c_file(void);
	virtual ~c_file();
	size_t putf(const char *buf,...)
        __attribute__ ((format (printf, 2, 3)));
	char * gets(char *buf,size_t len);
	size_t write(const void *buf,size_t len);
	size_t read(void *buf,size_t len);
	//buffered funcs: must call initrbuf first.
	size_t bread(size_t len);//buffered read, must be used instead of normal read, if you are using bgets
	//char * bgets(void);//buffered gets, should be faster than normal gets, definatly for tcp or gz. maybe not for stream.
	int bgets(void);//buffered gets, should be faster than normal gets, definatly for tcp or gz. maybe not for stream.
	unsigned int initrbuf(unsigned int s);
//	int open(const char *name,const char * mode);
	int flush(int local=0);
	int close(void);

	virtual int isopen(void)=0;
};

class c_file_stream : public c_file {
  private:
	FILE *fs;
	
	virtual char * dogets(char *data,size_t len);
	virtual size_t dowrite(const void *buf,size_t len);
	virtual size_t doread(void *buf,size_t len);
	virtual int doflush(void);
	virtual int doclose(void);
  public:
	virtual int isopen(void);	
	int open(const char *name,const char * mode);
	c_file_stream(void){fs=NULL;};
	~c_file_stream(){close();};
};
class c_file_tcp : public c_file {
  private:
	int sock;
	
	virtual char * dogets(char *data,size_t len);
	virtual size_t dowrite(const void *buf,size_t len);
	virtual size_t doread(void *buf,size_t len);
	virtual int doflush(void);
	virtual int doclose(void);
  public:
	virtual int isopen(void);	
	int open(const char *name,const char * mode);
	c_file_tcp(void){sock=-1;};
	~c_file_tcp(){close();};
};

#ifdef HAVE_LIBZ
class c_file_gz : public c_file {
  private:
	gzFile gzh;
	
	virtual char * dogets(char *data,size_t len);
	virtual size_t dowrite(const void *buf,size_t len);
	virtual size_t doread(void *buf,size_t len);
	virtual int doflush(void);
	virtual int doclose(void);
  public:
	virtual int isopen(void);	
	int open(const char *host,const char * port);
	c_file_gz(void){gzh=NULL;};
	~c_file_gz(){close();};
};
//class c_file_optgz : public c_file_gz, c_file_stream {
//  private:
//	
//	virtual size_t dowrite(void *buf,size_t len);
//	virtual size_t doread(void *buf,size_t len);
//	virtual int doflush(void);
//	virtual int doclose(void);
//  public:
//	virtual int isopen(void);	
//	int open(const char *name,const char * mode,int gz);
//};
//#else
//class c_file_optgz : public c_file_stream {
//  private:
//	int open(const char *name,const char * mode,int gz);
//};
#endif

#endif
