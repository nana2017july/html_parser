#include <iostream>
#include <fstream>
#include <sstream>

#include "html_element.hpp"
#include "html_analysys.hpp"

/* テスト実行用のソースです */


using namespace std;


#include "test/assert.hpp"
#include "test/test_analysys.hpp"
#include "test/test_element.hpp"


int main(int argc, char *argv[]){
	nana::test::Assert::assert.setOutput();
	nana::test::Assert::assert.exec();

	return 0;
}
