#pragma once
#include "Lexer.h"

class Parser
{
public:
	Parser(Lexer l);
	void parse();

private:
	Lexer lex;
};

