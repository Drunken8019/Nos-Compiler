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
	int curExprSize = 4;

	Register rax = { "rax", "eax", "ax", "al" };
	Register rcx = { "rcx", "ecx", "cx", "cl" };
	Register rdx = { "rdx", "edx", "dx", "dl" };
	Register rsi = { "rsi", "esi", "si", "sil" };
	Register rdi = { "rdi", "edi", "di", "dil" };
	Register r8 = { "r8", "r8d", "r8w", "r8b" };
	Register r9 = { "r9", "r9d", "r9w", "r9b" };
	Register r10 = { "r10", "r10d", "r10w", "r10b" };
	Register r11 = { "r11", "r11d", "r11w", "r11b" };
	Register r12 = { "r12", "r12d", "r12w", "r12b" };
	Register ptrR12 = { "[r12]", "[r12]", "[r12]", "[r12]" };
	Register r13 = { "r13", "r13d", "r13w", "r13b" };
	Register r14 = { "r14", "r14d", "r14w", "r14b" };
	Register r15 = { "r15", "r15d", "r15w", "r15b" };

	Register param[4] = {rcx, rdx, r8, r9};

	std::deque<ExprNode> temp = { ExprNode(rdi), ExprNode(r11), ExprNode(r13), ExprNode(r14), ExprNode(r15) };
	std::queue<ExprNode> freeTemp = std::queue<ExprNode>(temp);
	
	std::vector<ExprNode> inUseTemp;

	x86Generator(){};
	x86Generator(std::ofstream* o) : out(o) {};

	void printAST(Root root, std::vector<std::string> externs);

	void printDefaultHeader(std::vector<std::string> externs);
	std::string sizeWord(Type t);
	std::string chooseReg(Register reg);
	std::string chooseReg(Register reg, Type t);
	std::string keyWord(Token t);
	bool isCmp(Operator o);
	std::string resName(Token t);
	std::string unwrap(ExprNode en);
	std::string unwrap(ExprNode en, Type t);
	Type getType(ExprNode n);
	void setType(ExprNode* n, Type t);
	Type incrPtrType(Type t);
	Type decrPtrType(Type t);

	ExprNode printInstr(ExprNode instr, ExprNode l, ExprNode r);
	ExprNode printInstr(ExprNode instr, ExprNode l);
	int isTempInUse(ExprNode n);
	bool freeTempInUse(ExprNode n);

	void mov(ExprNode des, ExprNode src);

	void visit(Root* node) override;
	void visit(Expression* node) override;
	void visit(FuncCall* node) override;
	void visit(ReturnCall* node) override;
	void visit(IfStmnt* node) override;
	void visit(ElIfStmnt* node) override;
	void visit(ElseStmnt* node) override;
	void visit(WhileStmnt* node) override;
	void visit(ClassDefin* node) override;
	void visitSignature(ClassDefin* node) override;
	void visit(VarDef* node) override;
	void visit(FuncDef* node) override;
	void visitSignature(FuncDef* node) override;
	void visit(Body* node) override;
	void visit(DefinBody* node) override;
};