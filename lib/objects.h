/**
 * 头文件：objects.h
 * MiniDB的数据结构，以及必要的操作函数。
 */
#ifndef __OBJECTS_MINIDB_H__
#define __OBJECTS_MINIDB_H__

#include "stringop.h"

namespace minidb {

class Term {
	private:
		string value;
		string type;
	public:
		Term(const string v = "", const kwstring term = keywords::text):value(v),type(term.str()){};
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
		void print(ostream&) const;
};
typedef pair<string, Term> psterm;
class Row {
	private:
		// 由于map底层采用红黑树实现，其不可避免地会对key进行排序，但这会打乱Row内Term的顺序，因此只好退而求其次选择vector
		vector<psterm> terms;
	public:
		Row(){}
		bool doesExist(const string) const;
		int findIdIndex(const string) const;
		Term& findTerm(const string);
		void insertTerm(const string, const Term);
		void insertTerm(const psterm);
		void print(ostream&) const;
		void printTitle(ostream&) const;
		size_t size() const;
		vector<psterm>& getRaw();
		void setTerms(const vector<psterm>);
		void setTerm(const string, const Term);
		Row& mergeRowIntersect(const Row, const string);
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
		void removeRow(const int);
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

class BinaryExpression {
	protected:
		Term first;
		Term second;
		string op;
	public:
		BinaryExpression(const Term f = Term(), const Term str = Term(), const string op = symbols::equals):first(f),second(str),op(op){};
		virtual void verifyValidity() const = 0;
		void setFirst(const Term t){ first = t; }
		void setSecond(const Term t){ second = t; }
		void setOp(const string str){ op = str; }
};
class ComparisonExpression extends public BinaryExpression {
	private:
		void verifyValidity() const;
	public:
		ComparisonExpression(const Term f = Term(), const Term str = Term(), const string op = symbols::equals):BinaryExpression(f,str,op){};
		bool result() const;
};

map<string, Database> g_Databases;
string g_CurrentDatabaseName = "";

Database& getCurrentDatabase();
bool doesDatabaseExist(const string);
void useDatabase(const string);
void createDatabase(const string);

string parseValueType(const string);

const Row mergeRowUnion(Row, Row, const string, const string);

// 函数体定义全部写在下方

void ComparisonExpression::verifyValidity() const {
	if (isValidCmpOp(op)) return;
	throw InvalidArgument(i18n::parseKey("invalidcmpop", {op}));
}
bool ComparisonExpression::result() const {
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
	g_Databases.insert(pair<string, Database>(str, Database()));
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
void Table::removeRow(const int n) {
	if (n < 0 or (size_t)n > rows.size()) throw InvalidArgument(i18n::parseKey("outofbound", {itos(n)}));
	rows.erase(rows.begin()+n);
}
Row Table::getTitle() const {
	return title;
}

bool Row::doesExist(const string id) const {
	for (psterm p_term : terms) {
		if (p_term.first == id) return true;
	}
	return false;
}
int Row::findIdIndex(const string id) const {
	int i = -1;
	for (psterm p_term : terms) {
		++i;
		if (p_term.first == id) break;
	}
	return i;
}
Term& Row::findTerm(const string id) {
	int index = findIdIndex(id);
	if (index != -1) return terms.at(index).second;
	throw InvalidArgument(i18n::parseKey("nosuchterm", {id}));
}
void Row::insertTerm(const string id, const Term term) {
	if (doesExist(id)) throw InvalidArgument(i18n::parseKey("duplicateterm", {id}));
	terms.push_back(psterm(id, term));
}
void Row::insertTerm(const psterm p_term) {
	if (doesExist(p_term.first)) throw InvalidArgument(i18n::parseKey("duplicateterm", {p_term.first}));
	terms.push_back(p_term);
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
vector<psterm>& Row::getRaw() {
	return terms;
}
void Row::setTerms(const vector<psterm> p_term) {
	terms = p_term;
}
void Row::setTerm(const string id, const Term term) {
	Term& t = findTerm(id);
	if (t.isCompatibleWith(term)) {
		t.setValue(term.getValue());
	}
	else {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {t.getType(), term.getType()}));
	}
}
Row& Row::mergeRowIntersect(const Row row, const string tabn) {
	vstring row_ids;
	for (psterm p_term : terms) {
		row_ids.push_back(p_term.first);
	}
	vector<psterm> terms_raw = row.terms;
	for (psterm p_term : terms_raw) {
		string id = tabn + "." + p_term.first;
		if (doesContain(id, row_ids)) {
			Term term = p_term.second;
			this->setTerm(id, p_term.second);
		}
	}
	return *this;
}
const Row mergeRowUnion(Row row1, Row row2, const string tabn1, const string tabn2) {
	Row res;
	for (psterm& p_term : row1.getRaw()) {
		res.insertTerm(tabn1 + "." + p_term.first, p_term.second);
	}
	for (psterm& p_term : row2.getRaw()) {
		res.insertTerm(tabn2 + "." + p_term.first, p_term.second);
	}
	return res;
}

bool Term::operator< (const Term term) const {
	if (!isCompatibleWith(term)) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type == keywords::text) {
		return value < term.value;
	}
	else {
		double v1 = stringToDouble(value);
		double v2 = stringToDouble(term.value);
		return v1 < v2;
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
		double difference = stringToDouble(value) - stringToDouble(term.value);
		return (difference > -g_DoubleEqCritDelta and difference < g_DoubleEqCritDelta);
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
		double v1 = stringToDouble(value);
		double v2 = stringToDouble(term.value);
		return v1 > v2;
	}
}
bool Term::operator!= (const Term term) const {
	return !((*this) == term);
}
Term Term::operator+ (const Term term) const {
	if (!isCompatibleWith(term)) {
		throw InvalidArgument(i18n::parseKey("incmpttypes", {type, term.type}));
	}
	if (type == keywords::text) {
		return Term(value.substr(0,value.size()-1)+term.value.substr(1), keywords::text);
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
	string real_type = parseValueType(value);
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
	if (term == keywords::integer) type = keywords::integer.str();
	else if (term == keywords::_float) type = keywords::_float.str();
	else if (term == keywords::text) type = keywords::text.str();
	else if (term == keywords::variable) type = keywords::variable.str();
	else throw InvalidArgument(i18n::parseKey("unacptvt", {term}));
	return *this;
}
void Term::print(ostream& os) const {
	if (type == keywords::integer)	os << stringToInt(value);
	else if (type == keywords::_float)	os << std::fixed << std::setprecision(2) << stringToDouble(value);
	else if (type == keywords::text) {
		if (value.size() <= 2) os << "''";
		else os << '\'' << value.substr(1,value.size()-2) << '\'';
	}
	else os << value;
}

// 注意：该函数的invalid_argument是std::~而不是minidb::InvalidArgument。这是利用“stoi/stod在解析失败时抛出该异常”进行类型判断。
string parseValueType(const string value) {
	try {
		stod(value);
		if (value.find('.') != string::npos) return keywords::_float.str();
		else return keywords::integer.str();
	}
	catch (invalid_argument& e) {		
		if (value.size() != 0 and value.at(0) == '\'') return keywords::text.str();
		else return keywords::variable.str();
	}
}



}
#endif