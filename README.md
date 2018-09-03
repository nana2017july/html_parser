# HTMLパーサのサンプルコード
C++11の勉強のために試しに作成したコードです。<br>
HTMLのタグが正しく閉じていなくても動作するパーサです。<br>
HTMLの閉じタグのチェックをするサンプルコードを作ろうとして、その過程で作成したクラス等を公開しています。<br>
全てのコードで1Kステップ以下のとても軽量なライブラリです。
<br>

## このHTMLパーサで、できること
サンプルコードを使用すると以下のことができます。<br>
* HTMLのパース
* タグ名、属性値、HTML上の位置（行数と先頭からのバイト数）の取得
* 閉じタグのチェック
* 互い違いタグのチェック
* XPathもどきの形式（仮称：HtmlPath）でHTMLのタグ要素の取得
* 自作チェックの追加
* 自作パースの方法の追加
* 自作HtmlPathの追加

<br>

## はじめに
勉強のために作成してますので、関数名などが一般的、直感的ではないと思います。<br>
それでも参考になるようでしたら嬉しいです。<br>
使用、改変は自由ですが、ご利用は全て自己責任でお願いします。<br>
ライセンスは、Apache License, Version 2.0とします。<br>
<br>
**【関数・クラスのドキュメント（doxygen）】**<br>
https://nana2017july.github.io/html_parser/index.html<br> 
<br>


## 変更履歴
ver.2.2 新規作成<br>
ver.2.3 不具合修正。ドキュメントの変更なし。<br>
ver.2.4 centos7 g++(4.8.5)でコンパイルが通るようにした。g++用makefile追加。<br>


## 動作環境
【コンパイラ】<br>
以下のコンパイル環境で使用できることを条件にしました。<br>
* Dev-C++（もうオワコン？）
* Microsoft Visual Studio C++ 2017 
* CentOS7 g++(4.8.5)

【言語】<br>
* C++11

【対応文字コード】<br>
SJIS、EUC、UTF-8に対応しているはずです。<br>
ただし文字コード変換はしないため、クラスから取得したパース後の文字列は元のHTMLの文字コードのままです。<br>
各自、クラスから文字列を取得後に、変換したい文字コードに変換してご使用ください。<br>
<br>


## 環境に関する補足
実は、Dev-C++のC++11が標準の仕様と結構違っていました。<br>
以下のキーワードが使用できなかったので、使用しないようにしました。（本当は使いたかったのですが）<br>
* using xxx = yyy;
* default  （virtual ~T() = defaultなどができなかった）	
* final　　（class xxx final {～ ができなかった）
* override
<br>

**【Microsoft Visual Studio C++の補足】**<br>
Microsoft Visual Studio C++の場合、デフォルトでプロジェクトを作成すると、"stdafx.h"のincludeが必要になってしまい、<br>
includeしないとエラーになってしまいます。<br>
これを避けるためには、プロジェクト自体か、もしくは.cppのファイルすべてについて以下のことを設定してみてください。<br>
・左のエクスプローラ<br>
　　⇒プロジェクトもしくはcppファイルを右クリック<br>
　　⇒プロパティを選択<br>
　　⇒上部の「構成」コンボボックスを「すべての構成」に変更<br>
　　⇒構成プロパティ ／ C/C++ ／ プリコンパイル済みヘッダー　を選択<br>
　　⇒「プリコンパイル済みヘッダー」を「プリコンパイル済みヘッダーを使用しない」に変更<br>
<br>


## 対応文字コードに関する技術的補足（なぜ対応できているのか？）
このパーサは入力のHTML文字列を文字コード変換せずにパース（解析）をしています。<br>
しかし、単にタグの開始文字（&lt;）と終了文字(&gt;)を探しているだけなので、上記3つの文字コードについては解析に影響を与えません。<br>
なぜなら上記の文字コードは、全角文字の構成バイトに、開始文字（&lt;）や終了文字(&gt;)、クォート("、')のバイトが含まれないからです。<br>
分からない方のためにもう少し補足すると、文字コードが問題になるケースは、全角文字を構成するバイトの途中で<br>
開始文字（&lt;）などのバイトにマッチしてしまう場合です。パーサが全角文字の途中のバイトを開始文字と判断してしまうと、<br>
その全角1文字を構成するバイトの途中からタグの開始をしていると判断して、分解してしまうからです。<br>
上記以外の文字コードについては調べていないので、問題ないかどうかはわかりません。<br>
<br>


## インストール方法
自分のC++プロジェクトにhtml_element.cpp、html_analysys.cppを追加して、一緒にコンパイルするだけです。<br>
あとはソースファイルでそれぞれの.hppファイルをincludeするだけです。<br>
<br>

## クラスの簡易な説明
とりあえず覚えておくクラスは以下のクラスだけで十分です。<br>

|**クラス名**|**説明**|
|---|---|
|***HtmlDocumentクラス***| HTML全体とノードの構造を持つクラス。ルートノード（HtmlNode）と、HtmlPartの実体の保持と破棄を担う。|
|***HtmlNodeクラス***| 要素（開始タグ＋終了タグ）を表すノードです。<br>一般的にはノードというとTextノードなども含みますが、ここでは含みません。閉じタグチェックの目的で作ったのでテキストの実装はやめました（簡易にするため）。ただ、テキストを取得することもできます。|
|***HtmlPartクラス***| 1つのクラスは、１つの開始タグ、または、終了タグ、テキスト、コメントなどを表し、実際の記述内容を保持するHTML部品クラス。|


実際のHTMLと対応させると以下のようになります。<br>
<br>

**【HtmlPartとHTMLの対応】**<br>

```
<!doctype >           HtmlPart[1](タイプ：DECLARATION)
<html>                HtmlPart[2](タイプ：TAG)
  <div>               HtmlPart[3](タイプ：TAG)
    おはよう          HtmlPart[4](タイプ：TEXT)
  </div>              HtmlPart[5](タイプ：TAG)
  <!-- コメント -->   HtmlPart[6](タイプ：COMMENT)
</html>               HtmlPart[7](タイプ：TAG)
```
<br>

**【HtmlNodeの構成（HtmlPartとHTMLとの対応）】**
```
<html>                 HtmlNode (<html>要素を表す。HtmlPart[2]、HtmlPart[7]を内部に保持)
　┗<div>              HtmlNode (<div>要素を表す。HtmlPart[3]、HtmlPart[5]を内部に保持)
　　┗おはよう         対応クラスなし（HtmlNode）では扱わない
　┗<!-- コメント -->  対応クラスなし（HtmlNode）では扱わない
```
<br>


## 使用例
**【一番簡易なパーサ使用例（HTMLのパーツ（タグ、テキスト）を出力）】**<br> 
```cpp
#include <iostream>
#include <fstream>
#include <sstream>
//
#include "html_element.hpp"
#include "html_analysys.hpp"


using namespace std;

int main(int argc, char *argv[]){
	string str("<html><form><div>サンプル</div></form></html>");
	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	
	//パース
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();
	
	//パース結果の出力（(*i)は(const HtmlPart*)）
	for(nana::HtmlDocument::const_iterator i = docUptr->begin(); i != docUptr->end(); ++i){
	  cout << **i << "#" << (*i)->tagName() << endl;
	}
	return 0;
}
```

＜出力例（カッコ内はHTML上のタグの行数）＞<br>
```
[TAG(1)]<html>#html
[TAG(1)]<form>#form
[TAG(1)]<div>#div
[TEXT(1)]サンプル#
[TAG(1)]</div>#/div
[TAG(1)]</form>#/form
[TAG(1)]</html>#/html
```
<br>
<br>

**【XPathもどきの記法（仮称HtmlPath）でHTMLノードを取得する例１】**<br> 
```cpp
string str("<html><div id='main'><form name='f'><input name='1'></form></div><input name='2'></html>");

nana::HtmlSaxParser parser;
nana::DocumentHtmlSaxParserHandler handler;
istringstream is(str);
parser.parse(is, handler);
unique_ptr<nana::HtmlDocument> docUptr = handler.result();

//HtmlPathの実行クラス
nana::path::HtmlPathExecutor executor;

//以下は"//div[@id='main']//input"を指定しているのと同じ
executor.slash2().tag("div").predAttr("id", "main").slash2().tag("input");
unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec = executor.exec(docUptr->rootNode());

//実行結果の出力。(*i)は(const HtmlNode*)
for(auto i = retExec->begin(); i != retExec->end(); ++i){
	cout << **i << endl;
}
```

＜出力例＞<br>
```
0000000000239080[/html/div/form/input](input,input):[TAG(1)]<input name='1'>
```
<br>
<br>

**【XPathもどきの記法でHTMLノードを取得する例２：　ワイルドカードを使用する】**<br> 
HtmlPathでは、*や?のワイルドカードも使用可能。属性指定の述語でもワイルドカードを使用可能。<br>
ワイルドカードのエスケープ文字は\。<br>
```cpp
string str("<html><div id='main'><form name='f'><input name='1'></form></div><input name='2'></html>");

nana::HtmlSaxParser parser;
nana::DocumentHtmlSaxParserHandler handler;
istringstream is(str);
parser.parse(is, handler);
unique_ptr<nana::HtmlDocument> docUptr = handler.result();

//HtmlPathの実行クラス
nana::path::HtmlPathExecutor executor;
//以下は"/html/*"を指定しているのと同じ
executor.tag("html").tag("*");
unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec = executor.exec(docUptr->rootNode());

//結果の出力
for(auto i = retExec->begin(); i != retExec->end(); ++i){
	cout << (*i)->pathStr() << endl;
}
```

＜出力例＞<br>
```
/html/div
/html/input
```
<br>
<br>



**【HtmlNode、HtmlPartの使用例】**<br>
パース結果やHtmlPathの結果は、HtmlNodeクラス、HtmlPartクラスで返されます。それらで使用できるメソッドの一部を紹介します。<br>
```cpp
//上記のHtmlPathの使用例で（/html/div）を指定した場合の結果retExec変数を使用します。
const nana::HtmlNode& divNode = **retExec->begin();

//ノードは、（/html/div）とします
cout << "pathStr(): " << divNode.pathStr() << endl;
cout << "tagName(): " << divNode.tagName() << endl;

//子ノードをすべて出力（テキストノードが含まれないことも確認ポイント）
for(auto i = divNode.begin(); i != divNode.end(); ++i){
	cout << "child: " << (*i)->pathStr() << endl;
}

//タグ要素のHtmlPart
const nana::HtmlPart& part = *divNode.startTag();
cout << "HtmlPart::attr(): " << part.attr("id", 0) << endl;
```

＜出力例＞<br>
```
pathStr(): /html/div
tagName(): div
child: /html/div/form
HtmlPart::attr(): main
```
<br>
<br>



**【閉じタグのチェック例】**<br>
閉じタグのチェックの例です。他にもHTML5で廃止されたタグの使用もチェックできます。チェックは自作もできます。<br>
```cpp
string str("<html><div id='main'><form name='f'><input name='1'></div></form><input name='2'>");

nana::HtmlSaxParser parser;
nana::DocumentHtmlSaxParserHandler handler;
istringstream is(str);
parser.parse(is, handler);
unique_ptr<nana::HtmlDocument> docUptr = handler.result();

//タグチェック
nana::EndTagAccessor acc;
nana::HtmlNodeVisitor vis;

//チェック実行
vis.access(docUptr->rootNode(), acc);

//結果取得(結果の型は unique_ptr<vector<const HtmlNode*>>)
auto nonClosedUptr = acc.nonClosedResult();
auto alternatedUptr = acc.alternatedResult();

//(*i)は、(const HtmlNode*)
cout << "閉じタグチェック結果" << endl;
for(auto i = nonClosedUptr->begin(); i != nonClosedUptr->end(); ++i){
	std::cout << **i << endl;
}
cout << "入れ違いタグチェック結果" << endl;
for(auto i = alternatedUptr->begin(); i != alternatedUptr->end(); ++i){
	std::cout << **i << endl;
}
```

＜出力例（先頭の数字はオブジェクト（HtmlNode）のポインタ）＞<br>
```
閉じタグチェック結果
0000000000318F80[/html](html,):[TAG(1)]<html>
入れ違いタグチェック結果
0000000000319180[/html/div/form//div](,/div):[TAG(1)]</div>
```
<br>
<br>



**【複数のチェック方法を使用する例（閉じタグチェックとHTML5で廃止されたタグのチェック）】**<br>
```cpp
string str("<html><form name='f'><blink></blink><input name='2'></html>");

nana::HtmlSaxParser parser;
nana::DocumentHtmlSaxParserHandler handler;
istringstream is(str);
parser.parse(is, handler);
unique_ptr<nana::HtmlDocument> docUptr = handler.result();

//チェック方法は複数存在。複数同時使用する場合はCompositeを使用する。
nana::CompositeAccessor acc;
acc.add(new nana::EndTagAccessor)
	.add(new nana::DeprecatedInHtml5Accessor);
nana::HtmlNodeVisitor vis;

//実行
vis.access(docUptr->rootNode(), acc);

//結果取得(アクセサを指定して取得する)
auto nonClosedUptr = acc.accessor<nana::EndTagAccessor>(0).nonClosedResult();
auto deprecatedUptr = acc.accessor<nana::DeprecatedInHtml5Accessor>(1).result();

//実行結果の出力。(*i)は、(const HtmlNode*)
cout << "閉じタグチェック結果" << endl;
for(auto i = nonClosedUptr->begin(); i != nonClosedUptr->end(); ++i){
	std::cout << **i << endl;
}
cout << "HTML5で廃止されたタグチェック結果" << endl;
for(auto i = deprecatedUptr->begin(); i != deprecatedUptr->end(); ++i){
	std::cout << **i << endl;
}
```

＜出力例（先頭の数字はクラスのポインタ）＞<br>
```
閉じタグチェック結果
0000000000349100[/html/form](form,):[TAG(1)]<form name='f'>
HTML5で廃止されたタグチェック結果
0000000000349080[/html/form/blink](blink,/blink):[TAG(1)]<blink>
```
<br>
<br>


**【自作のチェックを作成する例】**<br>
例えばimgタグにalt属性を記述しているかをチェックするクラス。<br>
ここではクラスだけ記述します。使い方は上記の閉じタグのチェックの例と同じです。
```cpp
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
```

<br>
<br>

**【テキストまで含めて扱う使用例（HtmlPart）】**<br>
```cpp
string str("<html><div id='main'><form name='f'>\n<!-- memo -->Input Text:<input name='1'></form></div></html>");

nana::HtmlSaxParser parser;
nana::DocumentHtmlSaxParserHandler handler;
istringstream is(str);
parser.parse(is, handler);
unique_ptr<nana::HtmlDocument> docUptr = handler.result();

//HtmlPathの実行クラス
nana::path::HtmlPathExecutor executor;

//divノードを抽出する
executor.slash2().tag("div");
unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec = executor.exec(docUptr->rootNode());
const nana::HtmlNode& divNode = *(*retExec)[0];

//divタグの開始から、終了までの間にあるHtmlPartをすべて取得する
nana::HtmlDocument::SearchResultsUptr divPartsUptr = docUptr->range(divNode.startTag(), divNode.endTag());

//実行結果の出力。(*i)は(const HtmlPart*)
for(auto i = divPartsUptr->begin(); i != divPartsUptr->end(); ++i){
	cout << **i << endl;
}
```

＜出力例＞<br>
```
[TAG(1)]<div id='main'>
[TAG(1)]<form name='f'>
[TEXT(1)]
←※注：空行に見えますが、改行（\n）が出力されています。
[COMMENT(2)]<!-- memo -->
[TEXT(2)]Input Text:
[TAG(2)]<input name='1'>
[TAG(2)]</form>
[TAG(2)]</div>
```
<br>
<br>

<br>
<br>



