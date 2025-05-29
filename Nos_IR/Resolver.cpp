#include "Resolver.h"

void Resolver::resolveAST(ClassDefin* root)
{
	curClass = &root->c;
	for(Definition *d : root->defs)
	{
		d->accept(this);
	}
}

void Resolver::visit(Expression* node, std::string des){}
void Resolver::visit(VarAssign* node) 
{
	auto r = st->find(node->t.value);
	if (r == st->end()) { std::cout << "Couldn't resolve identifier \"" + node->t.value + "\"\n"; return; }
}
void Resolver::visit(FuncCall* node) 
{
	return;
}
void Resolver::visit(ReturnCall* node) 
{
	return;
}
void Resolver::visit(IfStmnt* node) 
{}
void Resolver::visit(ElseStmnt* node) 
{}
void Resolver::visit(WhileStmnt* node) 
{}
void Resolver::visit(ClassDefin* node) 
{
	return;
}
void Resolver::visit(VarDef* node) 
{
	auto r = st->find(node->t.value);
	if (r != st->end()) { std::cout << "Variable \"" + node->t.value + "\" already defined in scope\n"; return; }
	if (curFunc != nullptr)
	{
		curFunc->stackSize += node->var.size;
		node->var.numID = curFunc->varCount;
		curFunc->varCount++;
	}
	else
	{
		curClass->stackSize += node->var.size;
		node->var.numID = curClass->varCount;
		curClass->varCount++;
	}
	st->insert({ node->t.value, node->var });
}
void Resolver::visit(FuncDef* node) 
{
	curFunc = &node->func;
	auto r = curClass->functionTable.find(node->t.value);
	if (r != curClass->functionTable.end()) { std::cout << "Function \"" + node->t.value + "\" already defined in scope\n"; return; }
	curClass->functionTable.insert({ node->t.value, node->func });
	node->func.symbolTable = curClass->classSymbolTable;
	node->func.functionTable = &curClass->functionTable;
	st = &curFunc->symbolTable;
	ft = curFunc->functionTable;
	for (Statement* s : node->statements)
	{
		s->accept(this);
	}
}