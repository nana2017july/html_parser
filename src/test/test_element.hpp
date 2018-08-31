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



#include "../html_element.hpp"

namespace{

using namespace std;
using namespace nana::test;


TEST_FUNC(test_TagHtmlPart_parseTag){
	{
		nana::TagHtmlPart htmlPart("<input hhh>", 1, 1);
		A_TRUE(htmlPart.hasAttr("hhh", 0), "値なし");
	}
	{
		nana::TagHtmlPart htmlPart("<input hhh=111>", 1, 1);
		A_EQUALS(htmlPart.attr("hhh", 0), "111", "クォートなし");
	}
	{
		nana::TagHtmlPart htmlPart("<input hhh 111>", 1, 1);
		A_TRUE(htmlPart.hasAttr("hhh", 0), "値なし");
		A_TRUE(htmlPart.hasAttr("111", 0), "値なし");
	}
	{
		nana::TagHtmlPart htmlPart("<input hhh 111/>", 1, 1);
		A_TRUE(htmlPart.hasAttr("hhh", 0), "値なし");
		A_TRUE(htmlPart.hasAttr("111", 0), "値なし");
	}
	{
		nana::TagHtmlPart htmlPart("<input hhh  111  >", 1, 1);
		A_TRUE(htmlPart.hasAttr("hhh", 0), "値なし");
		A_TRUE(htmlPart.hasAttr("111", 0), "値なし");
	}
	{
		nana::TagHtmlPart htmlPart("<input>", 1, 1);
		A_EQUALS(htmlPart.attrNames()->size(), 0, "属性なし");
	}


}
	

TEST_FUNC(test_SimpleHtmlSaxParserHandler1){
	string str(" <!Doctype afdafa><tAg aA='xX'> <!--d/--></tag><tes2/><not gg='");

	nana::HtmlSaxParser parser;
	nana::SimpleHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	nana::HtmlDocument doc(handler.result(), nullptr);

	//
	const nana::HtmlDocument::HtmlPartUptrs& partsList = doc.htmlPartList();
	//
	A_EQUALS(partsList.size(), 8, "タグの数");
	//
	A_EQUALS(partsList[0]->str(), " ", "テキスト");
	A_EQUALS(partsList[0]->tagName(), "", "タグ名");
	A_EQUALS(partsList[0]->type(), nana::HtmlPart::TEXT, "タイプ");
	//
	A_EQUALS(partsList[1]->str(), "<!Doctype afdafa>", "記述");
	A_EQUALS(partsList[1]->tagName(), "", "タグ名");
	A_EQUALS(partsList[1]->type(), nana::HtmlPart::DECLARATION, "タイプ");
	//
	A_EQUALS(partsList[2]->str(), "<tAg aA='xX'>", "タグ名");
	A_EQUALS(partsList[2]->tagName(), "tag", "タグ名");
	A_EQUALS(partsList[2]->type(), nana::HtmlPart::TAG, "タイプ");
	A_EQUALS(partsList[2]->attr("aa", 0), "xX", "属性");
	A_EQUALS(partsList[2]->attr("aa", 1), "", "属性");
	A_FALSE(partsList[2]->hasAttr("aa", 0), "属性hasAttr");
	A_FALSE(partsList[2]->hasAttr("aa", 1), "属性hasAttr");
	//
	A_EQUALS(partsList[3]->str(), " ", "テキスト");
	A_EQUALS(partsList[3]->tagName(), "", "タグ名");
	A_EQUALS(partsList[3]->type(), nana::HtmlPart::TEXT, "タイプ");
	//
	A_EQUALS(partsList[4]->str(), "<!--d/-->", "記述");
	A_EQUALS(partsList[4]->tagName(), "", "タグ名");
	A_EQUALS(partsList[4]->type(), nana::HtmlPart::COMMENT, "タイプ");
	//
	A_EQUALS(partsList[5]->str(), "</tag>", "記述");
	A_EQUALS(partsList[5]->tagName(), "/tag", "タグ名");
	A_EQUALS(partsList[5]->type(), nana::HtmlPart::TAG, "タイプ");
	//
	A_EQUALS(partsList[6]->str(), "<tes2/>", "記述");
	A_EQUALS(partsList[6]->tagName(), "tes2", "タグ名");
	A_EQUALS(partsList[6]->type(), nana::HtmlPart::TAG, "タイプ");
	//
	A_EQUALS(partsList[7]->str(), "<not gg='", "記述");
	A_EQUALS(partsList[7]->tagName(), "", "タグ名");
	A_EQUALS(partsList[7]->type(), nana::HtmlPart::NOT_END, "タイプ");
};


///エラーのあるタグ
TEST_FUNC(test_SimpleHtmlSaxParserHandler2){
	string str("<tAg aA='xX' <!--d/-->< tes>< /tes><tes2/ ></tag>aa");

	nana::HtmlSaxParser parser;
	nana::SimpleHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	nana::HtmlDocument doc(handler.result(), nullptr);

	//
	const nana::HtmlDocument::HtmlPartUptrs& partsList = doc.htmlPartList();
	//
	A_EQUALS(partsList[0]->str(), "<tAg aA='xX' <!--d/-->", "テキスト");
	A_EQUALS(partsList[0]->tagName(), "tag", "タグ名");
	A_EQUALS(partsList[0]->type(), nana::HtmlPart::TAG, "タイプ");
	//
	A_EQUALS(partsList[1]->str(), "< tes>< /tes>", "テキスト");
	A_EQUALS(partsList[1]->tagName(), "", "タグ名");
	A_EQUALS(partsList[1]->type(), nana::HtmlPart::TEXT, "タイプ");
	//
	A_EQUALS(partsList[2]->str(), "<tes2/ >", "テキスト");
	A_EQUALS(partsList[2]->tagName(), "tes2", "タグ名");
	A_EQUALS(partsList[2]->type(), nana::HtmlPart::TAG, "タイプ");
	//
	A_EQUALS(partsList[3]->str(), "</tag>", "テキスト");
	A_EQUALS(partsList[3]->tagName(), "/tag", "タグ名");
	A_EQUALS(partsList[3]->type(), nana::HtmlPart::TAG, "タイプ");
}


TEST_FUNC(test_HtmlDocument1){
	string str(" <!Doctype afdafa><tAg aA='xX'> <!--d/--></tag>");

	nana::HtmlSaxParser parser;
	nana::SimpleHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	nana::HtmlDocument doc(handler.result(), nullptr);

	//
	A_EQUALS(doc.size(), 6, "タグの数");
	A_TRUE(&doc.rootNode() == nullptr, "ルートノード");
	A_EQUALS(doc.at(1)->str(), "<!Doctype afdafa>", "at()のテスト");
	A_TRUE(doc.at(6) == nullptr, "at()の最大超えるとnullptrを返すか？");
	
	//rangeのテスト--------------------------------------
	nana::HtmlDocument::SearchResultsUptr partsesUptr = doc.range(doc.at(1), doc.at(3));
	//
	A_EQUALS(partsesUptr->size(), 3, "range()のテスト");
	A_EQUALS(partsesUptr->at(0)->str(), "<!Doctype afdafa>", "range()のテスト");
	A_EQUALS(partsesUptr->at(1)->str(), "<tAg aA='xX'>", "range()のテスト");
	A_EQUALS(partsesUptr->at(2)->str(), " ", "range()のテスト");
};




} //namespace
