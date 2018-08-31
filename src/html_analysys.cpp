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
#include <iostream>
#include <stdexcept>


#include "html_analysys.hpp"

namespace nana {

using std::string;
using std::map;
using std::multimap;
using std::vector;
using std::istream;
using std::unique_ptr;
using std::cout;
using std::endl;



//実験--------------------------------------------



const bool forwardMatch(const string& p_targetStr, const string& p_searchStr){
	if(p_targetStr.size() > p_searchStr.size()) return false;
	if(p_targetStr.compare(p_searchStr) == 0) return true;
	return false;
}

const bool backwardMatch(const string& p_targetStr, const string& p_searchStr){
	if(p_targetStr.size() > p_searchStr.size()) return false;
	if(p_targetStr.find(p_searchStr, p_targetStr.size() - p_searchStr.size()) != std::string::npos) return true;
	return false;
}
const bool wildcardMatch(const char *ptn, const char *str){
	switch(*ptn){
	case '\0':
		return '\0' == *str;
	case '*':
		return wildcardMatch(ptn + 1, str)
			|| (('\0' != *str) && wildcardMatch(ptn, str + 1));
	case '?':
		return ('\0' != *str)
			&& wildcardMatch(ptn + 1, str + 1);
	default:
		if(*ptn == '\\') ++ptn;
		if(*ptn == '\0' && *str != '\0') return false;
		if(*ptn == '\0' && *str == '\0') return true;
		return ((unsigned char)*ptn == (unsigned char)*str)
			&& wildcardMatch(ptn + 1, str + 1);
	}
}

namespace{

///閉じてはいけないタグ
static const map<string, int> g_mapNotClosed = {
	{"<!--", 0}, {"<!doctype", 0}, {"br", 0}, {"img", 0}, {"hr", 0}, {"meta", 0},
	{"input", 0}, {"embed", 0}, {"area", 0}, {"base", 0}, {"col", 0},
	{"keygen", 0}, {"link", 0}, {"param", 0}, {"source", 0}
};

/**
@return 終了タグ検索。見つからなかった場合はNULL。p_curNodeにタグ情報を溜める。
@note input,imgなどの終了タグがなくてよいタグも考慮している
*/
const HtmlPart* _htmlNodeAnalyze(HtmlNode& p_curNode, const string& startTagName,
	HtmlDocument::const_iterator& i, HtmlDocument::const_iterator end){
	string curCloseTagName("");
	if(!startTagName.empty()) curCloseTagName = "/" + startTagName;

	//
	for(; i != end; ++i){
		const HtmlPart& ele = **i;
		if(ele.type() == HtmlPart::NOT_END){
			//タグの終了がない場合
			p_curNode.appendChild(unique_ptr<HtmlNode>(new HtmlNode(&ele, NULL, &p_curNode)));
			continue;
		}
		if(ele.type() != HtmlPart::TAG)continue;
		if(ele.str()[ele.str().size() - 2] == '/'){
			//単一閉じタグの場合
			p_curNode.appendChild(unique_ptr<HtmlNode>(new HtmlNode(&ele, &ele, &p_curNode)));
			continue;
		}
		if(g_mapNotClosed.find(ele.tagName()) != g_mapNotClosed.end()){
			//閉じなくていいタグ（inputなど）の場合
			p_curNode.appendChild(unique_ptr<HtmlNode>(new HtmlNode(&ele, &ele, &p_curNode)));
			continue;
		}
		if(ele.tagName()[0] == '/'){
			//閉じタグが見つかった
			if(ele.tagName() == curCloseTagName){
				//目的（探していた）の閉じタグ 
				return &ele;
			} else{
				//目的以外の閉じタグ
				p_curNode.appendChild(unique_ptr<HtmlNode>(new HtmlNode(nullptr, &ele, &p_curNode)));
				continue;
			}
		}
		//開始タグの場合。1つ下の階層を走査 
		unique_ptr<HtmlNode> childNodeUptr(new HtmlNode(&ele, nullptr, &p_curNode));
		const HtmlPart* endTag = _htmlNodeAnalyze(*childNodeUptr, ele.tagName(), ++i, end);
		childNodeUptr->setEndTag(endTag);
		p_curNode.appendChild(move(childNodeUptr));

		//最後まで行っていなければ次へ。最後なら終了 
		if(i != end) continue;
		return nullptr;
	}
	return nullptr;
}


void _htmlNodeAnalyzeBySameTagMatch(HtmlNode& p_curNode, const string& startTagName,
	HtmlDocument::const_iterator& i, HtmlDocument::const_iterator end, multimap<string, HtmlNode*>& p_stockTagNameMap)
{
	string curCloseTagName("");
	if(!startTagName.empty()) curCloseTagName = "/" + startTagName;

	//
	for(; i != end; ++i){
		const HtmlPart& ele = **i;
		if(ele.type() == HtmlPart::NOT_END){
			//タグの終了がない場合
			p_curNode.appendChild(unique_ptr<HtmlNode>(new HtmlNode(&ele, NULL, &p_curNode)));
			continue;
		}
		if(ele.type() != HtmlPart::TAG)continue;
		if(ele.str()[ele.str().size() - 2] == '/'){
			//単一閉じタグの場合
			p_curNode.appendChild(unique_ptr<HtmlNode>(new HtmlNode(&ele, &ele, &p_curNode)));
			continue;
		}
		if(g_mapNotClosed.find(ele.tagName()) != g_mapNotClosed.end()){
			//閉じなくていいタグ（inputなど）の場合
			p_curNode.appendChild(unique_ptr<HtmlNode>(new HtmlNode(&ele, &ele, &p_curNode)));
			continue;
		}
		if(ele.tagName()[0] == '/'){
			const char* tagName = ele.tagName().c_str() + 1;
			if(p_stockTagNameMap.find(tagName) != p_stockTagNameMap.end()){
				//開始タグがある場合
				auto ite = p_stockTagNameMap.equal_range(tagName).second;
				--ite;
				HtmlNode& startNode = *(ite->second);
				p_stockTagNameMap.erase(ite);
				startNode.setEndTag(&ele);
				if(startTagName == tagName) return;
			}else{
				//開始タグが見つからない場合
				p_curNode.appendChild(unique_ptr<HtmlNode>(new HtmlNode(nullptr, &ele, &p_curNode)));
			}
			continue;
		}
		//開始タグの場合。1つ下の階層を走査 
		unique_ptr<HtmlNode> childNodeUptr(new HtmlNode(&ele, nullptr, &p_curNode));
		p_stockTagNameMap.insert(std::make_pair(ele.tagName(), childNodeUptr.get()));
		//配下の操作。再起呼び出し
		_htmlNodeAnalyzeBySameTagMatch(*childNodeUptr, ele.tagName(), ++i, end, p_stockTagNameMap);
		p_curNode.appendChild(move(childNodeUptr));

		//最後まで行っていなければ次へ。最後なら終了 
		if(i != end) continue;
		return;
	}
	return;
} //end _htmlNodeAnalyzeBySameTagMatch(){

}//namespace{ 





//HTMLタグのタグ解析をする（再起）。
void analyzeHtmlNode(HtmlNode& p_ret, const HtmlDocument::HtmlPartUptrs& p_allDocParts){
	HtmlDocument::const_iterator i = p_allDocParts.begin();
	_htmlNodeAnalyze(p_ret, (*i)->tagName(), i, p_allDocParts.end());
}

//HTMLタグのタグ解析をする（同じタグ名どうしで出現順に階層無視してマッチさせる手法）。
void analyzeHtmlNodeBySameTagMatch(HtmlNode& p_ret, const HtmlDocument::HtmlPartUptrs& p_allDocParts){
	HtmlDocument::const_iterator i = p_allDocParts.begin();
	multimap<string, HtmlNode*> map;
	_htmlNodeAnalyzeBySameTagMatch(p_ret, (*i)->tagName(), i, p_allDocParts.end(), map);
}

//HTMLを解析し、タグ配列を返すHTMLパーサハンドラ。
unique_ptr<HtmlDocument> DocumentHtmlSaxParserHandler::result(){
	unique_ptr<HtmlDocument::HtmlPartUptrs> htmlPartsUptrVecUptr = SimpleHtmlSaxParserHandler::result();
	unique_ptr<HtmlNode> rootNodeUptr(new HtmlNode(nullptr, nullptr, nullptr));
	analyzeHtmlNode(*rootNodeUptr, *htmlPartsUptrVecUptr);
	return unique_ptr<HtmlDocument>(new HtmlDocument(move(htmlPartsUptrVecUptr), move(rootNodeUptr)));
};



void EndTagAccessor::access(const HtmlNode& p_node){
	if(p_node.isClosed()) return;
	if(p_node.startTag() == nullptr){
		const char* tagName = p_node.tagName().c_str() + 1;
		//終了タグのみがある場合
		if(m_stockMap.find(tagName) == m_stockMap.end()){
			m_nonClosedResult->push_back(&p_node);
		} else if(m_stockMap[tagName].empty()){
			m_nonClosedResult->push_back(&p_node);
		} else{
			//対になる開始タグがある場合
			//const HtmlNode& node = *m_stockMap[tagName].back();
			m_stockMap[tagName].pop_back();
			//親が開始・終了タグがある場合、NG結果に追加
			if(p_node.parent() == nullptr){
				m_alternatedResult->push_back(&p_node);
			} else if(p_node.parent()->isClosed()){
				m_alternatedResult->push_back(&p_node);
			}
		}
	} else if(p_node.startTag()->type() == HtmlPart::NOT_END){
		//タグのエラー
		m_nonClosedResult->push_back(&p_node);
	} else{
		//開始タグのみがある場合
		m_stockMap[p_node.tagName()].push_back(&p_node);
	}
};

EndTagAccessor::SearchResultsUptr EndTagAccessor::nonClosedResult(){
	for(auto i = m_stockMap.begin(); i != m_stockMap.end(); ++i){
		vector<const HtmlNode*>& vec = i->second;
		if(vec.empty()) continue;
		m_nonClosedResult->insert(m_nonClosedResult->end(), vec.begin(), vec.end());
	}
	return move(m_nonClosedResult);
};

const std::map<std::string, int> DeprecatedInHtml5Accessor::s_deprecatedTagMap({{"center", 0}, {"font", 0}, {"blink", 0}, {"strike", 0}, {"s", 0}, {"u", 0}, {"bgsound", 0}, {"marquee", 0}, {"applet", 0}, {"acronym", 0}, {"dir", 0},
{"frame", 0}, {"frameset", 0}, {"noframes", 0}, {"isindex", 0}, {"listing", 0}, {"xmp", 0}, {"noembed", 0}, {"plaintext", 0}, {"rb", 0}, {"basefont", 0}, {"big", 0}, {"spacer", 0}, {"tt", 0}});


//------------------------------
namespace path{

//直下のパス（1階層/tag）
unique_ptr<HtmlPath::HtmlNodePtrs> PathHtmlPath::filter(HtmlNodePtrs& p_nodePtrs){
	unique_ptr<HtmlNodePtrs> resultNodeVecUptr(new HtmlNodePtrs);
	for(auto i = p_nodePtrs.begin(); i != p_nodePtrs.end(); ++i){
		const HtmlNode& node = **i;
		vector<const HtmlNode*> matchedChildrenNodeList;
		for(auto j = node.begin(); j != node.end(); ++j){
			if(wildcardMatch(m_tagName.c_str(), (*j)->tagName().c_str())){
				matchedChildrenNodeList.push_back(j->get());
			}
		}
		//属性等のフィルタをかける
		match(*resultNodeVecUptr, matchedChildrenNodeList);
	}
	return move(resultNodeVecUptr);
};

void PathHtmlPath::match(HtmlNodePtrs& p_retNodePtrs, const vector<const HtmlNode*>& p_matchedChildrenNodeList)const{
	unique_ptr<vector<const HtmlNode*>> resultNodeVecUptr(new vector<const HtmlNode*>());
	*resultNodeVecUptr = p_matchedChildrenNodeList;
	for(auto i = m_htmlPathUptrList.begin(); i != m_htmlPathUptrList.end(); ++i){
		resultNodeVecUptr = (*i)->filter(*resultNodeVecUptr);
	}
	//フィルタ結果を追加する
	p_retNodePtrs.insert(p_retNodePtrs.end(), resultNodeVecUptr->begin(), resultNodeVecUptr->end());
}


//要素番号の指定。tagname[n]
unique_ptr<HtmlPath::HtmlNodePtrs> PositionHtmlPath::filter(HtmlNodePtrs& p_nodePtrs){
	unique_ptr<HtmlNodePtrs> resultNodeVecUptr(new HtmlNodePtrs);
	if(p_nodePtrs.size() > m_position){
		resultNodeVecUptr->push_back(p_nodePtrs[m_position]);
	}
	return move(resultNodeVecUptr);
};



//属性指定
unique_ptr<HtmlPath::HtmlNodePtrs> AttributesHtmlPath::filter(HtmlNodePtrs& p_nodePtrs){
	unique_ptr<HtmlNodePtrs> resultNodeVecUptr(new HtmlNodePtrs);
	for(auto i = p_nodePtrs.begin(); i != p_nodePtrs.end(); ++i){
		const HtmlPart* startTagPartsP = (*i)->startTag();
		if(startTagPartsP == nullptr) continue;
		unique_ptr<vector<const string*>> keyPVecUptr = startTagPartsP->attrNames();
		//キーの中に指定のキー名と値が存在するかをチェック
		for(auto j = keyPVecUptr->begin(); j != keyPVecUptr->end(); ++j){
			if(!m_attrName.empty()){
				if(!wildcardMatch(m_attrName.c_str(), (*j)->c_str())) continue;
			}
			if(!m_attrVal.empty()){
				const string& attrVal = startTagPartsP->attr((**j), 0);
				if(!wildcardMatch(m_attrVal.c_str(), attrVal.c_str())) continue;
			}
			//指定のキー名、値がマッチした場合
			resultNodeVecUptr->push_back(*i);
			break;
		}
	}
	return move(resultNodeVecUptr);
};


//子孫のタグ（//Descendant or self）検索
unique_ptr<HtmlPath::HtmlNodePtrs> DescendantsHtmlPath::filter(HtmlNodePtrs& p_nodePtrs){
	std::map<const HtmlNode*, int> alreadyRegNodeMap;
	unique_ptr<HtmlNodePtrs> vecUptr(new HtmlNodePtrs);
	_filter(*vecUptr, p_nodePtrs, alreadyRegNodeMap);
	return move(vecUptr);
};

//自身のノードを含めて配下のノードをすべてp_retNodePtrsに設定する
void DescendantsHtmlPath::_filter(HtmlNodePtrs& p_retNodePtrs, const HtmlNodePtrs& p_nodePtrs, std::map<const HtmlNode*, int>& p_alreadyRegNodeMap){
	unique_ptr<HtmlNodePtrs> resultNodeVecUptr(new HtmlNodePtrs);
	for(auto i = p_nodePtrs.begin(); i != p_nodePtrs.end(); ++i){
		const HtmlNode& curNode = **i;
		//自身を登録
		if(p_alreadyRegNodeMap.find(&curNode) == p_alreadyRegNodeMap.end()){
			p_retNodePtrs.push_back(&curNode);
			p_alreadyRegNodeMap[&curNode] = 1;
		}

		//自身が一番下のタグの場合、次のノードへ
		if(curNode.childNodeList().size() == 0) continue;

		//子供の配列を作り、filterにかける
		unique_ptr<HtmlNodePtrs> childNodeVecUptr(new HtmlNodePtrs);
		for(auto j = curNode.begin(); j != curNode.end(); ++j){
			const HtmlNode& childNode = **j;
			childNodeVecUptr->push_back(&childNode);
		}
		_filter(p_retNodePtrs, *childNodeVecUptr, p_alreadyRegNodeMap);
	}
};



//------------------------------------
//Htmlパスの実行者
unique_ptr<HtmlPath::HtmlNodePtrs> HtmlPathExecutor::exec(const HtmlNode& p_node)const{
	unique_ptr<HtmlNodePtrs> resultNodePtrsUptr(new HtmlNodePtrs);
	resultNodePtrsUptr->push_back(&p_node);
	for(auto i = m_htmlPathVec.begin(); i != m_htmlPathVec.end(); ++i){
		resultNodePtrsUptr = (*i)->filter(*resultNodePtrsUptr);
	}
	return move(resultNodePtrsUptr);
}; 

unique_ptr<HtmlPath::HtmlNodePtrs> HtmlPathExecutor::exec(const HtmlNodePtrs& p_nodePtrs)const{
	unique_ptr<HtmlNodePtrs> resultNodePtrsUptr(new HtmlNodePtrs);
	resultNodePtrsUptr->insert(resultNodePtrsUptr->end(), p_nodePtrs.begin(), p_nodePtrs.end());
	for(auto i = m_htmlPathVec.begin(); i != m_htmlPathVec.end(); ++i){
		resultNodePtrsUptr = (*i)->filter(*resultNodePtrsUptr);
	}
	return move(resultNodePtrsUptr);
};

//自分でパスを自由に追加する
HtmlPathExecutor& HtmlPathExecutor::add(HtmlPath* p_path){
	m_htmlPathVec.push_back(unique_ptr<HtmlPath>(p_path));
	m_isTag = false;
	return *this;
};

//タグ名の指定
HtmlPathExecutor& HtmlPathExecutor::tag(const string& p_tagName){
	add(new PathHtmlPath(p_tagName));
	m_isTag = true;
	return *this;
};

//述語：属性の指定(attribute)
HtmlPathExecutor& HtmlPathExecutor::predAttr(const string& p_tagName, const string& p_val){
	if(!m_isTag){
		cout << __FILE__ << ":(" << __LINE__ << "), error, predAttr() is used, but previous element is not tag()!!" << endl;
		throw std::runtime_error("attr() is specified, but previous element is not tag()!!");
	}
	(dynamic_cast<PathHtmlPath*>((m_htmlPathVec.rbegin())->get()))->add(new AttributesHtmlPath(p_tagName, p_val));
	return *this;
}

//述語：要素番号の指定（position）
HtmlPathExecutor& HtmlPathExecutor::predPos(const int p_index){
	if(!m_isTag){ 
		cout << __FILE__ << ":(" << __LINE__ << "), error, predPos() is used, but previous element is not tag()!!" << endl;
		throw std::runtime_error("pos() is specified, but previous element is not tag()!!");
	}
	(dynamic_cast<PathHtmlPath*>((m_htmlPathVec.rbegin())->get()))->add(new PositionHtmlPath(p_index));
	return *this;
}

}//namespace path






}//namespace nana