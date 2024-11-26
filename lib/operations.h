/**
 * 头文件：operations.h
 * 此头文件的内容是MiniDB实际执行命令的函数。
 */
#ifndef __OPERATIONS_MINIDB_H__
#define __OPERATIONS_MINIDB_H__

#include "objects.h"

namespace minidb {

bool gf_isFirst = true;

void runStCreateDatabase(const vstring);
void runStUseDatabase(const vstring);
void runStCreateTable(const vstring);
void runStDropTable(const vstring);
void runStInsertion(const vstring);
void runStInnerJoin(const vstring, ostream&);
void runStSelection(const vstring, ostream&);
void runStUpdate(const vstring);
void runStDeleteFrom(const vstring);

void runStDeleteFrom(const vstring params) {

}
void runStUpdate(const vstring params) {
	Database& database = getCurrentDatabase();
	
	string table_name = params.at(0);
	Table& table = database.findTable(table_name);

	vstring variable_names, values, conditions;
	auto it = params.begin()+1;
	for (int i = 1; it != params.end() and *it != keywords::where; ++it, ++i) {
		if (i & 1) variable_names.push_back(*it);
		else values.push_back(*it);
	}
	if (it != params.end()) {
		for (; it != params.end(); ++it) {
			conditions.push_back(*it);
		}
	}

	for (Row& row : table.getRaw()) {
		for (size_t i = 0; i < variable_names.size(); ++i) {
			string str = variable_names.at(i);
			Term& term = row.findTerm(str);
			term.setValue(values.at(i));
		}
	}
}
void runStInnerJoin(const vstring params, ostream& os) {
	Database& database = getCurrentDatabase();
	
	vstring columns;
	string tabn_first, tabn_second;
	vstring join_terms;
	
	int stage = 0;
	for (string str : params) {
		if (str == keywords::from) {
			stage = 1;
			continue;
		}
		if (str == "inner join") {
			stage = 2;
			continue;
		}
		switch(stage) {
			case 0:
				columns.push_back(str);
				break;
			case 1:
				tabn_first = str;
				break;
			case 2:
				tabn_second = str;
				stage = 3;
				break;
			case 3:
				join_terms.push_back(str);
				break;
		}
	}

	// 检查表名的合法性
	if (tabn_first == tabn_second) {
		throw InvalidArgument(i18n::parseKey("invdupc", {tabn_first}));
	}

	string jtabn_first = join_terms.at(0), jtabn_second = join_terms.at(2);

	if (jtabn_first != tabn_first and jtabn_first != tabn_second) {
		string expt = "\"" + tabn_first + "\" or \"" + tabn_second + "\"";
		throw InvalidArgument(i18n::parseKey("exptsthgotothers", {expt, jtabn_first}));
	}
	if (jtabn_second != tabn_first and jtabn_second != tabn_second) {
		string expt = "\"" + tabn_first + "\" or \"" + tabn_second + "\"";
		throw InvalidArgument(i18n::parseKey("exptsthgotothers", {expt, jtabn_second}));
	}
	if (jtabn_first == jtabn_second) {
		if (jtabn_first == tabn_first) {
			throw InvalidArgument(i18n::parseKey("dupclunre", {tabn_first, tabn_second}));
		}
		else {
			throw InvalidArgument(i18n::parseKey("dupclunre", {tabn_second, tabn_first}));
		}
	}

	// 找表
	Table& table_first = database.findTable(jtabn_first);
	Table& table_second = database.findTable(jtabn_second);
	
	// 检查列名，整理结果的标题行
	Row title;
	
	vstring tabn, coln;

	for (string str : columns) {
		auto pos = str.find('.');
		if (pos == string::npos) {
			throw InvalidArgument(i18n::parseKey("nmemspec", {str}));
		}
		string table_name = str.substr(0,pos);
		string column_name = str.substr(pos+1);
		
		if (table_name != tabn_first and table_name != tabn_second) {
			string expt = "\"" + tabn_first + "\" or \"" + tabn_second + "\"";
			throw InvalidArgument(i18n::parseKey("exptsthgotothers", {expt, table_name}));
		}
		if (!isAcceptableName(table_name)) {
			throw InvalidArgument(i18n::parseKey("unacptvarn", {table_name}));
		}
		if (!isAcceptableName(column_name) and column_name != symbols::fwildcard) {
			throw InvalidArgument(i18n::parseKey("unacptvarn", {column_name}));
		}

		tabn.push_back(table_name);
		coln.push_back(column_name);
	}

	for (int i = 0, size = tabn.size(); i < size ; ++i) {
		for (int j = i + 1; j < size ; ++j) {
			if (coln.at(i) == coln.at(j)) {
				throw InvalidArgument(i18n::parseKey("dupselterm", {coln.at(i)}));
			}
		}
	}

	for (int i = 0, size = tabn.size(); i < size ; ++i) {
		
		string table_name = tabn.at(i);
		string column_name = coln.at(i);

		if (column_name == symbols::fwildcard) {
			tabn.erase(tabn.begin() + i);
			coln.erase(coln.begin() + i);
			
			if (doesContain(table_name, tabn)) {
				throw InvalidArgument(i18n::parseKey("dupselwildc"));
			}

			// 展开通配符
			for (psterm p_term : database.findTable(table_name).getTitle().getRaw()) {
				tabn.push_back(table_name);
				coln.push_back(p_term.first);
			}
		}
		else {
			// 检查是否确实存在此项，存在则不会抛异常
			Term temp = database.findTable(table_name).getTitle().findTerm(column_name);
			// 插入一个项到标题行。注意：本项目认为结果表中存在同名列也属于错误。
			title.insertTerm(column_name, temp);
		}
		
	}

	// 结果表
	Table result(title);

	string jcoln_first = join_terms.at(1), jcoln_second = join_terms.at(3);

	for (Row row_first : table_first.getRaw()) {
		for (Row row_second : table_second.getRaw()) {
			if (row_first.findTerm(jcoln_first) == row_second.findTerm(jcoln_second)) {
				Row temp = title;
				temp.mergeRowIntersect(row_first).mergeRowIntersect(row_second);
				result.insertRow(temp);
			}
		}
	}

	// 输出
	// 判别是否为第一次输出
	if (gf_isFirst) gf_isFirst = false;
	else os << "---" << endl;

	result.print(os);
}
void runStSelection(const vstring params, ostream& os) {
	Database& database = getCurrentDatabase();

	vstring targets;
	string table_name;
	vstring conditions;

	bool f_isTargets = true;

	for (auto it = params.begin(); it != params.end(); ++it) {
		if (*it == keywords::from) {
			f_isTargets = false;
			++it;
			table_name = *it;
			continue;
		}
		if (f_isTargets) targets.push_back(*it);
		else conditions.push_back(*it);
	}

	Table& table = database.findTable(table_name);
	
	// 通配符检查
	if (doesContain(symbols::fwildcard, targets)) {
		// 通配符和变量名混用时直接报错
		if (targets.size() != 1) throw InvalidArgument(i18n::parseKey("dupselwildc"));
		else { 
		// 否则将通配符替换为所有项目
			vstring temp;
			msterm term = table.getTitle().getRaw();
			for (psterm p_term : term) {
				temp.push_back(p_term.first);
			}
			targets = temp;
		}
	}
	else {
		for (auto it = targets.begin(); it != targets.end(); ++it) {
			for (auto jt = it+1; jt != targets.end(); ++jt) {
				if (*it == *jt)	throw InvalidArgument(i18n::parseKey("dupselterm", {*it}));
			}
		}
	}

	Row title;
	for (string str : targets) {
		title.insertTerm(str, Term());
	}

	//结果表
	Table result(title);

	for (Row row : table.getRaw()) {
		Row row_temp;
		for (psterm p_term : row.getRaw()) {
			string table_name = p_term.first;
			Term term = p_term.second;
			if (doesContain(table_name, targets)) row_temp.insertTerm(p_term);
		}
		result.insertRow(row_temp);
	}

	// 判别是否为第一次输出
	if (gf_isFirst) gf_isFirst = false;
	else os << "---" << endl;

	result.print(os);
}
void runStInsertion(const vstring params) {
	Database& database = getCurrentDatabase();
	Table& table = database.findTable(params.at(0));

	Row row = table.getTitle();
	if (row.size() != params.size()-1) {
		throw ArgumentCountError(row.size(), params.size(), i18n::parseKey("upp"));
	}
	int i = 1;
	msterm terms = row.getRaw();
	for (auto it = terms.begin(); it != terms.end(); ++it) {
		it->second.setValue(params.at(i));
		++i;
	}
	row.setTerms(terms);
	table.insertRow(row);
}
void runStDropTable(const vstring params) {
	Database& database = getCurrentDatabase();
	database.dropTable(params.at(0));
}
void runStCreateTable(const vstring params) {
	Database& database = getCurrentDatabase();
	Row title;
	for (auto it = params.begin()+1; it != params.end(); ++it) {
		Term term;
		string term_name = *it;
		++it;
		term.setType(*it);
		title.insertTerm(term_name, term);
	}
	database.insertTable(params.at(0), Table(title));
}
void runStUseDatabase(const vstring params) {
	useDatabase(params.at(0));
}
void runStCreateDatabase(const vstring params) {
	createDatabase(params.at(0));
}

}

#endif