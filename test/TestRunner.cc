
#include "nrangetest.h"
#include "dupe_file_test.h"

#include <cppunit/TextTestRunner.h>

int main(void){
	TextTestRunner runner;
	runner.addTest(nrangeTest::suite());
	runner.addTest(nrangeEqTest::suite());
	runner.addTest(dupe_file_Test::suite());
	runner.run();

	return 0;
}
