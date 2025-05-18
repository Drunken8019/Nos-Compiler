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
	LCBrace, RCBrace, LParen, RParen, Equals, Semicolon, 
	Let, Define, Identifier, Exit, Number,
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
	Token nextToken();
	Token peek();
	void saveToken(Token t);
	void clearSaveBuffer();
	bool useSaveBuffer = false;

private:
	bool loadTokens(std::string curLine);
	std::ifstream* in;
	Location loc = { 0, 0 };
	std::queue<Token> tokenBuffer;
	std::queue<Token> readBuffer;

	std::unordered_map<std::string, TokenType> keywords = {
		{"exit", TokenType::Exit},
		{"let", TokenType::Let},
		{"def", TokenType::Define},
	};

	std::unordered_map<char, TokenType> symbols = {
		{'{', TokenType::LCBrace},
		{'}', TokenType::RCBrace},
		{'(', TokenType::LParen},
		{')', TokenType::RParen},
		{'=', TokenType::Equals},
		{';', TokenType::Semicolon}
	};
};

