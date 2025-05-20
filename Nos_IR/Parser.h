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

	Token emptyTok = { TokenType::COMPILER_EMPTY, "", {0, 0} };

	std::vector<Token> getStatement();
	bool parseStatement(std::vector<Token> stmnt, std::unordered_map<std::string, Variable> *parScope, int* parCount, int scopeCount, std::vector<int>* scopes);
	bool parseFunctionDef(std::vector<Token> stmnt, std::unordered_map<std::string, Variable> *parScope, int* parCount, int scopeCount, std::vector<int>* scopes);
	bool parseVarDef(std::vector<Token> stmnt, std::unordered_map<std::string, Variable> *parScope, int* parCount, int scopeCount, std::vector<int> scopes);
	bool parseFunctionCall(Token call, std::unordered_map<std::string, Variable> parScope, std::vector<int> scopes);
	bool parseFuncReturn(std::vector<Token> stmnt, std::unordered_map<std::string, Variable> parScope, int scopeCount, std::vector<int> scopes);
	bool parseVarAsign(std::vector<Token> stmnt, std::unordered_map<std::string, Variable> *parScope, int scopeCount, std::vector<int> scopes);
	bool compExpr(std::vector<Token> expr, std::string x86Dest, Token dest, std::unordered_map<std::string, Variable> parScope, int scopeCount, std::vector<int> scopes);
	bool compTAC(Token l, Token r, std::string x86Dest, Token dest, std::string x86Operand, std::unordered_map<std::string, Variable> parScope, int scopeCount, std::vector<int> scopes);
	void printErrorMsg(std::string msg, Token t);
	bool parseExit(std::vector<Token> stmnt, std::unordered_map<std::string, Variable> *parScope, int scopeCount, std::vector<int> scopes); //Will probably be removed
	Lexer lex;
	x86Generator gen = x86Generator();
};

