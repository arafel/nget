#include "file.h"

int main(void){
	c_file_gz f;
	if(!f.open("./test.txt.gz","w")){
		f.putf("aoeuaoeu%s\t%i\n","hi",3);
		f.close();
	}
	return 0;
}
