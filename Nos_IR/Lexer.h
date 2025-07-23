#pragma once
#include "Visitor.h"

class Lexer
{
public:
	Lexer();
	Lexer(std::ifstream* i);
	void initialize();
	Token nextToken();
	Token peek();
	Token peek_behind();
	void reset();
	bool stepBack(unsigned int len);
	bool setTo(unsigned int index);
	unsigned int getIndex();

private:
	bool loadTokens(std::string curLine);
	std::ifstream* in;
	Location loc = { 0, 0 };
	std::vector<Token> tokenBuffer;
	unsigned int index = 0;

	std::unordered_map<std::string, TokenType> keywords = {
		{"let", TokenType::Let},
		{"def", TokenType::Define},
		{"return", TokenType::Return},
		{"class", TokenType::ClassDef},
		{"if", TokenType::If},
		{"elif", TokenType::Elif},
		{"else", TokenType::Else},
		{"while", TokenType::While},
		{"extern", TokenType::Extern},
		{"nullptr", TokenType::NullPtr},
	};

	std::unordered_map<char, TokenType> symbols = {
		{'{', TokenType::LCBrace},
		{'}', TokenType::RCBrace},
		{'(', TokenType::LParen},
		{')', TokenType::RParen},
		{'[', TokenType::LSqParen},
		{']', TokenType::RSqParen},
		{'=', TokenType::Equals},
		{';', TokenType::Semicolon},
		{',', TokenType::Comma},
		{'+', TokenType::Plus},
		{'-', TokenType::Minus},
		{'*', TokenType::Asteriks},
		{'/', TokenType::Div},
		{'%', TokenType::Modulo},
		{'<', TokenType::LDBracket},
		{'>', TokenType::RDBracket},
		{':', TokenType::Colon},
		{'&', TokenType::Ampersand},
		{'|', TokenType::Pipe},
		{'!', TokenType::BoolNeg},
	};

	std::unordered_map<std::string, TokenType> compoundSymbols = {
		{"==", TokenType::DEquals},
		{"<=", TokenType::LDBEq},
		{">=", TokenType::RDBEq},
		{"!=", TokenType::NotEq},
		{"+=", TokenType::PlusEq},
		{"-=", TokenType::MinusEq},
		{"/=", TokenType::DivEq},
		{"*=", TokenType::MultEq},
		{"&&", TokenType::DAmpersand},
		{"||", TokenType::DPipe},
	};
};

