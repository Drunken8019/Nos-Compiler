#pragma once
#include "Lexer.h"
#include <vector>
#include "x86Generator.h"

class Parser
{
public:
	Parser(std::ifstream* in, std::ofstream* out);
	void parse();

private:
	std::vector<Token> getStatement();
	bool parseStatement(std::vector<Token> expr);
	bool parseFunctionDef(std::vector<Token> expr);
	bool parseVarDef(std::vector<Token> expr);
	Lexer lex;
	x86Generator gen;
};

