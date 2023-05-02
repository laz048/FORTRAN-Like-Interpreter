/*
 * parse.h
 */

#ifndef PARSE_H_
#define PARSE_H_

using namespace std;

#include <iostream>
#include "lex.h"
#include "lex.cpp"
#include "val.h"
#include "val.cpp"

namespace Parser {
	bool pushed_back = false;
	LexItem	pushed_token;

	static LexItem GetNextToken(istream& in, int& line) {
		if( pushed_back ) {
			pushed_back = false;
			return pushed_token;
		}
		return getNextToken(in, line);
	}

	static void PushBackToken(LexItem & t) {
		if( pushed_back ) {
			abort();
		}
		pushed_back = true;
		pushed_token = t;	
	}

}

static int error_count = 0;

int ErrCount()
{
    return error_count;
}

void ParseError(int line, string msg)
{
	++error_count;
	cout << line << ": " << msg << endl;
}

extern bool Prog(istream& in, int& line);
extern bool Stmt(istream& in, int& line);
extern bool Decl(istream& in, int& line);
extern bool PrintStmt(istream& in, int& line);
extern bool IfStmt(istream& in, int& line);
extern bool ReadStmt(istream& in, int& line);
extern bool IdList(istream& in, int& line, LexItem & tok);
extern bool VarList(istream& in, int& line);
extern bool Var(istream& in, int& line, LexItem & tok);
extern bool AssignStmt(istream& in, int& line);
extern bool ExprList(istream& in, int& line);
extern bool LogicExpr(istream& in, int& line, Value & retVal);
extern bool Expr(istream& in, int& line, Value & retVal);
extern bool Term(istream& in, int& line, Value & retVal);
extern bool SFactor(istream& in, int& line, Value & retVal);
extern bool Factor(istream& in, int& line, int sign, Value & retVal);
extern int ErrCount();

#endif /* PARSE_H_ */
