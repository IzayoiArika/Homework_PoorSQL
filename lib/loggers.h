/**
 * 头文件：loggers.h
 * 仅在定义了宏__DEBUG_ENVIRONMENT__时被调用。
 */

#ifndef __LOGGER_MINIDB_H__
#define __LOGGER_MINIDB_H__

#include "exceptions.h"

namespace minidb {

void logCreateDatabase(const vstring);
void logCreateTable(const vstring);
void logUseDatabase(const vstring);
void logDropTable(const vstring);
void logInsertion(const vstring);
void logSelection(const vstring);
void logInnerJoin(const vstring);
void logUpdate(const vstring);
void logDeleteFrom(const vstring);
void logNullStm();
void logWhere(const vstring);

// 函数体定义全部写在下方
void logCreateDatabase(const vstring params) {
	clog << i18n::parseKey("l_createdb",{params.at(0)}) << endl;
}
void logCreateTable(const vstring params) {
	clog << i18n::parseKey("l_createtab",{params.at(0)}) << endl;
	bool f_isTermName = true;
	for (auto it = params.begin()+1; it != params.end(); ++it) {
		if (f_isTermName) {
			clog << i18n::parseKey("l_createtabparam",{*it, *(it+1)}) << endl;
		}
		f_isTermName = !f_isTermName;
	}
}
void logUseDatabase(const vstring params) {
	clog << i18n::parseKey("l_usedb",{params.at(0)}) << endl;
}
void logDropTable(const vstring params) {
	clog << i18n::parseKey("l_droptab",{params.at(0)}) << endl;
}
void logInsertion(const vstring params) {
	clog << i18n::parseKey("l_insertion",{params.at(0)}) << endl;
	for (auto it = params.begin()+1; it != params.end(); ++it) {
		clog << i18n::parseKey("l_insertionval",{*it}) << endl;
	}
}
void logInnerJoin(const vstring params) {
	clog << i18n::parseKey("l_selection") << endl;
	int stage = 0;
	vstring innerjoin_clause;
	for (string str : params) {
		if (str == keywords::from) {
			stage = 1;
			continue;
		}
		if (str == "inner join") {
			stage = 2;
			continue;
		}
		switch (stage) {
			case 0:				// 列名
				clog << i18n::parseKey("l_selectioncol",{str}) << endl;
				break;
			case 1:				// 表名
				clog << i18n::parseKey("l_intab", {str}) << endl;
				break;
			case 2:				// 另一个表名
				innerjoin_clause.push_back(str);
				break;
		}
	}
	clog << i18n::parseKey("l_intab", {innerjoin_clause.at(0)}) << endl;
	clog << i18n::parseKey("l_innerjoin", {innerjoin_clause.at(1) + "." + innerjoin_clause.at(2), innerjoin_clause.at(3) + "." + innerjoin_clause.at(4)});
	clog << endl;
}
void logSelection(const vstring params) {
	clog << i18n::parseKey("l_selection") << endl;
	int stage = 0;
	vstring where_clause;
	for (string str : params) {
		if (str == keywords::from) {
			stage = 1;
			continue;
		}
		if (str == keywords::where) {
			stage = 2;
			continue;
		}
		switch (stage) {
			case 0:				// 列名
				clog << i18n::parseKey("l_selectioncol",{str}) << endl;
				break;
			case 1:				// 表名
				clog << i18n::parseKey("l_intab", {str}) << endl;
				break;
			case 2:				// where clause
				where_clause.push_back(str);
		}
	}
	logWhere(where_clause);
}
void logUpdate(const vstring params) {
	clog << i18n::parseKey("l_update") << endl;
	int stage = 0;
	vstring where_clause;
	string asgn_str = "";
	for (string str : params) {
		if (str == keywords::set) {
			stage = 1;
			continue;
		}
		if (str == symbols::next) {
			clog << i18n::parseKey("l_assignment", {asgn_str}) << endl;
			asgn_str = "";
			continue;
		}
		if (str == keywords::where) {
			clog << i18n::parseKey("l_assignment", {asgn_str}) << endl;
			asgn_str = "";
			stage = 2;
			continue;
		}
		switch (stage) {
			case 0:				// 表名
				clog << i18n::parseKey("l_intab", {str}) << endl;
				break;
			case 1:				// 赋值表达式
				asgn_str = asgn_str + str + " ";
				break;
			case 2:				// where clause
				where_clause.push_back(str);
		}
	}
	logWhere(where_clause);
}
void logDeleteFrom(const vstring params) {
	clog << i18n::parseKey("l_deletefrom", {params.at(0)}) << endl;
	vstring conditions = params;
	conditions.erase(conditions.begin());
	logWhere(conditions);
}
void logNullStm() {
	clog << i18n::parseKey("w_nullstm") << endl;
}
void logWhere(const vstring params) {	
	auto it = params.begin();
	if (it != params.end()) {
		clog << i18n::parseKey("l_where") << endl;
		while (true) {
			clog << i18n::parseKey("l_logicexpr", {*(it++), *(it++), *(it++)});
			if (it != params.end()) {
				clog << ' ' << (*(it++)) << endl;
				continue;
			}
			break;
		}
		clog << endl;
	}
}

}

#endif