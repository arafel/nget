/* mystring.h  Matthew Mueller */
#ifndef _MYSTRING_H_
#define _MYSTRING_H_
#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif

//#include <sortable.h>
#include <string.h>
#include <stdarg.h>

class mystring/*:public Sortable*/{
private:
	int slen;
	char * thestring;
public:
	mystring(const char * s="");
	mystring(const char c);
	mystring(const mystring &copystr);
//	mystring(const char *format, ...);//conflicts with the first one :(
	mystring(const char *format, va_list ap);
	~mystring();
      mystring& operator= (const mystring &copystr);
      mystring& operator= (const char *s);
      int operator< (const mystring&s) const;
      int operator== (const mystring&s) const;
      int operator== (const char *s) const;
	char operator [](int i) const {return thestring[i];}
      
	mystring& pathcat(const mystring &copystr);//cat on this string
	mystring* npathcat(const mystring &copystr);//return a new string

	int len(void) const {return slen;}
	int size(void) const {return slen;}
	const char * str(void) const {return thestring;}
	const char * c_str(void) const {return thestring;}
	char * mstr(void) {return thestring;}//don't change the length with this...
	void ins(int pos,const mystring& str);
	void del(int pos, int l);
	int spf(const char *format, ...)
		 __attribute__ ((format (printf, 2, 3)));//arg 1 is "this"
	int vaspf(const char *format, va_list ap);
/*	
	virtual hashValueType hashValue() const;
	virtual classType isA() const {return __firstUserClass;}
	virtual int isEqual(const Object& testObj) const {
		if ((len==((mystring&)testObj).len)&&(strcmp(thestring,((mystring&)testObj).thestring)==0))
			return 1;
		else
			return 0;
	}
	virtual int isLessThan(const Object& testObj) const {
		if (strcmp(thestring,((mystring&)testObj).thestring)<0)
			return 1;
		else
			return 0;
	}
	virtual char *nameOf() const {return "mystring";}
	virtual void printOn( ostream& outputStream) const {
		outputStream << thestring;
	}
 */
};

#endif
