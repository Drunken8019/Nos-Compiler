#pragma once
#include "Lexer.h"
#include "x86Generator.h"
#include "Resolver.h"

class Parser
{
public:
	Parser(std::ifstream* in, std::ofstream* out);
	void parse();

//private:

	Token emptyTok = { TokenType::COMPILER_EMPTY, "", {0, 0} };
	Token errTok = { TokenType::COMPILER_ERROR, "", {0, 0} };

	std::vector<Token> getStatement();
	Root parseRoot(std::vector<Token> stmnt);
	ClassDefin parseClassDef(std::vector<Token> stmnt);
	Definition* parseDefinition(std::vector<Token> stmnt);
	Statement* parseStatement(std::vector<Token> stmnt);
	FuncDef parseFunctionDef(std::vector<Token> stmnt);
	VarDef parseVarDef(std::vector<Token> stmnt);
	FuncCall parseFunctionCall(std::vector<Token> stmnt);
	ReturnCall parseFuncReturn(std::vector<Token> stmnt);
	VarAssign parseVarAsign(std::vector<Token> stmnt);
	IfStmnt parseIfStmnt(std::vector<Token> stmnt);
	ElseStmnt parseElseStmnt(std::vector<Token> stmnt, IfStmnt prec);
	WhileStmnt parseWhileStmnt(std::vector<Token> stmnt);

	bool resolveAST(ClassDefin *root);

	void printErrorMsg(std::string msg, Token t);

	Lexer lex;
	x86Generator gen;
	Resolver res;
};

