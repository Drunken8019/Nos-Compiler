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
	struct Syntax
	{
		std::vector<TokenType> preceding;
		std::vector<TokenType> subsequent;
	};
	Lexer lex;
	x86Generator gen;

	std::unordered_map<TokenType, Syntax> syntaxDef = {
		{TokenType::Identifier, //Token-Type for which the Syntax applies to
			{{TokenType::Equals}, //all relevant preceding Tokens
			{TokenType::Colon, TokenType::Equals}}, //all possible subsequent Tokens
		},

		{TokenType::Exit, //Token-Type
			{{}, //preceding
			{TokenType::Identifier, TokenType::Number}}, //subsequent
		},

		{TokenType::Equals, //Token-Type
			{{TokenType::Identifier}, //preceding
			{TokenType::Identifier, TokenType::Number}}, //subsequent
		}
	};
};

