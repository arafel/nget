#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
/* mystring.cpp  Matthew Mueller */
#include <stdarg.h>
#include <stdio.h>
#include "mystring.h"


int mystring::operator< (const mystring&s) const{
	return (strcmp(thestring,s.thestring)<0);
}
int mystring::operator== (const mystring&s) const{
	if (s.slen==slen) return (!strcmp(s.thestring,thestring));
	return 0;
}
int mystring::operator== (const char *s) const{
	return (!strcmp(s,thestring));
}

mystring& mystring::operator = ( const char * str){
	int i=strlen(str);
	if (slen!=i){
		delete thestring;
		slen=i;
		thestring=new char[slen+1];
	}
	strcpy(thestring,str);
	return *this;
}
mystring::mystring(const char * s){
	if (s) {
		slen=strlen(s);
		thestring=new char[slen+1];
		strcpy(thestring,s);
	}else{
		slen=0;
		thestring=new char[1];
		*thestring=0;
	}
}
mystring::mystring(const char c){
	slen=1;
	thestring=new char[slen+1];
	thestring[0]=c;
	thestring[1]=0;
}

mystring& mystring::operator = ( const mystring & str){
	if (slen!=str.slen){
		delete thestring;
		slen=str.slen;
		thestring=new char[slen+1];
	}
	strcpy(thestring,str.thestring);
	return *this;
}
mystring::mystring(const mystring &copystr){
	slen=copystr.slen;
	thestring=new char[slen+1];
	strcpy(thestring,copystr.thestring);
}
mystring::mystring(const char *format, va_list ap){
	slen=vasprintf(&thestring,format,ap);
}
//mystring::mystring(const char *format, ...){
//	va_list ap;
//	va_start(ap,format);
//	slen=vasprintf(&thestring,format,ap);
//	va_end(ap);
//}
mystring::~mystring(){
	delete thestring;
}

/*char * pathcat(const char * c, const char * n,int dir=0){
   static char buf[FTP_PATH_LEN];
   if (n==NULL){
      strcpy(buf,c);
      return buf;
   }
   else if (n[0]=='/' || n[0]=='~'){
      strcpy(buf,n);
//      return buf;
   } else {
      strcpy(buf,c);
      if (buf[strlen(buf)-1]!='/')
        strcat(buf,"/");
      strcat(buf,n);
   }
   return fixpath(buf,dir);
}*/
mystring& mystring::pathcat(const mystring &copystr){
	if (copystr.slen==0)
	     return *this;
	if (copystr.thestring[0]=='/' || copystr.thestring[0]=='~')
	     return (*this)=copystr;
//	int i=slen+copystr.slen
	mystring temp(*this);
	if (thestring[slen-1]=='/')
	     spf("%s%s",temp.thestring,copystr.thestring);
	else
	     spf("%s/%s",temp.thestring,copystr.thestring);
//#########fixpath()???
	return *this;
}
mystring* mystring::npathcat(const mystring &copystr){
	if (copystr.slen==0)
	     return new mystring(*this);
	if (copystr.thestring[0]=='/' || copystr.thestring[0]=='~')
	     return new mystring (copystr);
//	int i=slen+copystr.slen
//	mystring temp(*this);
	mystring *temp=new mystring;
	if (thestring[slen-1]=='/')
	     temp->spf("%s%s",thestring,copystr.thestring);
	else
	     temp->spf("%s/%s",thestring,copystr.thestring);
//#########fixpath()???
	return temp;
}

void mystring::ins(int pos,const mystring& str){
	if (str.slen<=0) return;
	if (pos>slen) pos=slen;
	else if (pos<0) pos=0;
	char * tmp;
	tmp=new char[slen+str.slen+1];
	memcpy(tmp,thestring,pos);
	tmp[pos]=0;
	strcat(tmp,str.thestring);
	strcat(tmp,&thestring[pos]);
	delete thestring;
	thestring=tmp;
	slen+=str.slen;
}
void mystring::del(int pos, int l){
	if ((l<=0)||(pos>=slen)||(pos<0)) return;
	if (l>slen-pos)l=slen-pos;
	char * tmp;
	tmp=new char[slen-l+1];
	if (pos>0) memcpy(tmp,thestring,pos);
	if (slen-(pos+l)>0) memcpy(&tmp[pos],&thestring[pos+l],slen-(pos+l));
	slen-=l;
	tmp[slen]=0;
	delete thestring;
	thestring=tmp;
}

int mystring::spf(const char *format, ...){
	va_list ap;
	va_start(ap,format);
	vaspf(format,ap);
	va_end(ap);
	return slen;
}
int mystring::vaspf(const char *format, va_list ap){
	if (thestring) delete thestring;
	return (slen=vasprintf(&thestring,format,ap));
}

/*
hashValueType mystring::hashValue() const {
	hashValueType hash=hashValueType(0);
	for (int i=0;i<slen;i++){
		hash ^= thestring[i];
		hash = _rotl(hash,1);
	};
	return hash;
}
*/
