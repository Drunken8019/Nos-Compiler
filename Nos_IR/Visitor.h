#pragma once
#include "Data.h"

class Visitor
{
public:
	virtual void visit(Expression* node, std::string des) = 0;
	virtual void visit(VarAssign* node) = 0;
	virtual void visit(FuncCall* node) = 0;
	virtual void visit(ReturnCall* node) = 0;
	virtual void visit(ClassDefin* node) = 0;
	virtual void visit(VarDef* node) = 0;
	virtual void visit(FuncDef* node) = 0;
};

