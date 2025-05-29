#pragma once
#include "Visitor.h"

class x86Generator : public Visitor
{
public:
	std::ofstream* out;
	std::unordered_map<std::string, Variable> st;
	std::unordered_map<std::string, Function> ft;
	Function curFunc;


	x86Generator(){};
	x86Generator(std::ofstream* o) : out(o) {};

	void printAST(ClassDefin root);

	void printDefaultHeader();

	void printMov(std::string type, std::string des, std::string src);

	void printAddSubMul(std::string x86Operand, std::string type, std::string des, std::string src);

	void visit(Expression* node, std::string des) override;
	void visit(VarAssign* node) override;
	void visit(FuncCall* node) override;
	void visit(ReturnCall* node) override;
	void visit(IfStmnt* node) override;
	void visit(ElseStmnt* node) override;
	void visit(WhileStmnt* node) override;
	void visit(ClassDefin* node) override;
	void visit(VarDef* node) override;
	void visit(FuncDef* node) override;
};