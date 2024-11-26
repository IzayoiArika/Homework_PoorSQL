/**
 * 头文件：objects.h
 * 此头文件的内容是MiniDB的数据结构。
 */
#ifndef __TERM_MINIDB_H__
#define __TERM_MINIDB_H__

#include "stringop.h"

namespace minidb {

enum class logic_op_type {
	lgand, lgor, lgxor, lgnull
};
class Term {
	private:
		string value;
		string type;
	public:
		Term(const string v = "", const string term = keywords::text):value(v),type(term){};
		bool operator< (const Term) const;
		bool operator== (const Term) const;
		bool operator> (const Term) const;
		bool operator!= (const Term) const;
		Term operator+ (const Term) const;
		Term operator- (const Term) const;
		Term operator* (const Term) const;
		Term operator/ (const Term) const;
		Term operator% (const Term) const;
		Term operator= (const Term);
		bool isCompatibleWith (const Term) const;
		bool doesFitType() const;
		string getValue() const;
		string getType() const;
		Term& setValue(const string);
		Term& setType(const string);
		void print(ostream&, bool = false) const;
};
class BinaryExpression {
	public:
		Term first;
		Term second;
		string op;
		BinaryExpression(const Term f = Term(), const Term str = Term(), const string op = symbols::equals):first(f),second(str),op(op){};
		virtual void verifyValidity() = 0;
};
class ComparisonExpression extends public BinaryExpression {
	private:
		void verifyValidity();
	public:
		ComparisonExpression(const Term f = Term(), const Term str = Term(), const string op = symbols::equals):BinaryExpression(f,str,op){};
		bool result();
};
typedef vector<ComparisonExpression> vcompex;
typedef vector<logic_op_type> vlogicop_t;
class ComparisonList {
	private:
		vcompex cmpexs;
		vlogicop_t type;
	public:
		ComparisonList(vstring);
};
typedef map<string, Term> msterm;
typedef pair<string, Term> psterm;
class Row {
	private:
		msterm terms;
	public:
		Row(){}
		bool doesExist(const string) const;
		Term& findTerm(const string);
		void insertTerm(const string, const Term);
		void insertTerm(const psterm);
		void print(ostream&) const;
		void printTitle(ostream&) const;
		size_t size() const;
		msterm& getRaw();
		void setTerms(const msterm);
		void setTerm(const string, const Term);
		Row& mergeRowIntersect(const Row);
};
class Table {
	private:
		Row title;
		vector<Row> rows;
	public:
		Table(const Row row):title(row){}
		void insertRow(const Row);
		void print(ostream&) const;
		vector<Row>& getRaw();
		Row getTitle() const;
};
typedef map<string, Table> mstable;
typedef pair<string, Table> pstable;
class Database {
	private:
		mstable tables;
	public:
		Database(){}
		bool doesExist(const string) const;
		void insertTable(const string, const Table);
		void dropTable(const string);
		Table& findTable(const string);
		mstable getRaw() const;
};
typedef pair<string, Database> psdb;

map<string, Database> g_Databases;
string g_CurrentDatabaseName = "";

Database& getCurrentDatabase();
bool doesDatabaseExist(const string);
void useDatabase(const string);
void createDatabase(const string);

string judgeValueType(const string);


// 函数体定义全部写在下方

ComparisonList::ComparisonList(vstring params) {
	int i = 0;
	string str_first, str_second, op;
	for (auto it = params.begin(); it != params.end(); ++it, ++i) {
		switch (i % 4) {
			case 0:
				str_first = *it;
				break;
			case 1:
				op = *it;
				break;
			case 2:
				str_second = *it;
				do {
					ComparisonExpression ex (
						Term(str_first, judgeValueType(str_first)),
						Term(str_second, judgeValueType(str_second)),
						op
					);
					cmpexs.push_back(ex);
					type.push_back(logic_op_type::lgnull);
				} while (false);
				break;
			case 3:
				if (*it == keywords::_and)		type.at(i / 4) = logic_op_type::lgand;
				else if (*it == keywords::_or)	type.at(i / 4) = logic_op_type::lgor;
				else if (*it == keywords::_xor)	type.at(i / 4) = logic_op_type::lgxor;
				else 							type.at(i / 4) = logic_op_type::lgnull;
		}
	}
}

void ComparisonExpression::verifyValidity() {
	if (isValidCmpOp(op)) return;
	throw InvalidArgument(i18n::parseKey("invalidcmpop", {op}));
}

bool ComparisonExpression::result() {
	verifyValidity();
	bool res;
	if (op == symbols::less)			res = (first < second);
	else if (op == symbols::greater)	res = (first > second);
	else if (op == symbols::equals)		res = (first == second);
	else if (op == symbols::neq)		res = (first != second);
	return res;
}

Database& getCurrentDatabase() {
	if (g_CurrentDatabaseName != "") {
		try {
			return g_Databases.at(g_CurrentDatabaseName);
		}
		catch (out_of_range& e) {
			throw InvalidArgument(i18n::parseKey("nosuchdb", {g_CurrentDatabaseName}));
		}
	}
	throw SyntaxError(i18n::parseKey("noavaldb"));
}
bool doesDatabaseExists(const string str) {
	return g_Databases.find(str) != g_Databases.end();
}
void useDatabase(const string str) {
	if (doesDatabaseExists(str)) g_CurrentDatabaseName = str;
	else throw InvalidArgument(i18n::parseKey("nosuchdb",{str}));
}
void createDatabase(const string str) {
	if (doesDatabaseExists(str)) throw InvalidArgument(i18n::parseKey("duplicatedb", {str}));
	g_Databases.insert(psdb(str, Database()));
}
bool Database::doesExist(const string str) const {
	for (pstable p_table : tables) {
		if (p_table.first == str) return true;
	}
	return false;
}
void Database::insertTable(const string name, const Table table) {
	if (doesExist(name))	throw InvalidArgument(i18n::parseKey("duplicatetab", {name}));
	tables.insert(pstable(name, table));
}
void Database::dropTable(const string str) {
	if (!doesExist(str))	throw InvalidArgument(i18n::parseKey("nosuchtab", {str}));
	tables.erase(str);
}
Table& Database::findTable(const string str) {
	auto it_table = tables.find(str);
	if (it_table != tables.end()) return (*it_table).second;
	throw InvalidArgument(i18n::parseKey("nosuchtab", {str}));
}
mstable Database::getRaw() const {
	return tables;
}

void Table::insertRow(const Row row) {
	rows.push_back(row);
}
void Table::print(ostream& os) const {
	title.printTitle(os);
	os << endl;
	for (Row row : rows) {
		row.print(os);
		os << endl;
	}
}
vector<Row>& Table::getRaw() {
	return rows;
}
Row Table::getTitle() const {
	return title;
}

bool Row::doesExist(const string id) const {
	if (terms.find(id) != terms.end()) return true;
	return false;
}
Term& Row::findTerm(const string id) {
	auto it = terms.find(id);
	if (it != terms.end()) return it->second;
	throw InvalidArgument(i18n::parseKey("nosuchterm", {id}));
}
void Row::insertTerm(const string id, const Term term) {
	insertTerm(psterm(id, term));
}
void Row::insertTerm(const psterm p_term) {
	if (doesExist(p_term.first)) throw InvalidArgument(i18n::parseKey("duplicateterm", {p_term.first}));
	terms.insert(p_term);
}
void Row::print(ostream& os) const {
	bool f_isFirst = true;
	for (psterm p_term : terms) {
		if (f_isFirst) f_isFirst = false;
		else os << ',';
		p_term.second.print(os);
	}
}
void Row::printTitle(ostream& os) const {
	bool f_isFirst = true;
	for (psterm p_term : terms) {
		if (f_isFirst) f_isFirst = false;
		else os << ',';
		os << p_term.first;
	}
}
size_t Row::size() const {
	return terms.size();
}
msterm& Row::getRaw() {
	return terms;
}
void Row::setTerms(const msterm p_term) {
	terms = p_term;
}
void Row::setTerm(const string id, const Term term) {
	Term& t = terms.at(id);
	if (t.isCompatibleWith(term)) {
		t.setValue(term.getValue());
	}
	else {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {t.getType(), term.getType()}));
	}
}
Row& Row::mergeRowIntersect(const Row row) {
	vstring row_ids;
	for (psterm p_term : terms) {
		row_ids.push_back(p_term.first);
	}
	msterm terms_raw = row.terms;
	for (psterm p_term : terms_raw) {
		string id = p_term.first;
		if (doesContain(id, row_ids)) {
			Term term = p_term.second;
			this->setTerm(id, p_term.second);
		}
	}
	return *this;
}
bool Term::operator< (const Term term) const {
	if (!isCompatibleWith(term)) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type == keywords::text) {
		return value < term.value;
	}
	else {
		return stringToDouble(value) < stringToDouble(term.value);
	}
}
bool Term::operator== (const Term term) const {
	if (!isCompatibleWith(term)) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type == keywords::text) {
		return value == term.value;
	}
	else {
		return stringToDouble(value) == stringToDouble(term.value);
	}
}
bool Term::operator> (const Term term) const {
	if (!isCompatibleWith(term)) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type == keywords::text) {
		return value > term.value;
	}
	else {
		return stringToDouble(value) > stringToDouble(term.value);
	}
}
bool Term::operator!= (const Term term) const {
	if (!isCompatibleWith(term)) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type != keywords::text) {
		return value != term.value;
	}
	else {
		double difference = stringToDouble(value) - stringToDouble(term.value);
		return (difference > -g_DoubleEqCritDelta and difference < g_DoubleEqCritDelta);
	}
}
Term Term::operator+ (const Term term) const {
	if (!isCompatibleWith(term)) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type == keywords::text) {
		return Term(value+term.value, keywords::text);
	}
	else {
		if (type == keywords::integer and term.type == keywords::integer) {
			return Term(to_string(stringToInt(value) + stringToInt(term.value)),keywords::integer);
		}
		else {
			return Term(to_string(stringToDouble(value) + stringToDouble(term.value)),keywords::_float);
		}
	}
}
Term Term::operator- (const Term term) const {
	if (type == keywords::text or term.type == keywords::text) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type == keywords::integer and term.type == keywords::integer) {
		return Term(to_string(stringToInt(value) - stringToInt(term.value)),keywords::integer);
	}
	else {
		return Term(to_string(stringToDouble(value) - stringToDouble(term.value)),keywords::_float);
	}
}
Term Term::operator* (const Term term) const {
	if (type == keywords::text or term.type == keywords::text) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type == keywords::integer and term.type == keywords::integer) {
		return Term(to_string(stringToInt(value) * stringToInt(term.value)),keywords::integer);
	}
	else {
		return Term(to_string(stringToDouble(value) * stringToDouble(term.value)),keywords::_float);
	}
}
Term Term::operator/ (const Term term) const {
	if (type == keywords::text or term.type == keywords::text) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (stringToDouble(term.value) == 0) {
		throw InvalidArgument(i18n::parseKey("divzero"));
	}
	if (type == keywords::integer and term.type == keywords::integer) {
		return Term(to_string(stringToInt(value) / stringToInt(term.value)),keywords::integer);
	}
	else {
		return Term(to_string(stringToDouble(value) / stringToDouble(term.value)),keywords::_float);
	}
}
Term Term::operator% (const Term term) const {
	if (type == keywords::text or term.type == keywords::text) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (stringToDouble(term.value) == 0) {
		throw InvalidArgument(i18n::parseKey("divzero"));
	}
	if (type == keywords::integer and term.type == keywords::integer) {
		return Term(to_string(stringToInt(value) % stringToInt(term.value)),keywords::integer);
	}
	else {
		return Term(to_string(fmod(stringToDouble(value) , stringToDouble(term.value))),keywords::_float);
	}
}
Term Term::operator= (const Term term) {
	type = term.type;
	value = term.value;
	return *this;
}
bool Term::isCompatibleWith (const Term term) const {
	if (
		(type == keywords::text and term.type != keywords::text)
	or	(type != keywords::text and term.type == keywords::text)
	) {
		return false;
	}
	return true;
}
bool Term::doesFitType() const {
	string real_type = judgeValueType(value);
	if (type == keywords::integer or type == keywords::_float) {
		return real_type == keywords::_float or real_type == keywords::integer;
	}
	else return real_type == type;
}
string Term::getValue() const {
	return value;
}
string Term::getType() const {
	return type;
}
Term& Term::setValue(const string v) {
	value = v;
	if(!doesFitType()) {
		throw InvalidArgument(i18n::parseKey("vnfitt", {v, type}));
	}
	return *this;
}
Term& Term::setType(const string term) {
	if (equalIgnoringCase(term, keywords::integer)) type = keywords::integer;
	else if (equalIgnoringCase(term, keywords::_float)) type = keywords::_float;
	else if (equalIgnoringCase(term, keywords::text)) type = keywords::text;
	else if (equalIgnoringCase(term, keywords::variable)) type = keywords::variable;
	else throw InvalidArgument(i18n::parseKey("unacptvt", {term}));
	return *this;
}
void Term::print(ostream& os, bool f_isSql) const {
	if (type == keywords::integer)	os << stringToInt(value);
	else if (type == keywords::_float)	os << std::fixed << std::setprecision(2) << stringToDouble(value);
	else if (type == keywords::text) {
		if (f_isSql) os << '\'' << value.substr(1,value.size()-2) << '\'';
		else os << '\"' << value.substr(1,value.size()-2) << '\"';
	}
	else os << value;
}

string judgeValueType(const string value) {
	try {
		int i = stoi(value);
		double d = stod(value);
		double difference = d - i;
		if (difference > -1e-6 and difference < 1e-6) return keywords::integer;
		else return keywords::_float;
	}
	catch (invalid_argument& e) {		// 注意：这里的invalid_argument是std::~而不是minidb::InvalidArgument
		try {
			stod(value);
			return keywords::_float;
		}
		catch (invalid_argument& e) {
			if (value.size() != 0 and value.at(0) == '\'') return keywords::text;
			else return keywords::variable;
		}
	}
}




}
#endif