#include "Visitor.h"

void Expression::accept(Visitor* v)
{
	v->visit(this, des);
}

void Statement::accept(Visitor* v)
{
	return;
}

void VarAssign::accept(Visitor* v)
{
	v->visit(this);
}

void FuncCall::accept(Visitor* v)
{
	v->visit(this);
}

void ReturnCall::accept(Visitor* v)
{
	v->visit(this);
}

void IfStmnt::accept(Visitor* v)
{
	v->visit(this);
}

void ElseStmnt::accept(Visitor* v)
{
	v->visit(this);
}

void WhileStmnt::accept(Visitor* v)
{
	v->visit(this);
}

void Definition::accept(Visitor* v)
{
	return;
}

void ClassDefin::accept(Visitor* v)
{
	v->visit(this);
}

void VarDef::accept(Visitor* v)
{
	v->visit(this);
}

void FuncDef::accept(Visitor* v)
{
	v->visit(this);
}