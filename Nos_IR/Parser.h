#pragma once
#include "Lexer.h"
#include "x86Generator.h"
#include "Resolver.h"
#include "TypeChecker.h"

class Parser
{
public:
	Parser(std::ifstream* in, std::ofstream* out);
	void parse();

//private:

	Token emptyTok = { TokenType::COMPILER_EMPTY, "", {0, 0} };
	Token errTok = { TokenType::COMPILER_ERROR, "", {0, 0} };
	std::vector<std::string> externs;
	std::stack<TokenType> openGroups;
	const Operator eq = Operator(Token(Equals, "=", Location(0, 0)));
	std::unordered_map<std::string, Type> primTypes = { {"char", Type(1, "char")},
		{"short", Type(2, "short")}, 
		{"int", Type(4, "int")}, 
		{"long", Type(8, "long")}, 
		{"void", Type(-1, "void")}, 
	};

	std::vector<Token> getStatement();
	Root parseRoot();
	Body parseBody();
	DefinBody parseDefinBody();
	DefinBody parseDefinBodyHeadless();
	ClassDefin parseClassDef();
	Definition* parseDefinition();
	Statement* parseStatement();
	FuncDef parseFunctionDef();
	FuncDef parseExternDef();
	Variable parseParamDef();
	VarDef parseVarDef();
	FuncCall parseFunctionCall();
	ReturnCall parseFuncReturn();
	IfStmnt parseIfStmnt();
	ElIfStmnt parseElIfStmnt();
	ElseStmnt parseElseStmnt();
	WhileStmnt parseWhileStmnt();
	Expression parseExpression();
	ExprNode parseExprNode();



	void printErrorMsg(std::string msg, Token t);
	Type getType(Token t);
	bool check(TokenType expected);
	bool match(TokenType expected);
	Token consume(TokenType t, const std::string& errorMsg);
	Token open(TokenType t, const std::string& errorMsg);
	Token close(TokenType t, const std::string& errorMsg);
	template<typename... TokenTypes>
	bool matchAny(TokenTypes... types);

	Lexer lex;
	x86Generator gen;
	Resolver res;
	TypeChecker tcheck;
};

