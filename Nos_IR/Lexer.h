#pragma once
#include <string>
#include <queue>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>

enum TokenType
{
	COMPILER_EOF, COMPILER_IDENT_COUNT,
	Identifier, Exit, Number, Colon, Equals, Semicolon
};

struct Location
{
	int line, column;
};

struct Token
{
	TokenType type;
	std::string value;
	Location loc;
};

class Lexer
{
public:
	Lexer();
	Lexer(std::ifstream* i);
	bool hasNext = true;
	Token nextToken();
	Token peek();

private:
	bool loadTokens(std::string curLine);
	std::ifstream* in;
	Location loc = { 0, 0 };
	std::queue<Token> tokenBuffer;

	std::unordered_map<std::string, TokenType> keywords = {
		{"exit", TokenType::Exit},
	};

	std::unordered_map<char, TokenType> symbols = {
		{':', TokenType::Colon},
		{'=', TokenType::Equals},
		{';', TokenType::Semicolon}
	};
};

