#pragma once
#include <fstream>
#include "Lexer.h"

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

	std::ofstream* out;


	x86Generator();
	x86Generator(std::ofstream* out);

	int calcVarOffset(int offset);
	std::string resolveIdent(Token t, std::unordered_map<std::string, int> varTable);

	void printAsm(std::string s);
	void printDefaultHeader();
	void printMov(Token des, Token src, std::unordered_map<std::string, int> varTable);
	void printMov(Token des, std::string src, std::unordered_map<std::string, int> varTable);
	void printMov(std::string des, Token src, std::unordered_map<std::string, int> varTable);
	void printMov(std::string des, std::string src, std::string type, std::unordered_map<std::string, int> varTable);

	void printAddSubMul(std::string x86Operand, Token des, Token src, std::unordered_map<std::string, int> varTable);
	void printAddSubMul(std::string x86Operand, Token des, std::string src, std::unordered_map<std::string, int> varTable);
	void printAddSubMul(std::string x86Operand, std::string des, Token src, std::unordered_map<std::string, int> varTable);
	void printAddSubMul(std::string x86Operand, std::string des, std::string src, std::string type, std::unordered_map<std::string, int> varTable);
};