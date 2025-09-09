#include "Visitor.h"

int blib::offset = 0;

void Expression::accept(Visitor* v)
{
	v->visit(this);
}

void Body::accept(Visitor* v)
{
	v->visit(this);
}

void DefinBody::accept(Visitor* v)
{
	v->visit(this);
}

void Statement::accept(Visitor* v)
{
	return;
}

void FuncCall::accept(Visitor* v)
{
	v->visit(this);
}

void ReturnCall::accept(Visitor* v)
{
	v->visit(this);
}

void IfFollower::accept(Visitor* v)
{
	return;
}

void IfStmnt::accept(Visitor* v)
{
	v->visit(this);
}

void ElIfStmnt::accept(Visitor* v)
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

void Root::accept(Visitor* v)
{
	v->visit(this);
}

void Definition::acceptSig(Visitor* v)
{
	return;
}

void FuncDef::acceptSig(Visitor* v)
{
	v->visitSignature(this);
}

void ClassDefin::acceptSig(Visitor* v)
{
	v->visitSignature(this);
}

void VarDef::acceptSig(Visitor* v)
{
	return;
}