#include "file.h"
#include <stdarg.h>

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
}

c_file::~c_file(){
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
	return 0;
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
int c_file_gz::isopen(void){
	return (gzh!=0);
}
inline size_t c_file_gz::dowrite(const void *data,size_t len){
	return gzwrite(gzh,data,len);
}
inline size_t c_file_gz::doread(void *data,size_t len){
	return gzread(gzh,data,len);
}
inline char * c_file_gz::dogets(char *data,size_t len){
	return gzgets(gzh,data,len);
}
//int c_file_optgz::open(const char *name,const char * mode,int gz){
//	return (!(gzh=gzopen(name,mode)));
//}
#endif

//size_t c_file_stream::dowrite(void *buf,size_t len);
//size_t c_file_stream::doread(void *buf,size_t len);
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
int c_file_stream::isopen(void){
	return (fs!=0);
}
