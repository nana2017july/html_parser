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
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <cstring>
#include <cstdlib>


#include "html_element.hpp"

namespace nana {

using std::string;
using std::vector;
using std::istream;
using std::unique_ptr;



/** cがHTMLの空白かどうかチェックする。HTML4仕様書の9.Textによると以下が
HTMLで扱うスペースだが、最後が面倒なので除いた。
ASCII space (&#x0020;)
ASCII tab (&#x0009;)
ASCII form feed (&#x000C;)
Zero-width space (&#x200B;)
*/
const bool IsSpace(const char& c){
	static const std::string space(" \t\n");
	return space.find_first_of(c) != std::string::npos;
};






///要素のタイプの文字列表現 
const std::string& HtmlPart::typeStr() const{
	static const std::string textStr("TEXT"), tagStr("TAG"), comStr("COMMENT"),
		docStr("DECLARATION"), hateStr("?");
	switch(this->type()){
	case HtmlPart::TEXT:
		return textStr;
	case HtmlPart::TAG:
		return tagStr;
	case HtmlPart::COMMENT:
		return comStr;
	case HtmlPart::DECLARATION:
		return docStr;
	default:
		return hateStr;
	}
};




//
std::ostream& operator << (std::ostream& p_os, const HtmlPart& p_htmlParts){
	p_os << "[" << p_htmlParts.typeStr() << "(" << p_htmlParts.lineNum() << ")]" << p_htmlParts.str();
	return p_os;
}



const std::string& HtmlNode::tagName()const{
	static const string strNull("[nullptr]"), strErr("[err]");
	if(m_startTagPartsPtr == nullptr && m_endTagPartsPtr == nullptr) return strNull;
	if(m_startTagPartsPtr == nullptr) return m_endTagPartsPtr->tagName();
	if(m_startTagPartsPtr->type() == HtmlPart::NOT_END) return strErr;
	return m_startTagPartsPtr->tagName();
};
//
const std::string HtmlNode::pathStr()const{
	const HtmlNode* nodeP = this->m_parentNodePtr;
	if(nodeP == nullptr) return "/";
	string str(this->tagName());
	for(int i = 0; i < 20; ++i, nodeP = nodeP->m_parentNodePtr){
		if(nodeP->m_parentNodePtr == nullptr) return "/" + str;
		str = nodeP->tagName() + "/" + str;
	}
	return ".../" + str;
};

//内部の開始・終了タグの文字列表現 
const string HtmlNode::tagStr()const{
	string ret("(");
	if(m_startTagPartsPtr != NULL) ret += m_startTagPartsPtr->tagName();
	ret += ",";
	if(m_endTagPartsPtr != NULL) ret += m_endTagPartsPtr->tagName();
	ret += ")";
	return ret;
};


//
std::ostream& operator << (std::ostream& p_os, const HtmlNode& p_htmlNode){
	p_os << (void*)&p_htmlNode;
	p_os << "[" << p_htmlNode.pathStr() << "]" << p_htmlNode.tagStr();
	if(p_htmlNode.startTag() != nullptr) p_os << ":" << *p_htmlNode.startTag();
	else if(p_htmlNode.endTag() != nullptr) p_os << ":" << *p_htmlNode.endTag();
	return p_os;
}


//指定のタグを範囲検索する(start ＜ endでないといけない)
HtmlDocument::SearchResultsUptr HtmlDocument::range(const HtmlPart* p_start, const HtmlPart* p_end)const{
	auto i = m_stockedPartUptrsUptr->begin();
	//開始位置まで進める 
	for(; i != m_stockedPartUptrsUptr->end() && (*i).get() != p_start; ++i){}
	//返却値作成 
	SearchResultsUptr htmlElementVecUPtr(new vector<const HtmlPart*>());
	for(; i != m_stockedPartUptrsUptr->end(); ++i){
		htmlElementVecUPtr->push_back(i->get());
		if(i->get() == p_end) break;
	}
	return move(htmlElementVecUPtr);
};

static void toLowerCaseStr(string& p_str){
	transform(p_str.begin(), p_str.end(), p_str.begin(), ::tolower);
}

const std::string& TagHtmlPart::attr(const std::string& p_key, const std::size_t p_index)const{
	static const string strNull("");
	AttrMap::const_iterator i = m_attrMap.find(p_key);
	if(i != m_attrMap.end()){
		if(p_index < (i->second).size()) return (i->second)[p_index];
		return strNull;
	}
	return strNull;
};

const bool TagHtmlPart::hasAttr(const std::string& p_key, const std::size_t p_index)const{
	auto i = m_attrMap.find(p_key);
	if(i != m_attrMap.end()){
		if(p_index < (i->second).size()) return true;
		return false;
	}
	return false;
};

///属性名の一覧
std::unique_ptr<vector<const std::string*>> TagHtmlPart::attrNames()const{
	unique_ptr<vector<const std::string*>> keyVecUptr(new vector<const std::string*>);
	for(auto i = m_attrMap.begin(); i != m_attrMap.end(); ++i) keyVecUptr->push_back(&(i->first));
	return move(keyVecUptr);

};

void TagHtmlPart::parseTag(){
	//HTML文字列中で今の位置の状態を表す 
	enum Status { TAG, NONE, ATTR_KEY, ATTR_VAL };
	enum Status s = TAG;
	static const string notTag(" \n\t=>/");
	static const string notKey(" \n\t=>/");
	string::const_iterator end = str().end();
	string::const_iterator p;
	string key, val;
	//
	for(string::const_iterator i = str().begin(); i != end; ++i){
		switch(s){
		case TAG:
			//タグ(例："<tag")の3文字目からタグの終わりを検索 
			p = find_first_of(i + 2, end, notTag.begin(), notTag.end());
			m_tagName = string(&(*i) + 1, &(*p) - &(*i) - 1);
			toLowerCaseStr(m_tagName);
			s = NONE;
			i = --p;
			break;
		case NONE:
			//空白ではない文字まで位置を進める 
			i = find_if(i, end, IsNotSpace);
			s = NONE;
			//属性の開始の場合
			if(*i != '>' && *i != '/'){
				s = ATTR_KEY;
				--i;
			}
			break;
		case ATTR_KEY:
			p = i;
			//キー名ではない文字まで位置を進める 
			i = find_first_of(i, end, notKey.begin(), notKey.end());
			//キー名取得
			key = string(p, i);
			toLowerCaseStr(key);
			//空白ではない文字まで位置を進める 
			i = find_if(i, end, IsNotSpace);
			//エンドの場合はキーのみ
			if(i == end || *i == '>' || *i == '/'){
				m_attrMap[key].push_back("");
				s = NONE;
				--i;
				break;
			}
			//イコールがあるかをチェック
			if(i != end && *i == '='){
				s = ATTR_VAL;
				++i;
				//空白ではない文字まで位置を進める 
				i = find_if(i, end, IsNotSpace);
			} else{
				//キーのみの場合
				m_attrMap[key].push_back("");
				s = NONE;
			}
			--i;
			break;
		case ATTR_VAL:
			p = i;
			if(*i == '"'){
				++i;
				//"の位置まで進める
				i = find(i, end, '"');
				val = string(++p, i);
			} else if(*i == '\''){
				++i;
				//'の位置まで進める
				i = find(i, end, '\'');
				val = string(++p, i);
			} else {
				//クォートなし。属性値ではないところまで進める
				i = find_first_of(i, end, notKey.begin(), notKey.end());
				val = string(p, i);
			}
			if(i == end) --i;
			//
			m_attrMap[key].push_back(val);
			//
			s = NONE;
			break;
		}
	}
}



void SimpleHtmlSaxParserHandler::tag(const std::string& p_str, const long p_line, const long p_pos){
	unique_ptr<HtmlPart> ptr;
	//分岐
	if(p_str.size() < 10){
		//文字数が足りないので、doctypeはありえない
		//通常のタグの場合
		ptr.reset(new TagHtmlPart(p_str, p_line, p_pos));
	} else{
		char lowerStr[11];
		transform(p_str.begin(), p_str.begin() + 9, lowerStr, ::tolower);
		if(p_str[1] == '?'){
			//？で始まる場合
			ptr.reset(new DeclarationHtmlPart(p_str, p_line, p_pos));
		}else if(strncmp(lowerStr, "<!doctype", 9) == 0 && IsSpace(p_str[9])){
			//doctype[空白]の場合
			ptr.reset(new DeclarationHtmlPart(p_str, p_line, p_pos));
		} else{
			//通常のタグの場合
			ptr.reset(new TagHtmlPart(p_str, p_line, p_pos));
		}
	}
	m_resultPartsUptrsUptr->push_back(move(ptr));
};




//HtmlSaxParser-----------------------------------------------

const bool HtmlSaxParser::copyUntilFind(string& p_str, istream& p_is, const char p_targetC){
	char c;
	while(p_is.get(c)){
		++m_pos;
		if(c == '\n') ++m_line;
		//
		p_str += c;
		if(c == p_targetC) return true;;
	}
	return false;
}

const bool HtmlSaxParser::copyUntilFindCommentClosed(string& p_str, istream& p_is){
	while(true){
		copyUntilFind(p_str, p_is, '>');
		if(strncmp(p_str.data() + p_str.size() - 3, "-->", 3) == 0) return true;
		if(!p_is) return false;
	}
	return false;
}


void HtmlSaxParser::parse(std::istream& p_is, HtmlSaxParserHandler& p_handler){
	string str("");
	char c_c2[] = {'\0', '\0', '\0'};//cとc2を連結した文字列
	char &c = c_c2[0]; //c_c2の1文字目と結びつける
	char &c2 = c_c2[1]; //c_c2の2文字目と結びつける
	long line = 1, pos = 0; //str文字列保管開始位置での改行の数
	m_line = 1; //カレント位置での改行の数
	m_pos = 0;

	//開始
	p_handler.start();

	//ループ 
	while(p_is.get(c)){
		++m_pos;
		if(c == '\n') ++m_line;
		//
		switch(c){
		case '<':
			if(!p_is.get(c2)){
				//ストリームの終わり
				str += c;
				break;
			}
			if(IsSpace(c2)){
				//c2が空白の場合("< ")、タグではないので次に行く 
				str += c_c2;
				continue;
			}
			//溜めた文字列をハンドラに渡す
			if(!str.empty()){
				p_handler.text(str, line, pos);
				clearStr(str, line, pos);
			}

			//新たな開始文字を設定 
			str = c_c2;

			//タグの終わりまでコピー 
			if(!copyUntilFind(str, p_is, '>')){
				//タグの終了（＞）が見つからない
				p_handler.notEnd(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			if(str.size() < 6){
				//tagで登録。文字数が少なすぎてコメントタグはありえない
				p_handler.tag(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			if(strncmp(str.data(), "<!--", 4) != 0){
				//＜で始まっているがコメントではないので、タグと判断
				p_handler.tag(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			//コメントの場合。p_strの終わりがコメントの終わりかチェック
			if(strncmp(str.data() + str.size() - 3, "-->", 3) == 0){
				//コメントの終わりが存在する場合
				p_handler.comment(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			//コメントの終わりが見つからないので見つかるまで探す 
			if(copyUntilFindCommentClosed(str, p_is)){
				//コメントの終わりが見つかった場合
				p_handler.comment(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			//コメントの終了が見つからない場合
			p_handler.notEnd(str, line, pos);
			clearStr(str, line, pos);
			continue;

			break;
		default:
			str += c;
			break;
		}
	}
	//ハンドラに渡す
	if(!str.empty()){
		p_handler.text(str, line, pos);
	}
};




} //namespace nana

