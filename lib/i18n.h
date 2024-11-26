/**
 * 头文件：i18n.h
 * 顾名思义，此头文件的内容是MiniDB国际化文本相关处理。
 * 写不出主要功能（指表达式计算）然后怒而搓了个I18n玩玩。
 */
#ifndef __I18N_MINIDB_H__
#define __I18N_MINIDB_H__

#include "environment.h"

namespace minidb{

string g_LangCode = "en";

enum class return_status {		
	success,
	argscerr,
	invarg,
	ferr,
	syntaxerr,
	unexpt = -1,
	i18nwk = -2,
	exbase = -3
};

class i18nstring {
	private:
		string pattern;
		vstring params;
	public:
		i18nstring(const char* s) { pattern = s; }
		i18nstring(const string s = "", const vstring v = {}):pattern(s),params(v){}
		void setPattern(const string s) { pattern = s;}
		void setParams(const vstring v) { params = v; }
		string prints() const;
};

// 所有可能的国际化字符串
map<string, i18nstring> g_I18nStrDict;

class MiniDBExceptionBase {
	protected:
		i18nstring msg;
	public:
		MiniDBExceptionBase(const char* s) { msg = s; };
		MiniDBExceptionBase(const string s = "") { msg = s; };
		MiniDBExceptionBase(const i18nstring s) { msg = s; };
		virtual const string what() const { return msg.prints(); }
		virtual return_status status() const { return return_status::exbase; }
};

class WrongI18nKey extends public MiniDBExceptionBase {
	private:
		string id;
	public:
		WrongI18nKey(const string s = ""):id(s){}
		virtual const string what() const override { return g_I18nStrDict.find("i18nwrongkey")->second.prints().c_str(); }
		virtual return_status status() const override { return return_status::i18nwk; }
};

ostream& operator<< (ostream& os, const i18nstring s);
bool isDigit(char);
string itos(int);
i18nstring author();
namespace i18n{
	void readKvpairs();
	i18nstring parseKey(const string, vstring = {});
	void initialize_hardcoded();
}


// 函数体定义多在下方


// 解析国际化字符串的参数并返回解析后字符串
string i18nstring::prints() const {
	string p = pattern;
	string res = "";
	int index = 0;
	bool nflag = false;
	for (char ch : pattern) {
		if (nflag) {
			if (ch == '%') {
				nflag = false;
				res.push_back('%');
			}
			else if (isDigit(ch)) {
				index *= 10;
				index += (ch - '0');
			}
			else {
				nflag = false;
				res = res + params.at(index-1) + ch;
				index = 0;
			}
		}
		else {
			if (ch == '%') {
				nflag = true;
			}
			else res.push_back(ch);
		}
	}
	if (nflag) res = res + params.at(index-1);
	return res;
}

// 重载ostream的左移运算符以更方便地输出国际化字符串
ostream& operator<< (ostream& os, const i18nstring s) {
	return os << s.prints();
}

bool isDigit(char ch) {
	return (ch == '0' or ch == '1' or ch == '2' or ch == '3' or ch == '4' or 
			ch == '5' or ch == '6' or ch == '7' or ch == '8' or ch == '9');
}

namespace i18n {

	// 读取国际化字符串
	void readKvpairs() {
		#ifdef __ENABLE_I18N__
			initialize_hardcoded();
			ifstream langf;
			langf.open(string("i18n/")+g_LangCode+".ini", ios::in);
			if (!langf.is_open()) {
				langf.open("i18n/zh_cn.ini", ios::in);
				if (!langf.is_open()) {
					g_LangCode = "zh_cn";
					throw MiniDBExceptionBase(i18n::parseKey("openlangf"));
				}
				else clog << i18n::parseKey("def_w_langf", {g_LangCode});
			}
			string line;
			while (getline(langf, line)) {
				auto pos = line.find('=');
				string id = line.substr(0, pos);
				string rval;
				bool trflag = false;
				for (char ch : line.substr(pos+1)) {
					if (trflag) {
						trflag = false;
						switch (ch) {
							case 'a':	ch = '\a';	break;
							case 'b':	ch = '\b';	break;
							case 'f':	ch = '\f';	break;
							case 'n':	ch = '\n';	break;
							case 'r':	ch = '\r';	break;
							case 'v':	ch = '\v';	break;
							case '\'':	ch = '\'';	break;
							case '"':	ch = '\"';	break;
							case '?':	ch = '\?';	break;
							case '0':	ch = '\0';	break;
						}
						rval.push_back(ch);
					}
					else if (ch == '\\') trflag = true;
					else rval.push_back(ch);
				}
				g_I18nStrDict.insert(pair<string, i18nstring>(id, i18nstring(rval)));
			}
			langf.close();
		#endif
		#ifndef __ENABLE_I18N__
			// 至尊打表之力
			g_I18nStrDict = {
				{"i18nwrongkey", "MiniDB> [Lang File Error] No such key named \"%1\" was found in en.ini language file."},
				{"welcome", "MiniDB> Welcome to MiniDB. Developed by %1 completely independently.\n--------------------"},
				{"exitstatus", "--------------------\nMiniDB> MiniDB exits with status %1."},
				{"unacptcmdl", "Unacceptable command line."},
				{"unacptvt", "Unacceptable value type \"%1\"."},
				{"unacptvarn", "Unacceptable variable name \"%1\"."},
				{"openlegfilef", "MiniDB> Failed to open legacy data file \"%1\"."},
				{"readlegfsuc", "MiniDB> Succeeded in reading legacy data."},
				{"openifilef", "Failed to open input file \"%1\"."},
				{"openofilef", "Failed to open output file \"%1\"."},
				{"atc", "MiniDB> All tasks accomplished."},
				{"opentmpf", "Failed to open temporary file \"%1\"."},
				{"incmpltstr", "Incomplete string."},
				{"incmpltparamlist", "Incomplete parameter list."},
				{"unexptstr", "Unexpected string \"%1\"."},
				{"unexptkw", "Unexpected keyword \"%1\"."},
				{"invalidmathop", "Invalid math operator \"%1\"."},
				{"invalidcmpop", "Invalid comparison operator \"%1\"."},
				{"invalidlgop", "Invalid logical operator \"%1\"."},
				{"nosuchdb", "No such database named \"%1\"."},
				{"nosuchtab", "No such table named \"%1\"."},
				{"nosuchterm", "No such term named \"%1\"."},
				{"noavaldb", "No database is currently using."},
				{"duplicatedb", "Duplicate database name \"%1\"."},
				{"duplicatetab", "Duplicate table name \"%1\"."},
				{"duplicateterm", "Duplicate term name \"%1\"."},
				{"incmpttypes", "Incompatible value types: (%1) and (%2)."},
				{"divzero", "Divzero."},
				{"vnfitt", "Value (%1) does not match the given type \"%2\"."},
				{"upp", "Unmatched parameter pattern."},
				{"redundant", "Redundant ',' after given parameters."},
				{"uncloparen", "Unclosed parenthesis."},
				{"strunexptsufx", "Unexpected suffix \"%1\" found after string constant \"%2\"."},
				{"strunexptprfx", "Unexpected prefix \"%1\" found before string constant \"%2\"."},
				{"strunexptsquote", "Unexpected single quote mark found in string \"%2\" at Pos.%1."},
				{"nmemspec", "No member specified despite table name given \"%1\"."},
				{"dupclunre", "Duplicate check in table \"%1\", leaving table \"%2\" unrelated."},
				{"invdupc", "Invalid duplicate check in table \"%1\"."},
				{"dupselwildc", "Duplicate selection. (Applying wildcard '*' and other selectors simultaneously.)"},
				{"dupselterm", "Duplicate selection. (Found duplicate term \"%1\".)"},
				{"exptsthgotnil", "Expected %1 but got nil."},
				{"exptsthgotothers", "Expected %1 but got %2."},
				{"exptkwgotnil", "Expected keyword \"%1\" but got nil."},
				{"exptkwgotothers", "Expected keyword \"%1\" but got \"%2\"."},
				{"exptparamsgotnil", "Expected parameter list (name type, ...) but got nil."},
				{"exptparamsgotothers", "Expected parameter list (name type, ...) but got \"%1...\"."},
				{"exptwheregotnil", "Expected keyword \"where\" but got nil. If you want to delete all data in the table, please use \"drop table\" statement."},
				{"gcrmtmpf", "MiniDB> [GC][Warning] Failed to remove temporary file \"%1\"."},
				{"gcrmtmps", "MiniDB> [GC] Deleted temporary file \"%1\"."},
				{"unexpectederr", "MiniDB> [Unexpected Error] An unexpected error occurred."},
				{"argscerr", "MiniDB> [Argument Count Error] %1 (Too %2 argument(s): %3 arg(s) expected"},
				{"argscerr_r", ", %1 received"},
				{"argscerr_e", ")"},
				{"invalidarg", "MiniDB> [Invalid Argument(s)] %1"},
				{"ferr", "MiniDB> [File Error] %1"},
				{"syntaxerr", "MiniDB> [Syntax Error] %1"},
				{"h_dataexh", "MiniDB> [Data Show]"},
				{"h_dataexhend", "MiniDB> [Data Show][End]"},
				{"h_debug_rawcmd", "MiniDB> [Debug] Raw command: "},
				{"much", "many"},
				{"less", "few"},
				{"w_nullstm", "MiniDB> [Warning] Received null statement."},
				{"l_createdb", "MiniDB> [Command] Database \"%1\" created."},
				{"l_createtab", "MiniDB> [Command] Table \"%1\" created."},
				{"l_createtabparam", "MiniDB> [Command][Parameter] name = \"%1\", type = \"%2\""},
				{"l_usedb", "MiniDB> [Command] Now using database \"%1\"."},
				{"l_droptab", "MiniDB> [Command] Table \"%1\" dropped."},
				{"l_insertion", "MiniDB> [Command] Inserting data into table \"%1\"."},
				{"l_insertionval", "MiniDB> [Command][Parameter] value = %1"},
				{"l_selection", "MiniDB> [Command] Selecting columns."},
				{"l_selectioncol", "MiniDB> [Command][Parameter] col_name = %1"},
				{"l_intab", "MiniDB> [Command][Parameter] in table \"%1\""},
				{"l_where", "MiniDB> [Command] Conditions:"},
				{"l_logicexpr", "MiniDB> [Command][Parameter] logic_expr | %1 %2 %3"},
				{"l_update", "MiniDB> [Command] Updating data."},
				{"l_assignment", "MiniDB> [Command][Parameter] let %1 be %2"},
				{"l_innerjoin", "MiniDB> [Command][Parameter] inner join logic_expr | %1 = %2"},
				{"p_tablename", "table name"},
				{"p_termname", "term name"},
				{"p_termvalue", "term value"},
				{"dupselwildc", "Duplicate selection. (Applying wildcard '*' and other selectors simultaneously.)"},
				{"dupselterm", "Duplicate selection. (Found duplicate term \"%1\".)"},
				{"def_w_langf", "MiniDB> [Warning] Failed to open language file \"%1.ini\", now using default language file."},
				{"openlangf", "MiniDB> [Language File Error] Failed to open language file en.ini."},
				{"authn", author()},
				{"notinfile", " (not in files)"},
				{"lnno", " (at Line %1)"}
			};
		#endif
	}

	// 这个函数实际上不进行parse，只是插入参数，但这样起名会让正文行文看起来好懂一些。
	i18nstring parseKey(const string id, vstring params) {
		auto p = g_I18nStrDict.find(id);
		if (p == g_I18nStrDict.end()) throw WrongI18nKey(id);

		i18nstring ins = p->second;
		ins.setParams(params);
		return ins;
	}

	// 部分国际化字符串可能在读取语言文件前被调用，因而不得不进行硬编码。
	// 仅在定义了宏__ENABLE_I18N__时被调用。
	void initialize_hardcoded() {
			if (g_LangCode == "zh_cn") {
				g_I18nStrDict = {
					{"openlangf", "MiniDB>【语言文件错误】未能成功打开语言文件zh_cn.ini。"},
					{"authn", author()},
					{"notinfile", "（不在输入文件内）"},
					{"lnno", "（位于第 %1 行）"}
				};
			}
			else if (g_LangCode == "en") {
				g_I18nStrDict = {
					{"def_w_langf", "MiniDB> [Warning] Failed to open language file \"%1.ini\", now using default language file."},
					{"openlangf", "MiniDB> [Language File Error] Failed to open language file en.ini."},
					{"authn", author()},
					{"notinfile", " (not in files)"},
					{"lnno", " (at Line %1)"}
				};
			}
	}

}

// int转字符串
string itos(int i) {
	string res;
	if (i < 0) {
		res.push_back('-');
		return res+itos(-i);
	}
	else if (i == 0) {
		return "0";
	}
	else while (i != 0) {
		res.push_back('0' + i % 10);
		i /= 10;
	}
	return string(res.rbegin(), res.rend());
}

// 你好，你好，你好，你好，你好，你好，你好，你好，你好，你好
i18nstring author() {
	if (g_LangCode == "zh_cn") return string("陈必珅");
	else return string("Bishen CHEN");
}


}
#endif