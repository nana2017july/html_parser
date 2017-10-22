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
#ifndef __NANA_ASSERT_H__
#define __NANA_ASSERT_H__


#include <cstdlib>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>


namespace nana{
namespace test{

/*
なぜか、このhpp,cppファイルを最後に追加しないとうまく
テストのaddができない。
なんでだろ？
MakeileのOBJ、LINKOBJの記述で一番最後にassert.cppを入れるようにすること
*/

using std::vector;
using std::string;
using std::cout;
using std::endl;

class test_base;



class Assert{
public:
	typedef test_base* Test;

	///
	static Assert assert;

	///
	Assert();

	void state(const bool result, const char* msg, const char* file, const int line);
	void a_true(const bool result, const char* msg, const char* file, const int line);
	void a_false(const bool result, const char* msg, const char* file, const int line);

	///
	void null(void* v1, const char* msg,
		const char* file, const int line);

	///
	void notNull(void* v1, const char* msg,
		const char* file, const int line);

	///
	static void add(Test test){
		assert.m_vec.push_back(test);
	};

	///
	const int testCnt() const{
		return static_cast<int>(m_vec.size());
	};

	///
	void addAssertCnt(){
		++m_assertCnt;
	};

	///テスト実行時にファイル名を出力するか 
	void setOutput(){
		m_isOutputTestFile = true;
	};

	//
	void output(const char* file, const int line);

	///
	void exec();

protected:

private:
	int m_errCnt;
	int m_assertCnt;
	vector<Test> m_vec;
	bool m_isOutputTestFile;
};




/**
asserEqualsで使用するサブ関数。
様々な型(char*など)に対応できるようにクラス化する。
*/
template <typename T, typename U>
struct AssertEqualsCheck{
public:
	bool equals;
	AssertEqualsCheck(const T&a, const U& e){
		equals = (a == e);
	};
};




/**
==演算子でテストする。
*/
template <typename T, typename U>
void asserEquals(const T& a, const U& e, const char* msg, const bool isOutput,
	const char* file, const int line){
	Assert::assert.addAssertCnt();
	AssertEqualsCheck<T, U> c(a, e);
	if(!c.equals){
		Assert::assert.output(file, line);
		if(isOutput) std::cout << ", expected: " << e << ", actual: " << a;
		cout << ", " << msg << endl;
	}
};




//extern Assert assert;




///テストの定義
class test_base{
public:
	test_base(const char* file, const int line)
		: m_file(file), m_line(line){
		Assert::add(this);
	};
	virtual ~test_base(){};
	virtual void exec(){};
	const string m_file;
	const int m_line;
};

#define TEST_FUNC(test) class test : public test_base{ \
public:\
	test(const char* file, const int line) : test_base(file, line) {}; \
	virtual void exec(); \
	static test jittai; \
}; \
test test::jittai(__FILE__, __LINE__); \
void test::exec()



#define A_EQUALS(actual, expected, msg) asserEquals(actual, expected, msg, true, __FILE__, __LINE__);
#define A_STATE(state1, msg) Assert::assert.state(state1, msg, __FILE__, __LINE__);
#define A_TRUE(state1, msg) Assert::assert.a_true(state1, msg, __FILE__, __LINE__);
#define A_FALSE(state1, msg) Assert::assert.a_false(state1, msg, __FILE__, __LINE__);
#define A_NOT_NULL(value, msg) Assert::assert.notNull((void*)value, msg, __FILE__, __LINE__);
#define A_NULL(value, msg) Assert::assert.null((void*)value, msg, __FILE__, __LINE__);


}//namespace test
}//namespace nana


#endif	//__NANA_ASSERT_H__


