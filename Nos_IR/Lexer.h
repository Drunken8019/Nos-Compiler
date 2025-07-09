#pragma once
#include "Visitor.h"

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
		{"let", TokenType::Let},
		{"def", TokenType::Define},
		{"return", TokenType::Return},
		{"class", TokenType::ClassDef},
		{"if", TokenType::If},
		{"elif", TokenType::Elif},
		{"else", TokenType::Else},
		{"while", TokenType::While},
		{"extern", TokenType::Extern},
		{"char", TokenType::Character},
		{"short", TokenType::Short},
		{"int", TokenType::Integer},
		{"long", TokenType::Long},
	};

	std::unordered_map<char, TokenType> symbols = {
		{'{', TokenType::LCBrace},
		{'}', TokenType::RCBrace},
		{'(', TokenType::LParen},
		{')', TokenType::RParen},
		{'=', TokenType::Equals},
		{';', TokenType::Semicolon},
		{',', TokenType::Comma},
		{'+', TokenType::Plus},
		{'-', TokenType::Minus},
		{'*', TokenType::Asteriks},
		{'/', TokenType::Div},
		{'<', TokenType::LDBracket},
		{'>', TokenType::RDBracket},
		{':', TokenType::Colon},
		{'&', TokenType::Ampersand},
		{'|', TokenType::Pipe},
	};

	std::unordered_map<std::string, TokenType> compoundSymbols = {
		{"==", TokenType::DEquals},
		{"<=", TokenType::LDBEq},
		{">=", TokenType::RDBEq},
		{"+=", TokenType::PlusEq},
		{"-=", TokenType::MinusEq},
		{"/=", TokenType::DivEq},
		{"*=", TokenType::MultEq},
		{"&&", TokenType::DAmpersand},
		{"||", TokenType::DPipe},
	};
};

