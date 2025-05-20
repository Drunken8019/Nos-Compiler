#pragma once
#include <fstream>
#include "Lexer.h"
#include "Data.h"

class x86Generator
{
public:
	std::ofstream* out;


	x86Generator();
	x86Generator(std::ofstream* out);

	int calcVarOffset(int offset, int scope, std::vector<int> scopes);
	std::string resolveIdent(Token t, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);
	int sumVector(std::vector<int> v, int end);

	void printAsm(std::string s);
	void printDefaultHeader();

	void printMov(Token des, Token src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);
	void printMov(Token des, std::string src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);
	void printMov(std::string des, Token src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);
	void printMov(std::string des, std::string src, std::string type, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);

	void printAddSubMul(std::string x86Operand, Token des, Token src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);
	void printAddSubMul(std::string x86Operand, Token des, std::string src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);
	void printAddSubMul(std::string x86Operand, std::string des, Token src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);
	void printAddSubMul(std::string x86Operand, std::string des, std::string src, std::string type, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes);
};