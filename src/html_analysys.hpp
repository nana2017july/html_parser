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
#ifndef NANA_HTML_ANALYSIS_INCLUDED
#define NANA_HTML_ANALYSIS_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <exception>


#include "html_element.hpp"


namespace nana{



using std::string;
using std::vector;
using std::istream;
using std::unique_ptr;




//実験--------------------------------------------



const bool forwardMatch(const string& targetStr, const string& searchStr);

const bool backwardMatch(const string& targetStr, const string& searchStr);
const bool wildcardMatch(const char *ptn, const char *str);


/**
<pre>
HTMLタグのタグ解析をする。一番内側の開始タグが閉じられるまで走査していく手法（階層マッチ）。
【解析例（解析手法によってHTMLタグ記述が間違っている場合に結果の階層構造が違ってくる）】
（入力）&lt;html>&lt;form>&lt;div>&lt;/form>&lt;/div>&lt;/html>
（結果）
　┗&lt;html>開始タグ
　　┗&lt;form>開始タグ
   　　　┗&lt;div> 開始終了が揃ったノード
	 　　　┗&lt;/form>終了タグ
	┗&lt;/html>終了タグ
</pre>
@param p_ret [out]このオブジェクトに結果を追加する
@param p_allDocParts [in]全てのHtmlPartsを順番(HTML記述の順番通り)に入れたもの
*/
void analyzeHtmlNode(HtmlNode& p_ret, const HtmlDocument::HtmlPartUptrs& p_allDocParts);


/**
<pre>
HTMLタグのタグ解析をする。一番近い終了タグとマッチしていく手法（同じタグ名どうしで出現順にタグマッチ）。
【解析例（解析手法によってHTMLタグ記述が間違っている場合に結果の階層構造が違ってくる）】
（入力）&lt;html>&lt;form>&lt;div>&lt;/form>&lt;/div>&lt;/html>
（結果）
　┗html　開始終了が揃ったノード
 　　┗form　開始終了が揃ったノード
   　　　┗div　開始終了が揃ったノード
</pre>
@param p_ret [out]このオブジェクトに結果を追加する
@param p_allDocParts [in]全てのHtmlPartsを順番(HTML記述の順番通り)に入れたもの
*/
void analyzeHtmlNodeBySameTagMatch(HtmlNode& p_ret, const HtmlDocument::HtmlPartUptrs& p_allDocParts);

/**
@brief HTMLを解析し、 HtmlDocument を作成して返すHTMLパーサハンドラ。
@see analyzeHtmlNode
*/
class DocumentHtmlSaxParserHandler :public SimpleHtmlSaxParserHandler {
public:
	DocumentHtmlSaxParserHandler(){};
	virtual ~DocumentHtmlSaxParserHandler(){};
	//パースした結果を取得する。実行前か結果取得後はnullptrが返る。
	unique_ptr<HtmlDocument> result();
};


//---------------------------------------------
/**
@brief HtmlNodeにアクセスし、タグ等の検査をするアクセサの基底クラス
@see HtmlNodeVisitor
*/
class HtmlNodeAccessor: noncopyable{
public:
	virtual ~HtmlNodeAccessor(){};
	/** タグにアクセス（チェック）する */
	virtual void access(const HtmlNode& p_node) = 0;
	/** 初期化。実行前に呼ばれる。 */
	virtual void init() = 0;
};

/**
@brief HtmlNodeを順番にすべて訪問するクラス。訪問した時に HtmlNodeAccessor を呼び出す。
@see HtmlNodeAccessor
*/
class HtmlNodeVisitor: noncopyable{
public:
	virtual ~HtmlNodeVisitor(){};
	//
	void access(const HtmlNode& p_node, HtmlNodeAccessor& p_accessor){
		p_accessor.init();
		_access(p_node, p_accessor);
	};
protected:
	void _access(const HtmlNode& p_node, HtmlNodeAccessor& p_accessor){
		//ルートノード以外の場合
		if(!(p_node.startTag() == nullptr && p_node.endTag() == nullptr))
			p_accessor.access(p_node);
		for(auto i = p_node.begin(); i != p_node.end(); ++i){
			_access(**i, p_accessor);
		}
	};
};

/**
@brief 複数のアクセスクラスを取りまとめて、ノードにアクセスできるアクセサクラス。
*/
class CompositeAccessor : public HtmlNodeAccessor{
public:
	///newしたオブジェクトを渡すこと。廃棄の管理はこのクラスで行う。
	CompositeAccessor& add(HtmlNodeAccessor* p_accs){
		m_nodeAccessorUptrs.push_back(unique_ptr<HtmlNodeAccessor>(p_accs));
		return *this; 
	};
	//
	virtual void access(const HtmlNode& p_node){
		for(auto i = m_nodeAccessorUptrs.begin(); i != m_nodeAccessorUptrs.end(); ++i){
			(*i)->access(p_node);
		}
	};
	//初期化
	virtual void init(){
		for(auto i = m_nodeAccessorUptrs.begin(); i != m_nodeAccessorUptrs.end(); ++i) (*i)->init();
	};
	/** 結果を取得するためのヘルパー関数
	@param T [in] add() したアクセサクラス
	@param p_index [in]アクセサを指定。 add() した順番（0～）
	*/
	template<class T>
	T& accessor(const int p_index){ return *dynamic_cast<T*>(m_nodeAccessorUptrs[p_index].get()); };
private:
	vector<unique_ptr<HtmlNodeAccessor>> m_nodeAccessorUptrs;
};

/** 
@brief 閉じていないタグと、互い違いになっているタグの抽出をするアクセサクラス。 
*/
class EndTagAccessor : public HtmlNodeAccessor{
public:
	typedef vector<const HtmlNode*> SearchResults;
	typedef unique_ptr<SearchResults> SearchResultsUptr;
	virtual ~EndTagAccessor(){};
	virtual void access(const HtmlNode& p_node);
	//初期化
	virtual void init(){
		m_nonClosedResult.reset(new SearchResults);
		m_alternatedResult.reset(new SearchResults);
		m_stockMap.clear();
	};
	///閉じていないタグの抽出結果
	SearchResultsUptr nonClosedResult();
	///互い違いになっているタグの抽出結果
	SearchResultsUptr alternatedResult(){ return move(m_alternatedResult); };
private:
	SearchResultsUptr m_nonClosedResult;
	SearchResultsUptr m_alternatedResult;
	std::map<std::string, std::vector<const HtmlNode*>> m_stockMap;
};

/**
@brief HTML5で禁止になったタグを抽出するアクセサクラス。
*/
class DeprecatedInHtml5Accessor : public HtmlNodeAccessor{
public:
	typedef vector<const HtmlNode*> SearchResults;
	typedef unique_ptr<SearchResults> SearchResultsUptr;
	virtual ~DeprecatedInHtml5Accessor(){};
	virtual void access(const HtmlNode& p_node){
		if(s_deprecatedTagMap.find(p_node.tagName()) != s_deprecatedTagMap.end()){
			m_result->push_back(&p_node);
		}
	};
	//初期化
	virtual void init(){ m_result.reset(new SearchResults); };
	//結果取得
	SearchResultsUptr result(){
		return move(m_result);
	};
private:
	SearchResultsUptr m_result;
	static const std::map<std::string, int> s_deprecatedTagMap;
};


/** 
@brief imgタグにalt属性が存在しないタグを抽出するアクセサクラス。（アクセシビリティの観点では重要なチェック項目。）
*/
class ImgAltAccessor : public HtmlNodeAccessor{
public:
	typedef vector<const HtmlNode*> SearchResults;
	typedef unique_ptr<SearchResults> SearchResultsUptr;
	virtual ~ImgAltAccessor(){};
	virtual void access(const HtmlNode& p_node){
		if(p_node.tagName() != "img") return;
		if(p_node.startTag() == nullptr) return; 
		if(!p_node.startTag()->hasAttr("alt", 0)){
			//alt属性が存在しない場合
			m_result->push_back(&p_node);
		}
	};
	//初期化
	virtual void init(){ m_result.reset(new SearchResults); };
	//
	SearchResultsUptr result(){
		return move(m_result);
	};
private:
	SearchResultsUptr m_result;
};


//-------------------------------
namespace path{

	/**
	@brief XPathもどきを扱うための基底クラス。パス抽出の実行者の１つを表す。
	*/
	class HtmlPath: noncopyable{
	public:
		typedef vector<const HtmlNode*> HtmlNodePtrs;
		virtual ~HtmlPath(){};
		/** 引数の配下のノードをフィルタし、指定のノードを抽出する。 */
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePtrs) = 0;
	};

	/**
	@brief タグ名と属性の検索（1階層/tagの指定）。
	*/
	class PathHtmlPath :public HtmlPath{
	public:
		/**
		@param p_tagName [in]タグ名をワイルドカードで指定
		*/
		PathHtmlPath(const string& p_tagName):m_tagName(p_tagName){};
		virtual ~PathHtmlPath(){};
		/**
		@param p_htmlPath [in]述語(pred)を指定
		*/
		PathHtmlPath* add(HtmlPath* p_htmlPathPtr){
			m_htmlPathUptrList.push_back(unique_ptr<HtmlPath>(p_htmlPathPtr));
			return this;
		};
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePVec);
	protected:
		///フィルタした結果をretVecに追加する
		void match(HtmlNodePtrs& retVec, const vector<const HtmlNode*>& p_matchedChildrenNodeList)const;
	private:
		vector<unique_ptr<HtmlPath>> m_htmlPathUptrList;
		const string m_tagName;
	};

	/**
	@brief 要素番号（述語）の指定。イメージは、tagname[n]。
	*/
	class PositionHtmlPath :public HtmlPath{
	public:
		PositionHtmlPath(const std::size_t p_pos): m_position(p_pos){};
		///
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePVec);
	private:
		std::size_t m_position;
	};

	/**
	@brief 属性（述語）指定。イメージは、tagname[@id="xxx"]
	*/
	class AttributesHtmlPath :public HtmlPath{
	public:
		///ワイルドカードで指定する
		AttributesHtmlPath(const string& p_attrName, const string& p_attrVal)
			:  m_attrName(p_attrName), m_attrVal(p_attrVal){};
		///
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePVec);
	private:
		const string m_attrName;
		const string m_attrVal;
	};

	/**
	@brief 子孫のタグ（//Descendant or self）検索。イメージは、//tagname
	*/
	class DescendantsHtmlPath :public HtmlPath{
	public:
		DescendantsHtmlPath(){};
		///
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePVec);
	protected:
		///自身のノードを含めて配下のノードをすべてresultに設定する
		void _filter(HtmlNodePtrs& p_result, const HtmlNodePtrs& p_nodePVec, std::map<const HtmlNode*, int>& p_alreadyRegNodeMap);
	private:
	};
	
	//------------------------------------
	/**
	@brief Htmlパスの実行者。
	*/
	class HtmlPathExecutor: noncopyable{
	public:
		typedef HtmlPath::HtmlNodePtrs HtmlNodePtrs;
		virtual ~HtmlPathExecutor(){};
		/** ノード１つを引数に実行（通常はルートノードを渡す）*/
		unique_ptr<HtmlNodePtrs> exec(const HtmlNode& p_node)const;
		/** 複数のノードを引数に実行（HtmlPath実行結果をさらにフィルタしたい場合などに使用）*/
		unique_ptr<HtmlNodePtrs> exec(const HtmlNodePtrs& p_nodePtrs)const;
		/** 登録したアクセサをすべて削除する */
		void clear(){ m_htmlPathVec.clear(); m_isTag = false; };
		/** 自分でパスを自由に追加する */
		HtmlPathExecutor& add(HtmlPath* p_path);
		/** 子孫すべての指定（descendant or self）*/
		HtmlPathExecutor& slash2(){ add(new DescendantsHtmlPath()); m_isTag = false; return *this; };
		/**
		タグ名の指定
		@param tagName [in]タグ名(ワイルドカード指定)
		*/
		HtmlPathExecutor& tag(const string& p_tagName);
		/**
		述語：属性の指定(attribute)。tag()以外の後に使用してはいけない。
		@param p_tagName [in]タグ名（ワイルドカード指定）
		@param p_val [in]値（ワイルドカード指定）
		@exception runtime_exception tag()の後に使用していない場合。
		*/
		HtmlPathExecutor& predAttr(const string& p_tagName, const string& p_val);
		/**
		述語：要素番号の指定（position）。tag()以外の後に使用してはいけない。
		@param p_index [in]要素番号
		@exception runtime_exception tag()の後に使用していない場合。
		*/
		HtmlPathExecutor& predPos(const int p_index);
	private:
		vector<unique_ptr<HtmlPath>> m_htmlPathVec;
		bool m_isTag;
	};

}//namespace path

} //namespace nana


#endif  // #ifndef NANA_HTML_ANALYSIS_INCLUDED
