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



#include "../html_analysys.hpp"

namespace{

using namespace std;
using namespace nana::test;



//------------------------------------------
///階層なし場合
TEST_FUNC(test_wildcardMatch1){
	A_EQUALS(nana::wildcardMatch("t*t", "1t2t"), false, "完全マッチする確認");
	A_EQUALS(nana::wildcardMatch("t*t", "t2tee"), false, "完全マッチする確認");
	A_EQUALS(nana::wildcardMatch("t*t", "t244hgfn2t"), true, "完全マッチする確認");
	A_EQUALS(nana::wildcardMatch("t\\*t", "t222t"), false, "エスケープの確認");
	A_EQUALS(nana::wildcardMatch("t\\*t", "t*t"), true, "エスケープの確認");
	A_EQUALS(nana::wildcardMatch("tt*", "ttbmn,123254"), true, "最後にワイルド指定確認");
	A_EQUALS(nana::wildcardMatch("tt\\*", "tt*"), true, "最後にエスケープ確認");
	A_EQUALS(nana::wildcardMatch("tt\\*", "ttdagaga"), false, "最後のエスケープ確認");
	A_EQUALS(nana::wildcardMatch("tt\\", "tt"), true, "最後の\\確認");
	A_EQUALS(nana::wildcardMatch("t?t", "t2t"), true, "ワイルド確認");
	A_EQUALS(nana::wildcardMatch("t?t", "t22t"), false, "ワイルド確認");
	A_EQUALS(nana::wildcardMatch("tt?", "tt"), false, "ワイルド確認");
}


//------------------------------------------

///階層がない場合
TEST_FUNC(test_DocumentHtmlSaxParserHandler1){
	string str(" <!Doctype afdafa><tAg aA='xX'> <!--d/--></tag><tes2/><not gg='");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	const nana::HtmlNode& node = docUptr->rootNode();
	//
	A_EQUALS(node.childNodeList().size(), 3, "タグの数");
	//
	A_TRUE(node.parent() == nullptr, "ルートの親アドレス");
	A_EQUALS(node.pathStr(), "/", "ルートのパス文字列");
	//
	A_EQUALS(node.childNodeList()[0]->childNodeList().size(), 0, "子ノードの数");
	A_EQUALS(node.childNodeList()[0]->parent(), &node, "親");
	A_EQUALS(node.childNodeList()[0]->pathStr(), "/tag", "パス文字列");
	A_EQUALS(node.childNodeList()[0]->tagName(), "tag", "タグ名");
	//
	A_EQUALS(node.childNodeList()[1]->childNodeList().size(), 0, "パス文字列");
	A_EQUALS(node.childNodeList()[1]->parent(), &node, "親");
	A_EQUALS(node.childNodeList()[1]->pathStr(), "/tes2", "パス文字列");
	A_EQUALS(node.childNodeList()[1]->tagName(), "tes2", "タグ名");
	//
	A_EQUALS(node.childNodeList()[2]->childNodeList().size(), 0, "パス文字列");
	A_EQUALS(node.childNodeList()[2]->parent(), &node, "親");
	A_EQUALS(node.childNodeList()[2]->pathStr(), "/[err]", "パス文字列");
	A_EQUALS(node.childNodeList()[2]->tagName(), "[err]", "タグ名");
};


///階層がある場合
TEST_FUNC(test_DocumentHtmlSaxParserHandler2){
	string str("<tag aA='xX'><tag2><div class>aaa</div></tag2><input type=\"hidden\"></tag><tes2/>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	const nana::HtmlNode& node = docUptr->rootNode();
	//
	A_EQUALS(node.childNodeList().size(), 2, "タグの数");
	//
	A_TRUE(node.parent() == nullptr, "ルートの親アドレス");
	A_EQUALS(node.pathStr(), "/", "ルートのパス文字列");
	//
	nana::HtmlNode& tag = *node.childNodeList()[0];
	//
	A_EQUALS(tag.childNodeList().size(), 2, "子ノードの数");
	A_EQUALS(tag.parent(), &node, "親");
	A_EQUALS(tag.pathStr(), "/tag", "パス文字列");
	A_EQUALS(tag.tagName(), "tag", "タグ名");
	//
	A_EQUALS(tag.childNodeList()[1]->pathStr(), "/tag/input", "パス文字列");
	A_EQUALS(tag.childNodeList()[1]->tagName(), "input", "タグ名");
	A_NOT_NULL(tag.childNodeList()[1]->startTag(), "開始タグの存在");

	//
	nana::HtmlNode& tag2 = *tag.childNodeList()[0];
	//
	A_EQUALS(tag2.childNodeList().size(), 1, "子ノードの数");
	A_EQUALS(tag2.parent(), &tag, "親");
	A_EQUALS(tag2.pathStr(), "/tag/tag2", "パス文字列");
	A_EQUALS(tag2.tagName(), "tag2", "タグ名");
	//
	A_EQUALS(tag2.childNodeList()[0]->childNodeList().size(), 0, "子ノードの数");
	A_EQUALS(tag2.childNodeList()[0]->parent(), &tag2, "親");
	A_EQUALS(tag2.childNodeList()[0]->pathStr(), "/tag/tag2/div", "パス文字列");
	A_EQUALS(tag2.childNodeList()[0]->tagName(), "div", "タグ名");
};



///HTMLのタグで正しい場合
TEST_FUNC(test_DocumentHtmlSaxParserHandler3){
	string str("<html>\n<script><!-- function{if(i < 0) return 0;} --></script>\n<div><input type=\"hiddne\"><img ><img src/><a href=''>aaa</a></div></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	const nana::HtmlNode& node = docUptr->rootNode();
	//
	const nana::HtmlNode& htmlNode = *node.childNodeList()[0];
	for(auto i = htmlNode.begin(); i != htmlNode.end(); ++i){
		A_TRUE((*i)->isClosed(), "閉じているか？");
	}

	//
	const nana::HtmlNode& scriptNode = *node.childNodeList()[0]->childNodeList()[0];
	A_EQUALS(scriptNode.tagName(), "script", "タグチェック");
	nana::HtmlDocument::SearchResultsUptr partsVecUptr =  docUptr->range(scriptNode.startTag(), scriptNode.endTag());
	A_EQUALS(partsVecUptr->size(), 3, "partsチェック");
	A_EQUALS((*partsVecUptr)[1]->str(), "<!-- function{if(i < 0) return 0;} -->", "partsチェック");

};


///HTMLのタグで正しくない場合
TEST_FUNC(test_DocumentHtmlSaxParserHandler4){
	string str("<html><div><a href=''>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	const nana::HtmlNode& node = docUptr->rootNode();
	//
	for(auto i = node.begin(); i != node.end(); ++i){
		A_FALSE((*i)->isClosed(), "閉じているか？");
	}

};



//------------------------------------------
///階層なし場合
TEST_FUNC(test_HtmlPath1){
	string str("<html><div class=\"1\"/><br><div class=\"2\"/></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::path::HtmlPathExecutor executor;
	executor.tag("html").tag("div").predPos(0);
	unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec = executor.exec(docUptr->rootNode());
	//
	A_EQUALS((*retExec).size(), 1, "抽出数");
	A_EQUALS((*retExec)[0]->pathStr(), "/html/div", "path()テスト");
	A_EQUALS((*retExec)[0]->startTag()->str(), "<div class=\"1\"/>", "抽出タグの確認");

	//
	nana::path::HtmlPathExecutor executor2;
	executor2.slash2().tag("br");
	unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec2 = executor2.exec(docUptr->rootNode());
	//
	A_EQUALS((*retExec2).size(), 1, "抽出数");
	A_EQUALS((*retExec2)[0]->pathStr(), "/html/br", "path()テスト");
	A_EQUALS((*retExec2)[0]->startTag()->str(), "<br>", "抽出タグの確認");
	
	//述語はtag()の後に使用しないといけない
	try{
		executor2.slash2().predAttr("*", "");
		A_TRUE(false, "例外が発生していない");
	}catch(std::runtime_error& e){
		//正しい
		A_NOT_NULL(e.what(), "文字がある");
	}catch(std::exception){
		A_TRUE(false, "runtime_exceptionが発生しないといけないのに発生していない");
	}
	try{
		executor2.slash2().predPos(0);
		A_TRUE(false, "例外が発生していない");
	} catch(std::runtime_error& e){
		//正しい
		A_NOT_NULL(e.what(), "文字がある");
	} catch(std::exception){
		A_TRUE(false, "runtime_exceptionが発生しないといけないのに発生していない");
	}
};



///階層あり場合
TEST_FUNC(test_HtmlPath2){
	string str("<html><div id='main'><form name='f'><input name='1'></form></div><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::path::HtmlPathExecutor executor;
	executor.slash2().tag("div").predAttr("id", "main").slash2().tag("input");
	unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec = executor.exec(docUptr->rootNode());
	//
	A_EQUALS((*retExec).size(), 1, "抽出数");
	A_EQUALS((*retExec)[0]->pathStr(), "/html/div/form/input", "path()テスト");
	A_EQUALS((*retExec)[0]->startTag()->str(), "<input name='1'>", "抽出タグの確認");

	//パス実行結果にさらにパスの探索をする場合
	executor.clear();
	executor.slash2().tag("div");
	//
	retExec = executor.exec(docUptr->rootNode());
	//
	executor.clear();
	executor.tag("form").tag("input");
	retExec = executor.exec(*retExec);
	//
	A_EQUALS((*retExec).size(), 1, "抽出数");
	A_EQUALS((*retExec)[0]->pathStr(), "/html/div/form/input", "path()テスト");

};




///階層あり。タグが閉じているが入れ違いになっている場合
TEST_FUNC(test_EndTagAccessor1){
	string str("<html><div id='main'><form name='f'><input name='1'></div></form><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::EndTagAccessor acc;
	nana::HtmlNodeVisitor vis;
	//
	vis.access(docUptr->rootNode(), acc);
	//
	auto nonClosedUptr = acc.nonClosedResult();
	auto alternatedUptr = acc.alternatedResult();
	//
	A_EQUALS(nonClosedUptr->size(), 0, "抽出数");
	A_EQUALS(alternatedUptr->size(), 1, "抽出数");
	A_EQUALS((*alternatedUptr)[0]->pathStr(), "/html/div/form//div", "path()テスト");
};


///階層あり。閉じタグがない場合
TEST_FUNC(test_EndTagAccessor2){
	string str("<html><div id='main'><form name='f'><input name='1'></div><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::EndTagAccessor acc;
	nana::EndTagAccessor::SearchResultsUptr alternatedUptr, nonClosedUptr;
	nana::HtmlNodeVisitor vis;
	//
	vis.access(docUptr->rootNode(), acc);
	//
	nonClosedUptr = acc.nonClosedResult();
	alternatedUptr = acc.alternatedResult();
	//
	A_EQUALS(nonClosedUptr->size(), 1, "抽出数");
	A_EQUALS(alternatedUptr->size(), 0, "抽出数");
	A_EQUALS((*nonClosedUptr)[0]->pathStr(), "/html/div/form", "path()テスト");
};


///階層あり。閉じタグがない場合
TEST_FUNC(test_CompositeAccessor1){
	string str("<html><div id='main'><form name='f'><input name='1'></div><blink></blink><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::CompositeAccessor acc;
	acc.add(new nana::EndTagAccessor)
	   .add(new nana::DeprecatedInHtml5Accessor);
	nana::HtmlNodeVisitor vis;
	//
	vis.access(docUptr->rootNode(), acc);
	//
	auto nonClosedUptr = acc.accessor<nana::EndTagAccessor>(0).nonClosedResult();
	auto deprecatedUptr = acc.accessor<nana::DeprecatedInHtml5Accessor>(1).result();
	//
	A_EQUALS(nonClosedUptr->size(), 1, "抽出数");
	A_EQUALS((*nonClosedUptr)[0]->pathStr(), "/html/div/form", "path()テスト");
	A_EQUALS(deprecatedUptr->size(), 1, "抽出数");
	A_EQUALS((*deprecatedUptr)[0]->pathStr(), "/html/div/form/blink", "path()テスト");
};


/**
HtmlSaxParser::copyUntilFindCommentClosedのバグ修正のテスト
コメントが開始した後、閉じタグが2つ続くとコメントの終了を認識できないバグ。
*/
TEST_FUNC(test_copyUntilFindCommentClosed){
	string str("<html><!--<form name='f'><input name='1'><--><blink></blink><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::EndTagAccessor acc;
	nana::HtmlNodeVisitor vis;

	//チェック実行
	vis.access(docUptr->rootNode(), acc);

	//結果取得(結果の型は unique_ptr<vector<const HtmlNode*>>)
	auto nonClosedUptr = acc.nonClosedResult();
	auto alternatedUptr = acc.alternatedResult();
	//
	A_EQUALS(nonClosedUptr->size(), 0, "抽出数");
	A_EQUALS(alternatedUptr->size(), 0, "抽出数");
};


} //namespace
