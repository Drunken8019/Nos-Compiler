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

	struct Variable
	{
		Token val;
		int count;
		int scopeElevation;
	};

	//std::unordered_map<std::string, int> varTable;
	Token emptyTok = { TokenType::COMPILER_EMPTY, "", {0, 0} };

	std::vector<Token> getStatement();
	//int calcVarOffset(int offset);
	//std::string resolveIdent(Token t);
	bool parseStatement(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope, int* parCount);
	bool parseFunctionDef(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope, int* parCount);
	bool parseVarDef(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope, int* parCount);
	bool parseFunctionCall(Token call, std::unordered_map<std::string, int> parScope);
	bool parseFuncReturn(std::vector<Token> stmnt, std::unordered_map<std::string, int> parScope);
	bool parseVarAsign(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope);
	bool compExpr(std::vector<Token> expr, std::string x86Dest, Token dest, std::unordered_map<std::string, int> parScope);
	bool compTAC(Token l, Token r, std::string x86Dest, Token dest, std::string x86Operand, std::unordered_map<std::string, int> parScope);
	void printErrorMsg(std::string msg, Token t);
	bool parseExit(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope); //Will probably be removed
	Lexer lex;
	x86Generator gen = x86Generator();
};

