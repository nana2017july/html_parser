/*
* Copyright 2017 the original author or authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include <iostream>
#include <sstream>

#include "assert.hpp"


using std::ostream;
using std::cout;
using std::endl;
using std::exception;

using namespace nana::test;

//extern
Assert Assert::assert;



ostream& operator <<(ostream& out, const bool bl){
	out << (bl ? "true" : "false");
	return out;
};


//-------------------------------

Assert::Assert() : m_errCnt(0), m_isOutputTestFile(false)
{ 
	cout << "Assert::Assert() created." << endl;
};
	
	
void Assert::state(const bool result, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(!(result)){
		output(file, line);
		cout << ", state error, " << msg << endl;
	}
};	
void Assert::a_true(const bool result, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(!(result)){
		output(file, line);
		cout << ", expected true error, " << msg << endl;
	}
};
void Assert::a_false(const bool result, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(result){
		output(file, line);
		cout << ", expected false error, " << msg << endl;
	}
};	


///
void Assert::null(void* v1, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(v1 != nullptr){
		output(file, line);
		cout << ", expected: NULL, actual: NOT NULL, " << msg << endl;
	}
};



///
void Assert::notNull(void* v1, const char* msg, const char* file, const int line){
	++m_assertCnt;
	if(v1 == nullptr){
		output(file, line);
		cout << ", expected: NOT NULL, actual: NULL, " << msg << endl;
	}
};




void Assert::output(const char* file, const int line){
	cout << "file=" << file << ", line=" << line ;
	++m_errCnt;
};



void Assert::exec(){
cout << this << ", " << m_vec.size() << endl;
	vector<Test>::iterator i;
	int testCnt = static_cast<int>(m_vec.size());
	int cnt = 0;
	int assertErrCnt = 0;
	
	//
	cout << testCnt << " tests are." << endl;
	
	//テストの実行 
	for(i=m_vec.begin(); i!=m_vec.end(); ++i){
		m_errCnt = 0;
		//
		if(m_isOutputTestFile){
			cout << (*i)->m_file << "(" << (*i)->m_line << ")" << endl;
		}
		
		//実行 
		try{
			(*i)->exec();
		}catch(exception& e){
			cout << "exception has raised in test: " << e.what() <<endl;	
			++m_errCnt;	
		}catch(...){
			cout << "unexpected error in test." << endl;
			++m_errCnt;
		}
		
		//エラー数 
		if(m_errCnt > 0){
			++cnt;
			assertErrCnt += m_errCnt;
		}
	}
	
	//
	cout << endl << "tests are finished.------- " << endl;
	cout << cnt << "/" << testCnt << " test errors. ";
	cout << "of " << assertErrCnt << "/" << m_assertCnt << " assert errors." << endl;
};



/*
test_base::test_base(){
	cout << &(Assert::assert) << ", test_base()" << endl;
	Assert::add(this);
	//if(Assert::assert.testCnt() == 2) Assert::assert.exec();
};
*/


