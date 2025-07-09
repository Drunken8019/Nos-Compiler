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
    case Asteriks:
        return 2;
    case Div:
        return 2;
    case UAmpersand:
        return 3;
    case UAsteriks:
        return 3;
    }
    return 0;
}

void Resolver::visit(Root* node)
{
    return;
}

void Resolver::visit(Expression* node, std::string des)
{
    //Function identifiers
    int fCallCount = 0;
    std::vector<Token> newTokens;
    for(int i = 0; i<node->tokens.size(); i++)
    {
        if(node->tokens[i].type == Identifier)
        {
            if(i+1 < node->tokens.size())
            {
                if(node->tokens[i+1].type == LParen)
                {
                    int fBegin = i;
                    FuncCall temp = { {Identifier, node->tokens[i].value, node->tokens[i].loc} };
                    i+=2;

                    while(node->tokens[i].type != RParen)
                    {
                        Expression e = { node->tokens[i] };
                        
                        while(node->tokens[i].type != Comma)
                        {
                            if (node->tokens[i].type == RParen || i >= node->tokens.size()) break;
                            e.tokens.push_back(node->tokens[i]);
                            i++;
                        }
                        if(node->tokens[i].type == Comma)
                        {
                            i++;
                        }
                        e.accept(this);
                        temp.params.push_back(e);
                        e.tokens.clear();
                        if (i >= node->tokens.size()) break;
                    }
                    node->exprFnTable.insert({ "f" + std::to_string(fCallCount), temp });
                    Token t = { Identifier, "f" + std::to_string(fCallCount), {0,0} };
                    newTokens.push_back(t);
                    fCallCount++;
                }
                else
                {
                    newTokens.push_back(node->tokens[i]);
                }
            }
            else
            {
                newTokens.push_back(node->tokens[i]);
            }
        }
        else
        {
            newTokens.push_back(node->tokens[i]);
        }
    }
    node->tokens = newTokens;

    //UnaryOperators
    for(int i = 0; i < node->tokens.size(); i++)
    {
        if(blib::canBeUnary(node->tokens[i].type))
        {
            if(i-1 >= 0)
            {
                if(node->tokens[i-1].type != Identifier && node->tokens[i - 1].type != Number)
                {
                    node->tokens[i].type = blib::getUnary(node->tokens[i].type);
                }
            }
            else
            {
                node->tokens[i].type = blib::getUnary(node->tokens[i].type);
            }
        }
    }

    //RPN
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
    /*if (node->f.params.size() > 4) TODO: Stack parameters
    {
        int align = 8;
        for (int i = 4; i < node->f.params.size(); i++)
        {
            if (align - node->f.params[i].type.size >= 0)
            {
                node->var.numID = curFunc->stackSize;
                curFunc->stackSize += node->var.type.size;
                curFunc->varCount++;
                spaceFor8ALign -= node->var.type.size;
            }
            else
            {
                curFunc->stackSize += spaceFor8ALign;
                spaceFor8ALign = 8;
                node->var.numID = curFunc->stackSize;
                curFunc->varCount++;
                spaceFor8ALign -= node->var.type.size;
            }
        }
    }*/

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
        if(reverseStackUsage)
        {
            int align = 8;
            if (align - node->var.type.size >= 0)
            {
                curFunc->stackSize -= node->var.type.size;
                node->var.numID = curFunc->stackSize + scopeOffset;
                curFunc->varCount++;
                align -= node->var.type.size;
            }
            else
            {
                curFunc->stackSize -= align;
                align = 8;
                node->var.numID = curFunc->stackSize + scopeOffset;
                curFunc->varCount++;
                align -= node->var.type.size;
            }
            st->insert({ node->t.value, node->var });
            node->expr.accept(this);
            return;
        }

        if(spaceFor8ALign - node->var.type.size >= 0)
        {
            node->var.numID = curFunc->stackSize + scopeOffset;
            curFunc->stackSize += node->var.type.size;
            curFunc->varCount++;
            spaceFor8ALign -= node->var.type.size;
        }
        else
        {
            curFunc->stackSize += spaceFor8ALign;
            spaceFor8ALign = 8;
            node->var.numID = curFunc->stackSize + scopeOffset;
            curFunc->varCount++;
            spaceFor8ALign -= node->var.type.size;
        }
	}
	else
	{
        node->var.numID = curClass->stackSize + scopeOffset;
		curClass->stackSize += node->var.type.size;
		curClass->varCount++;
	}
    if(spaceFor8ALign == 0)
    {
        spaceFor8ALign = 8;
    }
	st->insert({ node->t.value, node->var });
    node->expr.accept(this);
}
void Resolver::visit(FuncDef* node) 
{
    Register rcx = { "rcx", "ecx", "cx", "cl" };
    Register rdx = { "rdx", "edx", "dx", "dl" };
    Register r8 = { "r8", "r8d", "r8w", "r8b" };
    Register r9 = { "r9", "r9d", "r9w", "r9b" };

    Register param[4] = { rcx, rdx, r8, r9 };

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
    node->func.symbolTable = *symTable;
	node->func.functionTable = funcTable;
	st = &curFunc->symbolTable;
	ft = curFunc->functionTable;
    for(int i = 0; i< node->func.params.size(); i++)
    {
        if(i < 4)
        {
            Expression e;
            param[i].reqSize = node->func.params[i].type.size;
            e.tokens.push_back({ EXPR_TMP, param[i].getVal(), {} });
            VarDef vd = { node->func.params[i], {e}};
            vd.accept(this);
            node->func.params[i].numID = vd.var.numID;
        }
        else
        {
            int align = 8;
            if (align - node->func.params[i].type.size >= 0)
            {
                node->func.paramStackSpace += node->func.params[i].type.size;
                align -= node->func.params[i].type.size;
            }
            else
            {
                node->func.paramStackSpace += align;
                align = 8;
                align -= node->func.params[i].type.size;
            }
        }
    }
    int reqSizeParam = 0;
    /*if (node->func.paramStackSpace % 16 == 0)
    {
        reqSizeParam = node->func.paramStackSpace;
    }
    else
    {
        reqSizeParam = 16 - (node->func.paramStackSpace % 16);
        reqSizeParam += node->func.paramStackSpace;
    }*/
    
    if(node->func.paramStackSpace != 0)
    {
        reqSizeParam = 16 - (node->func.paramStackSpace % 16);
        reqSizeParam += node->func.paramStackSpace;
        node->func.paramStackSpace = reqSizeParam;
    }
    if (node->func.isExtern)
    {
        node->func.paramStackSpace += 40;
    }

	for (Statement* s : node->statements)
	{
		s->accept(this);
	}

    int reqSize = 0;
    /*if (node->func.stackSize % 16 == 0)
    {
        reqSize = node->func.stackSize;
    }
    else
    {
        reqSize = 16 - (node->func.stackSize % 16);
        reqSize += node->func.stackSize;
    }*/
    reqSize = 16 - (node->func.stackSize % 16);
    reqSize += node->func.stackSize;
    node->func.stackSize = reqSize;

    if (node->func.params.size() > 4)
    {
        //scopeOffset = node->func.paramStackSpace + 8;
        int prevStackSize = curFunc->stackSize;
        curFunc->stackSize += curFunc->paramStackSpace + 8;
        reverseStackUsage = true;
        for (int i = 4; i < node->func.params.size(); i++)
        {
            VarDef vd = { node->func.params[i], {} };
            vd.accept(this);
            node->func.params[i].numID = vd.var.numID;
        }
        curFunc->stackSize = prevStackSize;
        reverseStackUsage = false;
    }

    funcTable->insert({ node->t.value, node->func });
    spaceFor8ALign = 8;
}