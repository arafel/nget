#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif
#include "file.h"
#include <string>

int main(void){
//	c_file_gz f;
//	if(!f.open("./test.txt.gz","w")){
//		f.putf("aoeuaoeu%s\t%i\n","hi",3);
//		f.close();
//	}
	string s="aoeuaoeu";
	printf("%s\n",s.c_str());
	s.erase(s.size()-1,s.size());
	printf("%s\n",s.c_str());
	return 0;
}
