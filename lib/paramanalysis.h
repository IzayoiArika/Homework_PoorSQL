/**
 * 头文件：paramsanalysis.h
 * 此头文件的内容是MiniDB分析参数组成的函数。
 */
#ifndef __PARAM_ANALYSIS_MINIDB_H__
#define __PARAM_ANALYSIS_MINIDB_H__

#include "stringop.h"

namespace minidb {

enum cmd_type {				// SQL语句的种类
	createdb,	createtab,	usedb,		droptab,
	insertion,	selection,	update,		delfrom,
	innerjoin, 
	null = -1
};
// 这里单独把inner join拎出来特判

void eraseNewlFront(vstring&);							// 去除参数列表前方的新行
void eraseNewlBack(vstring&);							// 去除参数列表后方的新行
int cntAvailableArgs(const vstring);					// 检查参数列表中有多少有效的参数（也即不计"\newline"的总数）

cmd_type parseCreateStParams(vstring&);			// 解析并检查	create	开头语句的参数
cmd_type parseUseStParams(vstring&);			// 解析并检查	use		开头语句的参数
cmd_type parseDropStParams(vstring&);			// 解析并检查	drop	开头语句的参数
cmd_type parseInsertStParams(vstring&);			// 解析并检查	insert	开头语句的参数
cmd_type ParssDeletionStParams(vstring&);		// 解析并检查	delete	开头语句的参数
cmd_type parseSelectStParams(vstring&);			// 解析并检查	select	开头语句的参数

void parseCreateDatabaseParams(vstring&);		// 解析并检查	create database			语句的参数
void parseCreateTableParams(vstring&);			// 解析并检查	create table			语句的参数
void parseUseDatabaseParams(vstring&);			// 解析并检查	use database			语句的参数
void parseDropTableParams(vstring&);			// 解析并检查	drop table				语句的参数
void parseInsertIntoParams(vstring&);			// 解析并检查	insert into				语句的参数
void parseDeleteFromParams(vstring&);			// 解析并检查	delete from				语句的参数
cmd_type parseUpdateParams(vstring&);			// 解析并检查	update					语句的参数

void parseSelectionMainParams(vstring&);		// 解析并检查	select ... 	主句的参数
void parseSelectionJoinParams(vstring&);		// 呃……这很难解释，总之有用

void parseInnerJoinParams(vstring&);			// 解析并检查	inner join	从句的参数
void parseWhereClauseParams(vstring&);			// 解析并检查	where		从句的参数




void parseWhereClauseParams(vstring& params) {
	vstring res;
	eraseNewlFront(params);
	int size = cntAvailableArgs(params);
	if (size % 4 != 3) {
		throw ArgumentCountError(size/4*4+3, size, i18n::parseKey("incmpltparamlist"));
	}
	int i = 0;
	while (true) {
		eraseNewlFront(params);
		g_LnCounter.increment();
		if (params.size() == 0)	break;
		string current_str = params.at(0);
		switch (i % 4) {
			case 0:
				if (!isAcceptableName(current_str)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {current_str}));
				}
				break;
			case 1:
				if (!isValidCmpOp(current_str)) {
					throw SyntaxError(i18n::parseKey("invalidcmpop", {current_str}));
				}
				break;
			case 2:	break;
			case 3:
				if (getKeywordIndex(current_str) != keyword_name::_and and getKeywordIndex(current_str) != keyword_name::_or) {
					throw SyntaxError(i18n::parseKey("invalidlgop", {current_str}));
				}
				break;
		}
		res.push_back(current_str);
		params.erase(params.begin());
		++i;
	}
	params = res;
	eraseNewlBack(params);
}
void parseDeleteFromParams(vstring& params) {
	eraseNewlFront(params);
	vstring res;
	int stage = 0;				// 解析阶段标记
	string current_str;
	while (params.size() != 0) {
		eraseNewlFront(params);
		current_str = params.at(0);
		if (stage == 2) {
			parseWhereClauseParams(params);
			break;
		}
		switch (stage) {
			case 0:							// 读取表名
				g_LnCounter.increment();
				if (!isAcceptableName(current_str)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {current_str}));
				}
				res.push_back(current_str);
				stage = 1;
				break;
			case 1:							// 读取where
				g_LnCounter.increment();
				if (getKeywordIndex(current_str) != keyword_name::where) {
					throw SyntaxError(i18n::parseKey("unexptstr", {current_str}));
				}
				res.push_back(keywords::where);
				stage = 2;
				break;
		}
		params.erase(params.begin());
	}
	switch (stage) {
		case 0:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_tablename").prints()}));
		case 1:		throw SyntaxError(i18n::parseKey("exptwheregotnil"));
		case 2:		break;
	}
	params = res;
	eraseNewlBack(params);
}
cmd_type parseDeletionStParams(vstring& params) {
	eraseNewlFront(params);
	if (params.size() == 0) throw SyntaxError(i18n::parseKey("exptkwgotnil", {keywords::from}));
	g_LnCounter.increment();
	switch (getKeywordIndex(params.at(0))) {
		case keyword_name::from:
			params.erase(params.begin());		// 用于删去开头的"from"
			parseDeleteFromParams(params);
			return cmd_type::delfrom;
		default:
			throw SyntaxError(i18n::parseKey("unexptstr", {params.at(0)}));
	}
}
cmd_type parseUpdateParams(vstring& params) {
	eraseNewlFront(params);
	vstring res;
	int stage = 0;
	string now;
	while (params.size() != 0) {
		eraseNewlFront(params);
		now = params.at(0);
		if (stage == 6) {
			parseWhereClauseParams(params);
			break;
		}
		if (equalIgnoringCase(now, keywords::set)) {
			// 如果set没有出现在读取from的阶段（stage 1）则一定语法错误
			if (stage != 1) {
				throw SyntaxError(i18n::parseKey("unexptkw", {keywords::from}));
			}
		}
		switch (stage) {
			case 0:							// 读取表名
				g_LnCounter.increment();
				if (!isAcceptableName(now)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {now}));
				}
				res.push_back(now);
				stage = 1;
				break;
			case 1:							// 读取"set"
				if (now != keywords::set) {
					throw SyntaxError(i18n::parseKey("unexptstr", {now}));
				}
				stage = 2;
				break;
			case 2:							// 读取变量名
				g_LnCounter.increment();
				if (!isAcceptableName(now)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {now}));
				}
				res.push_back(now);
				stage = 3;
				break;
			case 3:							// 赋值符号"="
				if (now != symbols::assigns) {
					throw SyntaxError(i18n::parseKey("unexptstr", {now}));
				}
				stage = 4;
				break;
			case 4:							// 读取值
				g_LnCounter.increment();
				res.push_back(now);
				stage = 5;
				break;
			case 5:							// ","或"where"
				if (equalIgnoringCase(now, keywords::where)) stage = 6;
				else if (now == symbols::next) stage = 2;
				else throw SyntaxError(i18n::parseKey("unexptstr", {now}));
				break;
		}
		params.erase(params.begin());
	}
	switch (stage) {
		case 0:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_tablename").prints()}));
		case 1:		throw SyntaxError(i18n::parseKey("exptkwgotnil", {keywords::set}));
		case 2:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
		case 3:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {"\"=\""}));
		case 4:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termvalue").prints()}));
		case 5:		break;	// 不含where clause时从此处跳出
		case 6:		// 含where clause时从此处跳出,需要把params内容推入temp再复制回去
			if (params.size() == 0) throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
			res.push_back(keywords::where);
			for (string s : params) res.push_back(s);
			break;	
	}
	params = res;
	eraseNewlBack(params);
	return cmd_type::update;
}
cmd_type parseSelectStParams(vstring& params) {
	cmd_type type = cmd_type::selection;
	vstring main_clause;
	string type_str = "";
	vstring append_clause;
	bool f_isAppendClause = false;
	bool f_isInnerJoin = false;
	for (string str : params) {
		if (str == keywords::where) {
			f_isAppendClause = true;
			type_str = "where";
			continue;
		}
		else if (str == keywords::inner) {
			f_isInnerJoin = true;
			continue;
		}

		if (f_isInnerJoin and (!f_isAppendClause)) {
			if (str != keywords::join) {
				throw SyntaxError(i18n::parseKey("exptkwgotothers", {keywords::join, str}));
			}
			f_isAppendClause = true;
			type_str = "inner join";
			type = cmd_type::innerjoin;
			continue;
		}

		if (f_isAppendClause) {
			append_clause.push_back(str);
		}
		else {
			main_clause.push_back(str);
		}
	}
	if (f_isInnerJoin) {
		parseSelectionJoinParams(main_clause);
		parseInnerJoinParams(append_clause);
	}
	else if (append_clause.size() != 0) {
		parseSelectionMainParams(main_clause);
		parseWhereClauseParams(append_clause);
	}

	params = {};
	for (string str : main_clause) {
		params.push_back(str);
	}
	if (type_str != "") {
		params.push_back(type_str);
		for (string str : append_clause) {
			params.push_back(str);
		}
	}
	
	return type;
}

void parseInnerJoinParams(vstring& params) {
	if (params.size() != 5) {
		throw ArgumentCountError(5, params.size(), i18n::parseKey("upp"));
	}
	if (params.at(1) != keywords::on) {
		throw SyntaxError(i18n::parseKey("exptkwgotothers", {keywords::on, params.at(1)}));
	}
	if (params.at(3) != symbols::equals) {
		throw SyntaxError(i18n::parseKey("exptkwgotothers", {symbols::equals, params.at(3)}));
	}
	const string innerjoin_name = params.at(0);

	string match_first = params.at(2), match_second = params.at(4);

	auto pos_first = match_first.find('.'), pos_second = match_second.find('.');
	if (pos_first == string::npos) {
		throw InvalidArgument(i18n::parseKey("nmemspec", {match_first}));
	}
	if (pos_second == string::npos) {
		throw InvalidArgument(i18n::parseKey("nmemspec", {match_second}));
	}

	string table_first = match_first.substr(0,pos_first);
	string table_second = match_second.substr(0,pos_second);
	string column_first = match_first.substr(pos_first+1);
	string column_second = match_second.substr(pos_second+1);

	if (!isAcceptableName(innerjoin_name)) {
		throw InvalidArgument(i18n::parseKey("unacptvarn", {innerjoin_name}));
	}
	if (!isAcceptableName(table_first)) {
		throw InvalidArgument(i18n::parseKey("unacptvarn", {table_first}));
	}
	if (!isAcceptableName(table_second)) {
		throw InvalidArgument(i18n::parseKey("unacptvarn", {table_second}));
	}
	if (!isAcceptableName(column_first)) {
		throw InvalidArgument(i18n::parseKey("unacptvarn", {column_first}));
	}
	if (!isAcceptableName(column_second)) {
		throw InvalidArgument(i18n::parseKey("unacptvarn", {column_second}));
	}

	params = {
		innerjoin_name, table_first, column_first, table_second, column_second
	};
}

void parseSelectionMainParams(vstring& params) {
	eraseNewlFront(params);
	vstring res;
	int stage = 0;
	string now;
	while (params.size() != 0) {
		eraseNewlFront(params);
		now = params.at(0);
		if (now == symbols::next) {
			// 如果\next没有出现在读取\next的阶段（stage 1）则一定语法错误
			if (stage != 1) throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
		}
		if (equalIgnoringCase(now, keywords::from)) {
			// 如果from没有出现在读取from的阶段（stage 1）则一定语法错误
			if (stage != 1) {
				throw SyntaxError(i18n::parseKey("unexptkw", {"from"}));
			}
			stage = 2;
		}
		switch (stage) {
			case 0:							// 读取列名（通配符\times也是合理的列名）
				g_LnCounter.increment();
				if (!isAcceptableName(now) and now != symbols::times) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {now}));
				}
				res.push_back(now);
				stage = 1;
				break;
			case 1:							// 读取\next或"from"
				if (now != symbols::next) {
					throw SyntaxError(i18n::parseKey("unexptstr", {now}));
				}
				stage = 0;
				break;
			case 2:							// 处理"from"
				g_LnCounter.increment();
				res.push_back(keywords::from);
				stage = 3;
				break;
			case 3:							// 读取表名
				g_LnCounter.increment();
				if (!isAcceptableName(now)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {now}));
				}
				res.push_back(now);
				stage = 4;
				break;
		}
		params.erase(params.begin());
	}
	switch (stage) {
		case 0:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
		case 1:		throw SyntaxError(i18n::parseKey("redundant"));
		case 2:		throw exception();
		case 3:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_tablename").prints()}));
		case 4:		break;
	}
	params = res;
	eraseNewlBack(params);
}

void parseSelectionJoinParams(vstring& params) {
	eraseNewlFront(params);
	vstring res;
	int stage = 0;
	string now;
	while (params.size() != 0) {
		eraseNewlFront(params);
		now = params.at(0);
		if (now == symbols::next) {
			// 如果\next没有出现在读取\next的阶段（stage 1）则一定语法错误
			if (stage != 1) throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
		}
		if (equalIgnoringCase(now, keywords::from)) {
			// 如果from没有出现在读取from的阶段（stage 1）则一定语法错误
			if (stage != 1) {
				throw SyntaxError(i18n::parseKey("unexptkw", {"from"}));
			}
			stage = 2;
		}
		switch (stage) {
			case 0:							// 读取列名（通配符\times也是合理的列名）
				do {
					g_LnCounter.increment();
					auto pos = now.find('.');
					if (pos == string::npos) {
						throw InvalidArgument(i18n::parseKey("nmemspec", {now}));
					}
					string table_name = now.substr(0,pos);
					string column_name = now.substr(pos+1);
					
					if (!isAcceptableName(table_name)) {
						throw InvalidArgument(i18n::parseKey("unacptvarn", {table_name}));
					}
					if (!isAcceptableName(column_name) and column_name != symbols::fwildcard) {
						throw InvalidArgument(i18n::parseKey("unacptvarn", {column_name}));
					}
					res.push_back(now);
					stage = 1;
				} while (false);
				break;
			case 1:							// 读取\next或"from"
				if (now != symbols::next) {
					throw SyntaxError(i18n::parseKey("unexptstr", {now}));
				}
				stage = 0;
				break;
			case 2:							// 处理"from"
				g_LnCounter.increment();
				res.push_back(keywords::from);
				stage = 3;
				break;
			case 3:							// 读取表名
				g_LnCounter.increment();
				if (!isAcceptableName(now)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {now}));
				}
				res.push_back(now);
				stage = 4;
				break;
		}
		params.erase(params.begin());
	}
	switch (stage) {
		case 0:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
		case 1:		throw SyntaxError(i18n::parseKey("redundant"));
		case 2:		throw exception();
		case 3:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_tablename").prints()}));
		case 4:		break;
	}
	params = res;
	eraseNewlBack(params);
}

cmd_type parseInsertStParams(vstring& params) {
	eraseNewlFront(params);
	if (params.size() == 0) throw SyntaxError(i18n::parseKey("exptkwgotnil", {keywords::into}));
	g_LnCounter.increment();
	switch (getKeywordIndex(params.at(0))) {
		case keyword_name::into:
			params.erase(params.begin());		// 用于删去开头的"into"
			parseInsertIntoParams(params);
			return cmd_type::insertion;
		default:
			throw SyntaxError(i18n::parseKey("unexptstr", {params.at(0)}));
	}
}
void parseInsertIntoParams(vstring& params) {
	eraseNewlFront(params);
	vstring res;
	int stage = 0;				// 解析阶段标记
	string now;
	bool f_halt = false;
	while (params.size() != 0 and (!f_halt)) {
		eraseNewlFront(params);
		now = params.at(0);
		if (now == symbols::next) {
			// 如果\next没有出现在读取\next的阶段（stage 3）则一定语法错误
			if (stage != 3) throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
		}
		if (now == symbols::paramsend) {
			stage = 5;
			params.erase(params.begin());
			f_halt = true;
			continue;
		}
		switch (stage) {
			case 0:							// 读取表名
				g_LnCounter.increment();
				if (!isAcceptableName(now)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {now}));
				}
				res.push_back(now);
				stage = 1;
				break;
			case 1:							// 必须为values紧跟\paramsbegin
				g_LnCounter.increment();
				if (getKeywordIndex(now) != keyword_name::values) {
					throw SyntaxError(i18n::parseKey("exptkwgotothers", {"values", now}));
				}
				params.erase(params.begin());
				if (params.size() == 0) throw SyntaxError(i18n::parseKey("exptkwgotnil"));
				now = params.at(0);
				if (now != symbols::paramsbegin) {
					throw SyntaxError(i18n::parseKey("exptkwgotothers", {now}));
				}
				stage = 2;
				break;
			case 2:							// 读取参数名
			case 4:							// \next后的等待阶段+重新读取参数名
				g_LnCounter.increment();
				res.push_back(now);
				stage = 3;
				break;
			case 3:							// 读取\next或\paramsend
				if (now == symbols::next) {
					stage = 4;
				}
				else throw SyntaxError(i18n::parseKey("exptsthgotothers", {"',' or ')'", now}));
				break;
		}
		params.erase(params.begin());
	}
	switch (stage) {
		case 0:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_tablename").prints()}));
		case 1:		throw SyntaxError(i18n::parseKey("exptparamsgotnil"));
		case 2:
		case 4:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
		case 5:		break;
		case 3:
		default:	throw SyntaxError(i18n::parseKey("uncloparen"));
	}
	if (params.size() != 0)	throw SyntaxError(i18n::parseKey("exptsthgotothers", {"';'", params.at(0)}));
	params = res;
	eraseNewlBack(params);
}
cmd_type parseDropStParams(vstring& params) {
	eraseNewlFront(params);
	if (params.size() == 0) throw SyntaxError(i18n::parseKey("exptkwgotnil", {keywords::table}));
	g_LnCounter.increment();
	switch (getKeywordIndex(params.at(0))) {
		case keyword_name::table:
			params.erase(params.begin());		// 用于删去开头的"table"
			parseDropTableParams(params);
			return cmd_type::droptab;
		default:
			throw SyntaxError(i18n::parseKey("unexptstr", {params.at(0)}));
	}
}
void parseDropTableParams(vstring& params) {
	eraseNewlFront(params);
	g_LnCounter.increment();
	if (!isAcceptableName(params.at(0))) throw InvalidArgument(i18n::parseKey("unacptvarn", {params.at(0)}));
	eraseNewlBack(params);
	if (params.size() != 1) throw InvalidArgument(i18n::parseKey("unexptstr", {params.at(1)}));
}
cmd_type parseUseStParams(vstring& params) {
	eraseNewlFront(params);
	if (params.size() == 0) throw SyntaxError(i18n::parseKey("exptkwgotnil", {keywords::database}));
	g_LnCounter.increment();
	switch (getKeywordIndex(params.at(0))) {
		case keyword_name::database:
			params.erase(params.begin());		// 用于删去开头的"database"
			parseUseDatabaseParams(params);
			return cmd_type::usedb;
		default:
			throw SyntaxError(i18n::parseKey("unexptstr", {params.at(0)}));
	}
}
void parseUseDatabaseParams(vstring& params) {
	eraseNewlFront(params);
	g_LnCounter.increment();
	if (!isAcceptableName(params.at(0))) throw InvalidArgument(i18n::parseKey("unacptvarn", {params.at(0)}));
	eraseNewlBack(params);
	if (params.size() != 1) throw InvalidArgument(i18n::parseKey("unexptstr", {params.at(1)}));
}
cmd_type parseCreateStParams(vstring& params) {
	eraseNewlFront(params);
	if (params.size() == 0) throw SyntaxError(i18n::parseKey("exptkwgotnil", {keywords::database+"\" or \""+keywords::table}));
	g_LnCounter.increment();
	switch (getKeywordIndex(params.at(0))) {
		case keyword_name::database:
			params.erase(params.begin());		// 用于删去开头的"database"
			parseCreateDatabaseParams(params);
			return cmd_type::createdb;
		case keyword_name::table:
			params.erase(params.begin());		// 用于删去开头的"table"
			parseCreateTableParams(params);
			return cmd_type::createtab;
		default:
			throw SyntaxError(i18n::parseKey("unexptstr", {params.at(0)}));
	}
}
void parseCreateDatabaseParams(vstring& params) {
	eraseNewlFront(params);
	g_LnCounter.increment();
	if (!isAcceptableName(params.at(0))) throw InvalidArgument(i18n::parseKey("unacptvarn", {params.at(0)}));
	eraseNewlBack(params);
	if (params.size() != 1) throw InvalidArgument(i18n::parseKey("unexptstr", {params.at(1)}));
}
void parseCreateTableParams(vstring& params) {
	eraseNewlFront(params);
	vstring res;
	int stage = 0;				// 解析阶段标记
	string now;
	bool f_halt = false;
	while (params.size() != 0 and (!f_halt)) {
		eraseNewlFront(params);
		if (params.size() == 0) break;
		now = params.at(0);
		if (now == symbols::next) {
			// 如果\next没有出现在读取\next的阶段（stage 4）则一定语法错误
			if (stage != 4) throw SyntaxError(i18n::parseKey("incmpltparamlist"));
		}
		if (now == symbols::paramsend) {
			stage = 6;
			f_halt = true;
			continue;
		}
		switch (stage) {
			case 0:							// 读取表名
				g_LnCounter.increment();
				if (!isAcceptableName(now)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {now}));
				}
				res.push_back(now);
				stage = 1;
				break;
			case 1:							// 必须为\paramsbegin
				if (now != symbols::paramsbegin) {
					throw SyntaxError(i18n::parseKey("exptparamsgotothers", {now}));
				}
				stage = 2;
				break;
			case 2:							// 读取参数名
			case 5:							// \next后的等待阶段+重新读取参数名
				g_LnCounter.increment();
				if (!isAcceptableName(now)) {
					throw InvalidArgument(i18n::parseKey("unacptvarn", {now}));
				}
				res.push_back(now);
				stage = 3;
				break;
			case 3:							// 读取参数类型
				g_LnCounter.increment();
				if (equalIgnoringCase(now, keywords::integer))		res.push_back(keywords::integer);
				else if (equalIgnoringCase(now, keywords::_float))	res.push_back(keywords::_float);
				else if (equalIgnoringCase(now, keywords::text))		res.push_back(keywords::text);
				else throw SyntaxError(i18n::parseKey("unacptvt", {now}));
				stage = 4;
				break;
			case 4:							// 读取\next或\paramsend
				if (now == symbols::next) {
					stage = 5;
				}
				else throw SyntaxError(i18n::parseKey("exptsthgotothers", {"',' or ')", now}));
				break;
		}
		params.erase(params.begin());
	}
	switch (stage) {
		case 0:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_tablename").prints()}));
		case 1:		throw SyntaxError(i18n::parseKey("exptparamsgotnil"));
		case 2:
		case 5:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termname").prints()}));
		case 3:		throw SyntaxError(i18n::parseKey("exptsthgotnil", {i18n::parseKey("p_termtype").prints()}));
		case 6:		break;
		case 4:
		default:	throw SyntaxError(i18n::parseKey("uncloparen"));
	}
	params = res;
	eraseNewlBack(params);
}
void eraseNewlFront(vstring& params) {
	while (params.size() != 0 and params.at(0) == symbols::newl) {
		g_LnCounter.newl();
		params.erase(params.begin());
	}
}
void eraseNewlBack(vstring& params) {
	int size = params.size();
	while (size != 0 and params.at(size-1) == symbols::newl) {
		g_LnCounter.newl();
		params.erase(params.end()-1);
		size--;
	}
}
int cntAvailableArgs(const vstring vs) {
	int c = 0;
	for (string s : vs) {
		if (s != symbols::newl) ++c;
	}
	return c;
}


}

#endif