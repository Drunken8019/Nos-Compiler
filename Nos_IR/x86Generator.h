#pragma once
#include "Visitor.h"

class x86Generator : public Visitor
{
public:
	std::ofstream* out;
	std::unordered_map<std::string, Variable> st;
	std::unordered_map<std::string, Function> ft;
	Function curFunc;
	Expression* curExpr = nullptr;
	int lCount = 0;
	int wCount = 0;
	int curExprResSize = 4;

	Register rax = { "rax", "eax", "ax", "al" };
	Register rcx = { "rcx", "ecx", "cx", "cl" };
	Register rdx = { "rdx", "edx", "dx", "dl" };
	Register rsi = { "rsi", "esi", "si", "sil" };
	Register rdi = { "rdi", "edi", "di", "dil" };
	Register r8 = { "r8", "r8d", "r8w", "r8b" };
	Register r9 = { "r9", "r9d", "r9w", "r9b" };
	Register r10 = { "r10", "r10d", "r10w", "r10b" };
	Register r11 = { "r11", "r11d", "r11w", "r11b" };

	Register param[4] = {rcx, rdx, r8, r9};

	x86Generator(){};
	x86Generator(std::ofstream* o) : out(o) {};

	void printAST(Root root, std::vector<std::string> externs);

	void printDefaultHeader(std::vector<std::string> externs);
	std::string sizeWord(Token var);
	std::string chooseReg(Register reg);
	std::string keyWord(Token t);
	bool isCmp(Token t);
	bool isUnary(Token t);
	std::string resName(Token t);

	void mov(Token des, Token src);
	void mov(Token des, Register src);
	void mov(Register des, Token src);
	void mov(std::string type, std::string des, std::string src);

	void arithOp(Token op, Token des, Token src);
	void arithOp(Token op, Token des, Register src);
	void arithOp(Token op, Register des, Token src);
	void arithOp(std::string x86Operand, std::string type, std::string des, std::string src);

	void visit(Root* node) override;
	void visit(Expression* node, std::string des) override;
	void visit(VarAssign* node) override;
	void visit(FuncCall* node) override;
	void visit(ReturnCall* node) override;
	void visit(IfStmnt* node) override;
	void visit(ElIfStmnt* node) override;
	void visit(ElseStmnt* node) override;
	void visit(WhileStmnt* node) override;
	void visit(ClassDefin* node) override;
	void visit(VarDef* node) override;
	void visit(FuncDef* node) override;
};