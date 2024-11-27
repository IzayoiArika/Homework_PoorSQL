/**
 * 头文件：stringop.h
 * 保存了一些可能用到的字符串处理、判断函数。
 */
#ifndef __STRINGOP_MINIDB_H__
#define __STRINGOP_MINIDB_H__

#include "auxiliaries.h"

namespace minidb {

template <typename T> bool doesContain(const T, const vector<T>);		// 在给定vector中查找key，找到返回true，否则返回false。
bool doesContain(const char, const string);							// 在给定string中查找char，找到返回true，否则返回false。
bool isReservedKeyword(const string);					// 判断给定字符串是否为关键字。
bool doesFitNameRequirement(const string);				// 判断给定字符串是否符合变量名命名原则（不考虑与关键字冲突的情况）。
bool isAcceptableName(const string); 					// 判断给定字符串是否符合变量名命名原则。
bool isToken(const string);								// 判断给定字符串是否应该被计入token。
bool isValidCmpOp(const string);						// 判断给定字符串是否为合法的比较运算符。

vstring splitParameters(const string, const string);	// 传入一系列分割符，将给定字符串按分割符切成数段（丢弃分隔符）返回。
vstring splitParameters(const string, const char);

string trim(const string);								// 去除字符串头尾空白
string trimDuplicateWs(const string);					// 去除字符串中间重复空白
void checkStrValidity(const vstring);					// 检查字符串是否有效

istream& getsUntil(istream&, string&, const string);	// 读取一整行字符串，直到遇到给定的字符串或读取出错为止。

const vector<char> g_Whitespaces = {' ', '\t', '\n'};		// 空白字符列表

namespace keywords {										// 字符串列表
	const kwstring create = "create";
	const kwstring drop = "drop";
	const kwstring database = "database";
	const kwstring use = "use";
	const kwstring table = "table";
	const kwstring insert = "insert";
	const kwstring into = "into";
	const kwstring inner = "inner";
	const kwstring join = "join";
	const kwstring values = "values";
	const kwstring select = "select";
	const kwstring from = "from";
	const kwstring where = "where";
	const kwstring _and = "and";
	const kwstring _or = "or";
	const kwstring _xor = "xor";
	const kwstring on = "on";
	const kwstring update = "update";
	const kwstring set = "set";
	const kwstring _delete = "delete";
	const kwstring integer = "integer";
	const kwstring _float = "float";
	const kwstring text = "text";

	const kwstring variable = "variable";
}
const vector<kwstring> g_Keywords = {							// 关键字列表（纯小写）
	keywords::create,	keywords::drop,		keywords::database,		keywords::use,
	keywords::table,	keywords::insert,	keywords::into,			keywords::inner,
	keywords::join,		keywords::values,	keywords::select,		keywords::from,
	keywords::where,	keywords::_and,		keywords::_or,			keywords::_xor,
	keywords::on,		keywords::update,	keywords::set,			keywords::_delete,
	keywords::integer,	keywords::_float,	keywords::text
};
enum class keyword_index {						// 关键字枚举类型，注意必须与g_Keywords顺序完全一致（最后两个除外）
	create,		drop,		database,	use,
	table,		insert,		into,		inner,
	join,		values,		select,		from,
	where,		_and,		_or,		_xor,
	on,			update,		set,		_delete,
	integer,	_float,		text,
	unexpected = -1,
	newline = -2
};

ostream& operator<< (ostream&, const keyword_index);		// 重载ostream左移运算符实现自定义类型输出
bool operator== (const string, const kwstring);
bool operator!= (const string, const kwstring);
keyword_index getKeywordIndex(const string);				// 将string类型的关键字转化为keyword_name类型




// 函数体定义全部写在下方

template <typename T> bool doesContain(const T check_value, const vector<T> value_list) {
	for (T value : value_list) {
		if (check_value == value) return true;
	}
	return false;
}
bool doesContain(const char ch, const string str) {
	for (char c : str) {
		if (ch == c) return true;
	}
	return false;
}
bool isToken(const string str) {
	const vstring nontokens = {
		"(",	")",	",",	
	};
	if (doesContain(str, nontokens)) return false;
	return true;
}
string trim(const string str) {
	auto lpos = str.find_first_not_of(" \t\n");
	auto rpos = str.find_last_not_of(" \t\n");

	if (lpos == string::npos) return "";
	return str.substr(lpos,rpos+1);
}
string trimDuplicateWs(const string str) {
	bool f_isPrevWs = false;
	bool f_isInStr = false;
	string res;
	for (char ch : str) {
		if (ch == '\'') {
			f_isInStr = !f_isInStr;
		}
		if (f_isInStr) {
			res.push_back(ch);
			continue;
		}
		if (doesContain(ch, g_Whitespaces)) {
			if (!f_isPrevWs) {
				res.push_back(' ');
				f_isPrevWs = true;
			}
		}
		else {
			res.push_back(ch);
			f_isPrevWs = false;
		}
	}
	return trim(res);
}
void checkStrValidity(const vstring strs) {
	for (string str : strs) {
		auto pos = str.find('\'');
		if (pos == string::npos) {
			// 没有单引号，说明压根不是字符串
			continue;
		}
		if (pos != 0) {
			// 有单引号但不是在开头，必然为非法输入
			throw InvalidArgument(i18n::parseKey("strunexptprfx", {str.substr(0,pos),str.substr(pos+1)}));
		}
		auto rpos = str.rfind('\'');
		if (rpos == pos) {
			// 正反找相同，说明总共只有一个单引号，则字符串不完整（结尾未闭合）
			throw InvalidArgument(i18n::parseKey("incmpltstr"));
		}
		if (rpos != str.size()-1) {
			// 找到另一个单引号却不在开头，则字符串结束后仍有其他内容
			throw InvalidArgument(i18n::parseKey("strunexptsufx", {str.substr(rpos+1),str.substr(1,rpos-1)}));
		}
		auto mpos = str.find('\'', 1);
		if (mpos != rpos) {
			// 在开头之后找到其他单引号，可能是结尾的也可能是其他的。如果不是结尾，那就只能是其他的。
			// 由于任务要求不包括对转义符的处理，这里不会把双单引号转义（''）处理为文本内单引号（'）。
			// 也因此认为：除开头结尾以外，字符串在其余任何位置出现单引号，都被认为是非法输入。
			throw InvalidArgument(i18n::parseKey("strunexptsquote", {itos(mpos-1), str}));
		}
	}
}
vstring splitParameters(const string res, const string delim) {
	vstring tokens;
	string token;
	bool f_isInStr = false;
	for (char ch : res+delim.at(0)) {			// 逐字符遍历字符串，追加一个字符的目的是保证最后一个token正常处理
		switch (ch) {
			case '\'':
				f_isInStr = !f_isInStr;			// 遇到单引号就切换“是否在字符串内”的状态
				token.push_back('\'');
				break;
			default:
				if (f_isInStr) {
					token.push_back(ch);		// 在字符串里的字符原样保留
					break;
				}
											// 不在字符串里的字符
				if (doesContain(ch, delim)) {	// 读取到分割符时结束当前字符串并推入tokens
					if (token != "") tokens.push_back(token);			// 正是因为要读到分割符才推入，所以遍历开始前在字符串末尾追加了一个分割符
					token = "";
					break;
				}
				token.push_back(ch);			// 原样保留不是分割符的字符
				break;
		} 
	}
	checkStrValidity(tokens);
	return tokens;
}
vstring splitParameters(const string res, const char delim) {
	return splitParameters(res, string(1, delim));
}
bool isReservedKeyword(const string str) {
	if (str == "") return false;
	for (kwstring keyword_str : g_Keywords) if (keyword_str == str) return true;
	return false;
}
bool doesFitNameRequirement(const string str) {
	if (str == "") return false;
	string allowed_chars = "_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	if (!doesContain(str.at(0), allowed_chars)) return false;
	allowed_chars.append("0123456789");
	for (char ch : str.substr(1)) {
		if (!doesContain(ch, allowed_chars)) return false;
	}
	return true;
}
bool isAcceptableName(const string r) {
	return (!isReservedKeyword(r) and doesFitNameRequirement(r));
}
istream& getsUntil(istream& is, string& str, const string delim) {
	const int dsize = delim.size();
	string res;
	for (int i = 1; !is.fail(); ++i) {
		char ch;
		is >> ch;
		res.push_back(ch);
		if (i < dsize) continue;
		if (res.substr(i-dsize) == delim) {
			str = trimDuplicateWs(res.substr(0, i-dsize));
			return is;
		}
	}
	str = trimDuplicateWs(res);
	return is;
}
ostream& operator<< (ostream& os, const keyword_index kw) {
	return os << g_Keywords.at(static_cast<int>(kw)).str();
}
bool operator== (const string s, const kwstring kws) {
	return (kws == s);
}
bool operator!= (const string s, const kwstring kws) {
	return !(s == kws);
}
keyword_index getKeywordIndex(const string str) {
	if (str == symbols::newl) return keyword_index::newline;
	int i = 0;
	for (kwstring kws : g_Keywords) {
		if (kws == str) return static_cast<keyword_index>(i);
		++i;
	}
	return keyword_index::unexpected;
}

bool isValidCmpOp(const string s) {
	return (	s == symbols::less
			or	s == symbols::equals
			or	s == symbols::greater
			or	s == symbols::neq
			);
}
bool isValidLogicOp(const string s) {
	return (	s == keywords::_and
			or	s == keywords::_or
			or	s == keywords::_xor
			);
}

}
#endif