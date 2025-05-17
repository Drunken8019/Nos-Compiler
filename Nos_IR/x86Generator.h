#pragma once
#include <fstream>
#include "Lexer.h"
#include "ArithmeticExpression.h"

class x86Generator
{
public:
	x86Generator();
	x86Generator(std::ofstream* out);
	void printAsm(std::string s);
	void printDefaultHeader();
	void startMatching(Lexer l);

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
	private:
		CodeTemplate CTemplate;

	public:
		std::vector<Snippet> def;
		SyntaxTemplate(std::vector<Snippet> d, CodeTemplate CT) : def(d), CTemplate(CT)
		{
		}

		std::string x86Code(std::vector<Token> tokens)
		{
			std::string result = "";
			for(Option o : CTemplate.indexedx86Code)
			{
				switch(o.isString)
				{
				case true:
					result.append(o.getx86Code());
					break;
				case false:
					result.append(tokens.at(o.getIndex()).value);
					break;
				}
			}
			return result;
		}
	};

	std::vector<SyntaxTemplate> syntax = 
	{
		{
			{TokenType::Identifier, TokenType::Colon},
			{{0, 1, "\n"}}
		},
		{
			{TokenType::Exit, TokenType::Number, TokenType::Semicolon},
			{{"mov rcx, ", 1, "\ncall ExitProcess\n"}}
		},
	};

	std::ofstream* out;
};


