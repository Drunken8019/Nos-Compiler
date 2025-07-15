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
	int scopeOffset = 0;
	bool reverseStackUsage = false;

	void resolveAST(Root* root);
	int getPrec(ExprNode n);

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

