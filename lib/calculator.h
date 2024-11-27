/**
 * 头文件：calculator.h
 * 计算右值表达式。
 */
#ifndef __CALCULATOR_MINIDB_H__
#define __CALCULATOR_MINIDB_H__

#include "objects.h"



namespace minidb {

void applyAsgnExpr(Row&, const string);
vstring convert2Postfix(const string);
Term calculatePostfix(const vstring, Row);
bool isExprOps(const string);
int getOpPriority(const string);
vstring g_exprOps = {
	symbols::plus,	symbols::minus,		symbols::times,		symbols::divides,
	symbols::mods,	symbols::lparen,	symbols::rparen
};

void applyAsgnExpr(Row& row, const string asgn_expr) {
	auto pos = asgn_expr.find('=');
	if (pos == string::npos) throw InvalidArgument(i18n::parseKey("exptsthgotothers", {i18n::parseKey("p_asgn").str(), "\"" + asgn_expr + "\""}));
	string lvalue = trim(asgn_expr.substr(0,pos));
	string rvalue = trim(asgn_expr.substr(pos+1));
	if (!isValidVarName(lvalue)) throw InvalidArgument(i18n::parseKey("invalidlval", {lvalue}));
	Term& term = row.findTerm(lvalue);
	Term res = calculatePostfix(convert2Postfix(rvalue), row);
	if (term.isCompatibleWith(res)) term = res;
	else throw InvalidArgument(i18n::parseKey("incmpttypes", {term.getType(), res.getType()}));
}
// 这里的expr是右值表达式
vstring convert2Postfix(const string expr) {
	stack<string> ops;
	vstring res;

	for (string token : splitByDelimiters(expr, ' ')) {
		if (token == "") continue;
		if (isExprOps(token)) {
			if (token == symbols::lparen) {
				ops.push(token);
			}
			else if (token == symbols::rparen) {
				string top;
				do {
					if (ops.empty()) throw SyntaxError(i18n::parseKey("mismparen"));
					top = ops.top();
					ops.pop();
					if (top != symbols::lparen) res.push_back(top);
					else break;
				} while (true);
			}
			else {
				if (ops.empty()) {
					ops.push(token);
					continue;
				}
				string top;
				do {
					if (ops.empty()) break;
					top = ops.top();
					if (getOpPriority(top) < getOpPriority(token)) break;
					else {
						ops.pop();
						res.push_back(top);
					}
				} while (true);
				ops.push(token);
			}
			continue;
		}
		else res.push_back(token);
	}
	while (!ops.empty()) {
		string op = ops.top();
		res.push_back(op);
		ops.pop();
	}
	return res;
}
Term calculatePostfix(const vstring params, Row row) {
	stack<Term> operands;
	for (string token : params) {
		if (isExprOps(token)) {
			Term second = operands.top();
			operands.pop();
			Term first = operands.top();
			operands.pop();
			Term res;
			if (token == symbols::plus)			res = first + second;
			else if (token == symbols::minus)	res = first - second;
			else if (token == symbols::times)	res = first * second;
			else if (token == symbols::divides)	res = first / second;
			else if (token == symbols::mods)	res = first % second;
			else if (token == symbols::lparen)	throw SyntaxError(i18n::parseKey("mismparen"));
			else throw SyntaxError(i18n::parseKey("unexptstr", {token}));
			operands.push(res);
		}
		else {
			Term term;
			string type = parseValueType(token);
			if (type == keywords::variable) {
				term = row.findTerm(token);
			}
			else {
				term.setType(type).setValue(token);
			}
			operands.push(term);
		}
	}
	return operands.top();
}
int getOpPriority(const string op) {
	if (op == symbols::plus or op == symbols::minus) return 1;
	else if (op == symbols::times or op == symbols::divides or op == symbols::mods) return 2;
	else if (op == symbols::lparen) return 0;
	else return -1;
}
bool isExprOps(const string op) {
	for (string str : g_exprOps) {
		if (op == str) return true;
	}
	return false;
}

}

#endif