#pragma once
#include <fstream>
#include "Lexer.h"
#include "ArithmeticExpression.h"

enum CTemplates
{
	Function, VarDef, ExitProc
};

class x86Generator
{
public:
	template<typename L, typename R> class Option
	{
	private:
		L opt1;
		R opt2;

	public:
		const bool isLeft, isRight;
		Option(L o) : isLeft(true), isRight(false)
		{
			opt1 = o;
			opt2 = {};
		}

		Option(R o) : isLeft(false), isRight(true)
		{
			opt1 = {};
			opt2 = o;
		}

		L getLeft() { return opt1; }
		R getRight() { return opt2; }
	};

	struct CodeTemplate
	{
		std::vector<Option<std::string, Token>> indexedx86Code;
	};

	std::unordered_map<std::string, int> varTable;

	std::ofstream* out;

	x86Generator();
	x86Generator(std::ofstream* out);
	void printAsm(std::string s);
	void printDefaultHeader();
	std::string x86Code(CodeTemplate CTemplate);
	int variableCounter = 0;
};