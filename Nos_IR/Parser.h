#pragma once
#include "Lexer.h"
#include <vector>
#include "x86Generator.h"

class Parser
{
public:
	Parser(std::ifstream* in, std::ofstream* out);
	void parse();

//private:
	template<typename T> class Expression
	{
	public:
		bool lSet = false, rSet = false, opSet = false;
		T left;
		Token operand;
		T right;

		void setLeft(T l) { left = l; lSet = true; }
		void setRight(T r) { left = r; rSet = true; }
		void setOp(Token o) { operand = o; opSet = true; }
	};

	int varCount = 0;
	//std::unordered_map<std::string, int> varTable;

	std::vector<Token> getStatement();
	//int calcVarOffset(int offset);
	//std::string resolveIdent(Token t);
	bool parseStatement(std::vector<Token> stmnt);
	bool parseFunctionDef(std::vector<Token> stmnt);
	bool parseVarDef(std::vector<Token> stmnt);
	bool parseFunctionCall(std::vector<Token> stmnt);
	bool parseVarAsign(std::vector<Token> stmnt);
	bool compExpr(std::vector<Token> expr, std::string x86Dest, Token dest);
	bool compSMA(Token l, Token r, std::string x86Dest, Token dest, std::string x86Operand);
	bool parseExit(std::vector<Token> stmnt); //Will probably be removed
	Lexer lex;
	x86Generator gen = x86Generator();
};

