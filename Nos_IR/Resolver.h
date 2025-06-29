#pragma once
#include "Visitor.h"
#include "stack"

class Resolver : public Visitor
{
public:
	std::unordered_map<std::string, Variable>* st = nullptr;
	std::unordered_map<std::string, Function>* ft = nullptr;
	Function *curFunc = nullptr;
	Class *curClass = nullptr;
	int followerCount = 0;
	int spaceFor8ALign = 8;

	void resolveAST(Root* root);
	int getPrec(Token t);

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

