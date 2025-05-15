#pragma once
#include <string>
#include <queue>
#include <iostream>
#include <fstream>
#include <unordered_map>

enum TokenType
{
	Identifier, Exit, Number, Colon
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
	Lexer(std::ifstream* i);
	bool hasNext = true;
	Token nextToken();
	Token peek();

private:
	bool loadTokens(std::string curLine);
	std::ifstream* in;
	size_t pos;
	Location loc;
	std::queue<Token> tokenBuffer;

	std::unordered_map<std::string, TokenType> keywords = {
		{"exit", TokenType::Exit}
	};
};

