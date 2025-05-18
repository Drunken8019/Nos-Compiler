#pragma once
#include <fstream>
#include "Lexer.h"
#include "ArithmeticExpression.h"

enum SyntaxType
{
	Function, VarDef, ExitProc
};

class x86Generator
{
private:
	class Snippet
	{
	private:
		ArithmeticExpression expr;
		TokenType irOp;

	public:
		const bool isToken, isArithmetic;
		Snippet(Token t) : isToken(false), isArithmetic(true)
		{
			expr = { t };
			irOp = TokenType::Identifier;
		}

		Snippet(TokenType t) : isToken(true), isArithmetic(false)
		{
			irOp = t;
		}

		ArithmeticExpression getExpr() { return expr; }
		TokenType getType() { return irOp; }
	};

	class Option
	{
	private:
		std::string opt1;
		int opt2;

	public:
		const bool isString, isIndex;
		Option(const char* s) : isIndex(false), isString(true)
		{
			opt1 = s;
			opt2 = -1;
		}

		Option(int i) : isIndex(true), isString(false)
		{
			opt2 = i;
		}

		std::string getx86Code() { return opt1; }
		int getIndex() { return opt2; }
	};

	struct CodeTemplate
	{
		std::vector<Option> indexedx86Code;
	};

	class SyntaxTemplate
	{
	public:
		CodeTemplate CTemplate;
		std::vector<Snippet> def;
		SyntaxType type;
		SyntaxTemplate(SyntaxType t, std::vector<Snippet> d, CodeTemplate CT) : type(t), def(d), CTemplate(CT)
		{
		}
	};

	std::vector<SyntaxTemplate> syntax =
	{
		{SyntaxType::Function,
			{TokenType::Define, TokenType::Identifier, TokenType::RCBrace},
			{{1, 2, "\n"}}
		},
		{SyntaxType::ExitProc,
			{TokenType::Exit, TokenType::Number, TokenType::Semicolon},
			{{"mov rcx, ", 1, "\ncall ExitProcess\n"}}
		},
		{SyntaxType::VarDef,
			{TokenType::Let, TokenType::Identifier, TokenType::Equals, TokenType::Number, TokenType::Semicolon},
			{{"push dword ", 3, "\n"}}
		},
		{SyntaxType::VarDef,
			{TokenType::Let, TokenType::Identifier, TokenType::Equals, TokenType::Identifier, TokenType::Semicolon},
			{{"push dword, ", 3, "\ncall ExitProcess\n"}}
		},
	};

	std::unordered_map<std::string, int> varTable;

	std::ofstream* out;

public:
	x86Generator();
	x86Generator(std::ofstream* out);
	void printAsm(std::string s);
	void printDefaultHeader();
	void startMatching(Lexer l);
	std::string x86Code(std::vector<Token> tokens, SyntaxTemplate t);
	int variableCounter = 0;
};