#pragma once
#include <string>
#include <queue>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include "Data.h"

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
		{"return", TokenType::Return},
	};

	std::unordered_map<char, TokenType> symbols = {
		{'{', TokenType::LCBrace},
		{'}', TokenType::RCBrace},
		{'(', TokenType::LParen},
		{')', TokenType::RParen},
		{'=', TokenType::Equals},
		{';', TokenType::Semicolon},
		{'+', TokenType::Plus},
		{'-', TokenType::Minus},
		{'*', TokenType::Mult},
		{'/', TokenType::Div},
	};
};

