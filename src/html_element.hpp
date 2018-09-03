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
#ifndef NANA_HTML_DOCUMENT_INCLUDED
#define NANA_HTML_DOCUMENT_INCLUDED


#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <memory>
#include <ostream>
#include <istream>
#include <cstring>
#include <cstdlib>



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
const bool IsSpace(const char& c);

inline const bool IsNotSpace(const char& c){
	return !IsSpace(c);
};


///コピーさせないために
class noncopyable{
protected:
	noncopyable(){};
	~noncopyable(){};
private:  // emphasize the following members are private
	noncopyable(const noncopyable&);
	noncopyable& operator=(const noncopyable&){ return *this; };
};



/**
@breif <pre>
HTMLの構成の１つを表す。構成とは以下のように定義する（一般的ではないことに注意）。
・開始タグ
・終了タグ
・テキスト
・コメント
・宣言（!doctype、?xml）
</pre>*/
class HtmlPart : noncopyable{
public:
	enum Type { TEXT, TAG, COMMENT, /**宣言（!doctype、?xmlなど）*/DECLARATION, /**タグの右側の＞がない*/NOT_END };
	HtmlPart(const std::string& p_str, const long p_line, const long p_pos)
		:m_contentStr(p_str), m_line(p_line), m_pos(p_pos) {};
	///
	virtual ~HtmlPart(){};
	///要素のタイプ（テキスト、タグなど） 
	virtual const Type type() const = 0;
	///要素のタイプの文字列表現 
	virtual const std::string& typeStr() const;
	///タグ名（タイプがタグでない場合は空文字を返す） 
	virtual const string& tagName() const{
		static const string emp("");
		return emp;
	};
	///位置（行数）
	virtual const long lineNum()const{ return m_line; };
	///位置（先頭からのバイト数）
	virtual const long posNum()const{ return m_pos; };
	///タグ全体の文字列 
	virtual const std::string& str() const{ return m_contentStr; };
	///属性を取得する（見つからない場合、タイプがタグでない場合は空文字を返す） 
	virtual const std::string& attr(const std::string& p_key, const std::size_t p_index) const{
		static const std::string strNull("");
		return strNull;
	};
	///属性が存在するか 
	virtual const bool hasAttr(const string& p_key, const int& p_index) const{
		return false;
	};
	///属性名の一覧
	virtual std::unique_ptr<vector<const string*>> attrNames()const
	{ return unique_ptr<vector<const string*>>(new vector<const string*>); };
private:
	const std::string m_contentStr;
	const long m_line;
	const long m_pos;
};

//
std::ostream& operator << (std::ostream& os, const HtmlPart& htmlParts);


/**
@breif <pre>
HTMLのタグ要素を扱うクラス（一般的にはNodeはテキストなども扱うがここでは扱わない事に注意）。
内部に保持するタグ（ HtmlPart ）は他のクラスが実体を持ち、寿命を管理するので
このクラス内ではポインタだけ保持し、解放等も行わない。
このクラス内に保持する自身のクラスの実体は自身で管理する。
</pre>*/
class HtmlNode : noncopyable{
public:
	typedef std::vector<std::unique_ptr<HtmlNode>> NodeUptrs;
	typedef NodeUptrs::const_iterator const_iteraotr;
	HtmlNode()
		: m_startTagPartsPtr(nullptr), m_endTagPartsPtr(nullptr), m_parentNodePtr(nullptr)
	{ };
	//
	HtmlNode(const HtmlPart* p_Start, const HtmlPart* p_End, const HtmlNode* p_Parent)
	: m_startTagPartsPtr(p_Start), m_endTagPartsPtr(p_End), m_parentNodePtr(p_Parent){};
	///
	virtual ~HtmlNode(){};
	///開始タグ（存在しない場合、nullptr）。
	const HtmlPart* startTag()const{ return m_startTagPartsPtr; };
	///終了タグ（存在しない場合、nullptr）
	const HtmlPart* endTag()const{ return m_endTagPartsPtr; };
	///パーサハンドラ以外は使用禁止。引数はクラス内部で廃棄の管理をしない。
	void setEndTag(const HtmlPart* p_endTag){ m_endTagPartsPtr = p_endTag; };
	void appendChild(std::unique_ptr<HtmlNode>&& p_Child){
		p_Child->m_parentNodePtr = this;
		m_childNodeUptrs.push_back(move(p_Child));
	};
	///開始タグと終了タグがセットで存在するか（タグが閉じているか？）。
	const bool isClosed()const{ return (m_startTagPartsPtr != nullptr && m_endTagPartsPtr != nullptr); };
	///子ノード（ childNodeList() ）の開始位置イテレータ
	const_iteraotr begin() const{ return m_childNodeUptrs.begin(); };
	///子ノード（ childNodeList() ）の最後の次の位置のイテレータ
	const_iteraotr end() const{ return m_childNodeUptrs.end(); };
	///子ノード
	const NodeUptrs& childNodeList() const{ return m_childNodeUptrs; };
	///親ノードのポインタ。存在しない場合、nullptr。
	const HtmlNode* parent() const{ return m_parentNodePtr; };
	///タグ名
	const std::string& tagName()const;
	///パス
	const std::string pathStr()const;
	///内部保持している開始と終了タグを文字列出力（デバッグ用） 
	const string tagStr()const;
private:
	const HtmlPart* m_startTagPartsPtr;
	const HtmlPart* m_endTagPartsPtr;
	const HtmlNode* m_parentNodePtr;
	///子ノード。
	NodeUptrs m_childNodeUptrs;
};


//
std::ostream& operator << (std::ostream& os, const HtmlNode& htmlNode);


/**
@breif <pre>
タグの解析をした結果を保存するクラス。
解析は以下のことしか行わない。それ以上の解析は他のクラスや関数で行う。
・コメント（＜！--）
・declaration（＜！doctype、＜？）
・タグ（＜yyy ＞＜/xxx＞＜zzz/＞それぞれ別々に扱う）
・テキスト（上記以外のもの）
</pre>
*/
class HtmlDocument: noncopyable{
public:
	typedef std::vector<unique_ptr<HtmlPart> > HtmlPartUptrs;
	typedef HtmlPartUptrs::const_iterator const_iterator;
	typedef vector<const HtmlPart*> SearchResults;
	typedef unique_ptr<SearchResults> SearchResultsUptr;
	//
	HtmlDocument(unique_ptr<HtmlPartUptrs>&& p_partUptr, unique_ptr<HtmlNode>&& p_rootNode)
	: m_stockedPartUptrsUptr(move(p_partUptr)), m_rootNodeUptr(move(p_rootNode)) {};
	///htmlPartList() のイテレータ( unique_ptr<HtmlPart> )
	const_iterator begin() const{ return m_stockedPartUptrsUptr->begin(); };
	const_iterator end() const{ return m_stockedPartUptrsUptr->end(); };

	///HTMLのタグのリスト全て
	const HtmlPartUptrs& htmlPartList()const{ return *m_stockedPartUptrsUptr; };
	///ルートノード
	const HtmlNode& rootNode()const{ return *m_rootNodeUptr; };
	
	///要素の数
	const std::size_t size()const{ return m_stockedPartUptrsUptr->size(); };
	///位置を指定してタグを取得する(見つからない場合nullptr)
	const HtmlPart* at(const std::size_t p_index)const{
		if(p_index < m_stockedPartUptrsUptr->size()) return (*m_stockedPartUptrsUptr)[p_index].get();
		return nullptr;
	};
	/**指定のタグを範囲検索する(start ＜ endでないといけない)
	@param start [in]範囲の開始。このポインタと同じポインタを開始位置とする。
	@param end [in]範囲の終了。このポインタと同じポインタを終了位置とする。
	@return 指定の範囲のパーツを返す。存在しない場合、サイズ0のオブジェクトを返す
	*/
	SearchResultsUptr range(
		const HtmlPart* p_start, const HtmlPart* p_end)const;
private:
	unique_ptr<HtmlPartUptrs> m_stockedPartUptrsUptr;
	unique_ptr<HtmlNode> m_rootNodeUptr;
};


class TagHtmlPart : public HtmlPart {
public:
	typedef std::map<std::string, vector<string> > AttrMap;
	TagHtmlPart(const string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){ parseTag(); };
	virtual ~TagHtmlPart(){};
	virtual const HtmlPart::Type type() const{ return TAG; };
	virtual const string& tagName()const{ return m_tagName; };
	virtual const std::string& attr(const std::string& p_key, const std::size_t p_index)const;
	virtual const bool hasAttr(const std::string& p_key, const std::size_t p_index)const;
	///属性名の一覧
	virtual std::unique_ptr<vector<const std::string*>> attrNames()const;
protected:
	void parseTag();
private:
	std::string m_tagName;
	AttrMap m_attrMap;
};



class TextHtmlPart : public HtmlPart {
public:
	TextHtmlPart(const string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){};
	virtual const HtmlPart::Type type() const{ return TEXT; };
};

class CommentHtmlPart : public HtmlPart {
public:
	CommentHtmlPart(const std::string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){};
	virtual const HtmlPart::Type type() const{ return COMMENT; };
};

///doctypeや?xmlなどを表す。
class DeclarationHtmlPart : public HtmlPart {
public:
	DeclarationHtmlPart(const std::string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){};
	virtual const HtmlPart::Type type() const{ return DECLARATION; };
};

///タグの終了（＞）が見つからない要素を表す 
class NotEndHtmlPart : public HtmlPart {
public:
	NotEndHtmlPart(const std::string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){};
	virtual const HtmlPart::Type type() const{ return NOT_END; };
};

//HtmlParserのハンドラ----------------------------
/**
@brief HTMLパースをするためのハンドラ。使用する派生クラスで解析の仕方を変わる。
*/
class HtmlSaxParserHandler :noncopyable{
public:
	HtmlSaxParserHandler(){};
	virtual ~HtmlSaxParserHandler(){};
	///開始を知らせる。ハンドラの初期化用。 
	virtual void start() = 0;
	///テキストの場合にSaxから呼び出される。 
	virtual void text(const std::string& p_str, const long p_line, const long p_pos) = 0;
	///タグ（コメント以外の＜＞でくくられたもの）の場合にSaxから呼び出される。 
	virtual void tag(const std::string& p_str, const long p_line, const long p_pos) = 0;
	///コメント（＜！－－－－＞）の場合にSaxから呼び出される。
	virtual void comment(const std::string& p_str, const long p_line, const long p_pos) = 0;
	///タグの終了（＞）がない場合にSaxから呼び出される。
	virtual void notEnd(const std::string& p_str, const long p_line, const long p_pos) = 0;
};


/**
@brief HTMLを解析し、タグ配列( HtmlPart の配列)を返すHTMLパーサハンドラ。
*/
class SimpleHtmlSaxParserHandler :public HtmlSaxParserHandler {
public:
	SimpleHtmlSaxParserHandler(){};
	virtual ~SimpleHtmlSaxParserHandler(){};
	virtual void start(){
		m_resultPartsUptrsUptr = unique_ptr<HtmlDocument::HtmlPartUptrs>(new HtmlDocument::HtmlPartUptrs);
	};
	virtual void text(const std::string& p_str, const long p_line, const long p_pos){
		unique_ptr<HtmlPart> ptr(new TextHtmlPart(p_str, p_line, p_pos));
		m_resultPartsUptrsUptr->push_back(move(ptr));
	};
	virtual void tag(const std::string& p_str, const long line, const long pos);
	virtual void comment(const std::string& p_str, const long p_line, const long p_pos){
		unique_ptr<HtmlPart> ptr(new CommentHtmlPart(p_str, p_line, p_pos));
		m_resultPartsUptrsUptr->push_back(move(ptr));
	};
	virtual void notEnd(const std::string& p_str, const long p_line, const long p_pos){
		unique_ptr<HtmlPart> ptr(new NotEndHtmlPart(p_str, p_line, p_pos));
		m_resultPartsUptrsUptr->push_back(move(ptr));
	}
	//パースした結果を取得する。実行前か結果取得後はnullptrが返る。
	unique_ptr<HtmlDocument::HtmlPartUptrs> result(){
		return move(m_resultPartsUptrsUptr);
	};

private:
	unique_ptr<HtmlDocument::HtmlPartUptrs> m_resultPartsUptrsUptr;
};



//HtmlSaxParser-----------------------------------------------
/**
@brief HTMLパーサ。派生クラスは存在しない。使用するハンドラを変えることで解析方法を変えられる。
*/
class HtmlSaxParser :noncopyable{
public:
	void parse(std::istream& is, HtmlSaxParserHandler& handler);
protected:
	const bool copyUntilFind(string& p_str, istream& p_is, const char p_targetC);
	const bool copyUntilFindCommentClosed(string& p_str, istream& p_is);
	//クリア関数(保管変数strのクリアと、現在の位置と行数を保管する)
	inline void clearStr(string& str, long& line, long& pos){ str.clear(); line = m_line; pos = m_pos; };
private:
	long m_line, m_pos;//行数, 位置（先頭からのバイト数）
};



} //namespace nana


#endif  // #ifndef NANA_HTML_DOCUMENT_INCLUDED