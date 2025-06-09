#include "Resolver.h"

void Resolver::resolveAST(Root* root)
{
	for(Definition *d : root->defs)
	{
		d->accept(this);
	}
}

int Resolver::getPrec(Token t)
{
    switch (t.type)
    {
    case DEquals:
        return 0;
    case LDBracket:
        return 0;
    case RDBracket:
        return 0;
    case LDBEq:
        return 0;
    case RDBEq:
        return 0;
    case NotEq:
        return 0;
    case Plus:
        return 1;
    case Minus:
        return 1;
    case Mult:
        return 2;
    case Div:
        return 2;
    }
    return 0;
}

void Resolver::visit(Root* node)
{
    return;
}

void Resolver::visit(Expression* node, std::string des)
{
    std::stack<Token> op;

    for (Token t : node->tokens)
    {
        if (t.type == Number || t.type == Identifier)
        {
            node->rpn.push(t);
        }
        else if (t.type == LParen)
        {
            op.push(t);
        }
        else if (t.type != Comma)
        {
            if (!op.empty())
            {
                if (t.type == RParen)
                {
                    while (op.top().type != LParen)
                    {
                        node->rpn.push(op.top());
                        op.pop();
                        if (op.empty()) break;
                    }
                    if (!op.empty()) op.pop();
                }
                else
                {
                    if (op.top().type != LParen)
                    {
                        while (getPrec(op.top()) >= getPrec(t))
                        {
                            node->rpn.push(op.top());
                            op.pop();
                            if (op.empty()) break;
                        }
                    }
                    op.push(t);
                }
            }
            else
            {
                op.push(t);
            }
        }
    }
    while (!op.empty())
    {
        node->rpn.push(op.top());
        op.pop();
    }
}
void Resolver::visit(VarAssign* node) 
{
	//auto r = st->find(node->t.value);
	//if (r == st->end()) { std::cout << "Couldn't resolve identifier \"" + node->t.value + "\"\n"; return; }
    node->expr.accept(this);
}
void Resolver::visit(FuncCall* node) 
{
    for(Expression &e : node->params)
    {
        e.accept(this);
    }
	return;
}
void Resolver::visit(ReturnCall* node) 
{
    node->expr.accept(this);
	return;
}
void Resolver::visit(IfStmnt* node) 
{
    followerCount = 0;
    std::unordered_map<std::string, Variable> prevSymTable = *st;

    node->cond.accept(this);
    //Mby check if expr is bool
    for(Statement* s : node->body)
    {
        s->accept(this);
    }
    if(node->next != nullptr)
    {
        node->next->accept(this);
    }
    node->followerCount = followerCount;
    node->symbolTable = *st;
    *st = prevSymTable;
}
void Resolver::visit(ElIfStmnt* node)
{
    std::unordered_map<std::string, Variable> prevSymTable = *st;
    followerCount++;
    node->cond.accept(this);
    //Mby check if expr is bool
    for (Statement* s : node->body)
    {
        s->accept(this);
    }
    if (node->next != nullptr)
    {
        node->next->accept(this);
    }
    node->symbolTable = *st;
    *st = prevSymTable;
}
void Resolver::visit(ElseStmnt* node) 
{
    std::unordered_map<std::string, Variable> prevSymTable = *st;
    followerCount++;
    for (Statement* s : node->body)
    {
        s->accept(this);
    }
    node->symbolTable = *st;
    *st = prevSymTable;
}
void Resolver::visit(WhileStmnt* node) 
{
    std::unordered_map<std::string, Variable> prevSymTable = *st;

    node->cond.accept(this);
    //Mby check if expr is bool
    for (Statement* s : node->body)
    {
        s->accept(this);
    }
    node->symbolTable = *st;
    *st = prevSymTable;
}
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
    node->expr.accept(this);
}
void Resolver::visit(FuncDef* node) 
{
    std::unordered_map<std::string, Function>* funcTable;
    std::unordered_map<std::string, Variable>* symTable;
    if(curClass == nullptr)
    {
        if(ft == nullptr)
        {
            ft = new std::unordered_map<std::string, Function>();
        }
        st = new std::unordered_map<std::string, Variable>();
        funcTable = ft;
        symTable = st;
    }
    else
    {
        funcTable = &curClass->functionTable;
        symTable = &curClass->classSymbolTable;
    }
	curFunc = &node->func;
	auto r = funcTable->find(node->t.value);
	if (r != funcTable->end()) { std::cout << "Function \"" + node->t.value + "\" already defined in scope\n"; return; }
    funcTable->insert({ node->t.value, node->func });
    node->func.symbolTable = *symTable;
	node->func.functionTable = funcTable;
	st = &curFunc->symbolTable;
	ft = curFunc->functionTable;
	for (Statement* s : node->statements)
	{
		s->accept(this);
	}
}