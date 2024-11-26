/**
 * 头文件：auxiliaries.h
 * 此头文件的内容是MiniDB底层的辅助函数和辅助类。
 */
#ifndef __BASICS__MINIDB_H__
#define __BASICS__MINIDB_H__

#include "exceptions.h"

namespace minidb {

// 计数器空间，这个计数器实际上就是用于计算行号的
/** 原理：
 *  从头到尾读取文件的时候累计token个数，并记录每个换行符出现时是第几个token。
 *  报错时，由于所有错误都从问题发生处直接throw异常直至返回__Main，后续未处理内容不会再对token总数产生影响。
 *  此时token总数大于第n-1行小于第n行，则它位于第n行。
 */
class TokenCounter {
	private:
		int count;								// token总数
		vector<int> lnpos;						// 某行结束时的总token数，注意lnpos.at(1)是第一行的行号

	public:										// 一些比较简单的函数我就不拎出去写了
		TokenCounter() { count = 0; lnpos = {}; }					// 构造函数
		void increment() { ++count; }							// token总数自增
		void clearAll() { lnpos = {}; count = 0; }				// 清空所有内容
		int tokens() { return count; }							// 返回总token个数
		string where(int);										// 返回行号文本
		string where() { return where(tokens()); }
		void newl() { lnpos.push_back(tokens()); }				// 向lnpos中添加新行的位置

} g_LnCounter;											// 只声明这一个对象就足够用了

void standardizeInputFile(ifstream&);

namespace symbols {								
	const string newl = "\\newl";				// 换行标志
	const string next = ",";					// 参数分隔标志
	const string paramsbegin = "(";				// 左括号
	const string paramsend = ")";				// 右括号
	const string cmdend = ";";					// 语句结束标志
	const string less = "<";					// 小于
	const string greater = ">";					// 大于
	const string equals = "=";					// 等于
	const string assigns = "=";					// 赋值
	const string plus = "+";					// 加
	const string minus = "-";					// 减
	const string times = "*";					// 乘
	const string fwildcard = "*";				// 全体通配符
	const string divides = "/";					// 除
	const string mods = "%";					// 取模
	const string neq = "!=";					// 不等于
	const map<char, string> grmsymbols = {		// 记录了所有语法中可能包含的符号
		{';', cmdend},		{',', next},		{'(', paramsbegin},
		{')', paramsend},	{'<', less},		{'>', greater},
		{'=', equals},		{'+', plus},		{'-', minus},
		{'*', times},		{'/', divides},		{'%', mods},
		{'\n', newl}
	};
}

int stringToInt(const string);
double stringToDouble(const string);


// 函数体定义大部分写在下方


int stringToInt (const string s) {
	stringstream ss(s);
	int i;
	ss >> i;
	return i;
}

double stringToDouble (const string s) {
	stringstream ss(s);
	double d;
	ss >> d;
	return d;
}

// 输出行号
string TokenCounter::where(int n) {
	int size = lnpos.size();
	stringstream ss;
	if (size == 0) ss << i18n::parseKey("notinfile");
	else {
		size_t ln = 1;
		while (ln < lnpos.size() and n > lnpos.at(ln)) {
			++ln;
		}
		ss << i18n::parseKey("lnno",{itos(ln)});
	}
	return ss.str();
}

// 标准化输入文件
// 实际上就是在符号的两侧添加一个空格，利用空格来进行token拆分
void standardizeInputFile(ifstream& ifile) {
	g_LnCounter.clearAll();
	ofstream ofile;
	ofile.open(g_TempFileName.c_str(), ios::out);
	if (!ofile.is_open()) {
		throw FailedFileOperation(i18n::parseKey("opentmpf",{g_TempFileName}));
	}
	bool f_isInStr = false;		// 对字符串做特判
	bool f_isNeq = false;		// Project要求的唯一非单字符符号是“!=”，对其做特判
	while (true) {
		char ch;
		ifile >> noskipws >> ch;	// noskipws保证读入空白字符，否则没法分割token也没法计算行号
		if (ifile.eof()) break;
		if (f_isInStr) {
			if (ch == '\'') {
				f_isInStr = false;
			}
			else if (ch == '\n') {
				// 字符串内换行说明字符串不闭合
				throw InvalidArgument(i18n::parseKey("incmpltstr"));
			}
			ofile << ch;
			continue;
		}
		if (f_isNeq) {
			f_isNeq = false;
			if (ch == '=') {
				ofile << ' ' << symbols::neq << ' ';
				continue;
			}
			else {
				ofile << " ! ";
			}
		}
		switch (ch) {
			case '\n':
				ofile << " " << symbols::newl << " ";
				g_LnCounter.newl();
				break;
			case '\'':
				ofile << '\'';
				f_isInStr = true;
				break;
			case '!':
				f_isNeq = true;
				break;
			default:
				do {	// 套do-while (false); 的目的是在switch-case块内部声明变量
					auto it = symbols::grmsymbols.find(ch);
					if (it != symbols::grmsymbols.end()) {
						ofile << ' ' << it->second << ' ';
					}
					else ofile << ch;
				} while (false);
		}
	}
	ifile.close();
	ofile.close();
	ifile.open(g_TempFileName, ios::in);		// 原本ifile打开的是指定的输入文件，但从这里开始，打开的是规整化后的临时文件
	if (!ifile.is_open()) {
		throw FailedFileOperation(i18n::parseKey("opentmpf",{g_TempFileName}));
	}
	g_LnCounter.clearAll();
}


}
#endif