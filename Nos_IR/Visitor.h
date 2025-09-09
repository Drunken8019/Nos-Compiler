#pragma once
#include "Data.h"

class Visitor
{
public:
	virtual void visit(Root* node) = 0;
	virtual void visit(Expression* node) = 0;
	virtual void visit(Body* node) = 0;
	virtual void visit(DefinBody* node) = 0;
	virtual void visit(FuncCall* node) = 0;
	virtual void visit(ReturnCall* node) = 0;
	virtual void visit(IfStmnt* node) = 0;
	virtual void visit(ElIfStmnt* node) = 0;
	virtual void visit(ElseStmnt* node) = 0;
	virtual void visit(WhileStmnt* node) = 0;
	virtual void visit(ClassDefin* node) = 0;
	virtual void visitSignature(ClassDefin* node) = 0;
	virtual void visit(VarDef* node) = 0;
	virtual void visit(FuncDef* node) = 0;
	virtual void visitSignature(FuncDef* node) = 0;
};

