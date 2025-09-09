#include "Resolver.h"

void Resolver::resolveAST(Root* root)
{
    root->body.accept(this);
}

int Resolver::getPrec(ExprNode n) //Lower value means lower operator precedence                                                                  
{
    if (!n.isOperator()) return 0;
    Operator o = std::get<Operator>(n.value);
    switch (o.tok.type)
    {
    case Equals:
        return -1;
    case PlusEq:
        return -1;
    case MinusEq:
        return -1;
    case MultEq:
        return -1;
    case DivEq:
        return -1;
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
    case Modulo:
        return 2;
    case UAmpersand:
        return 3;
    case UAsteriks:
        return 3;
    }
    return 0;
}

void Resolver::visit(Root* node) //TODO: remove starting functions like resolveAst, just visite the root node instead
{
    return;
}

void Resolver::visit(Expression* node)
{
    //UnaryOperators
    for(int i = 0; i < node->nodes.size(); i++)
    {
        if(node->nodes[i].isOperator() && blib::canBeUnary(node->nodes[i].value))
        {
            if(i-1 >= 0)
            {
                if(node->nodes[i-1].isOperator() || node->nodes[i - 1].isLeftParen())
                {
                    std::get<Operator>(node->nodes[i].value).setUnary(true);
                }
            }
            else
            {
                std::get<Operator>(node->nodes[i].value).setUnary(true);
            }
        }
    }

    //RPN
    std::stack<ExprNode> op;

    for (ExprNode n : node->nodes)
    {
        if (n.isLiteral() || n.isVariableUse() || n.isFuncCall() || n.isRegister())
        {
            if(n.isFuncCall())
            {
                FuncCall fc = std::get<FuncCall>(n.value);
                fc.accept(this);
                n = ExprNode(fc);
            }
            node->rpn.push_back(n);
        }
        else if (n.isLeftParen())
        {
            op.push(n);
        }
        else if (!op.empty())
        {
            if (n.isRightParen())
            {
                while (!op.top().isLeftParen())
                {
                    node->rpn.push_back(op.top());
                    op.pop();
                    if (op.empty()) break;
                }
                if (!op.empty()) op.pop();
            }
            else
            {
                if (!op.top().isLeftParen())
                {
                    while (getPrec(op.top()) >= getPrec(n))
                    {
                        node->rpn.push_back(op.top());
                        op.pop();
                        if (op.empty()) break;
                    }
                }
                op.push(n);
            }
            }
        else
        {
            op.push(n);
        }
    }
    while (!op.empty())
    {
        node->rpn.push_back(op.top());
        op.pop();
    }
}
void Resolver::visit(FuncCall* node) 
{
    const Operator eq = Operator(Token(Equals, "=", Location(0, 0)));
    Register rcx = { "rcx", "ecx", "cx", "cl" };
    Register rdx = { "rdx", "edx", "dx", "dl" };
    Register r8 = { "r8", "r8d", "r8w", "r8b" };
    Register r9 = { "r9", "r9d", "r9w", "r9b" };
    Register param[4] = { rcx, rdx, r8, r9 };

    auto r = ft->find(node->t.value);
    if(r == ft->end()) 
    {
        std::cout << "Unkown identifier '" << node->t.value << "'\n";
        return;
    }
    node->f = r->second;

    if (node->f.isExtern)
    {
        node->f.paramStackSpace += 40;
    }


    for (int i = 0; i < node->params.size(); i++) //Handles parameters for functions: 1. Placed in Register/Stack 2. Where on stack
    {
        if (i < 4)
        {
            node->params[i]->nodes.insert(node->params[i]->nodes.begin(), eq);
            param[i].type = node->f.params[i].type;
            node->params[i]->nodes.insert(node->params[i]->nodes.begin(), param[i]);
            node->params[i]->accept(this);
        }
        else
        {
            int align = 8;
            if (align - node->f.params[i].type.size >= 0)
            {
                node->f.paramStackSpace -= node->f.params[i].type.size;
                node->f.params[i].numID = node->f.paramStackSpace;

                node->params[i]->nodes.insert(node->params[i]->nodes.begin(), eq);
                Literal destination = Literal("[rsp+" + std::to_string(node->f.params[i].numID) + "]");
                destination.type = node->f.params[i].type;
                node->params[i]->nodes.insert(node->params[i]->nodes.begin(), destination);

                node->params[i]->accept(this);
                align -= node->f.params[i].type.size;
            }
            else
            {
                node->f.paramStackSpace -= align;
                node->f.params[i].numID = node->f.paramStackSpace;

                node->params[i]->nodes.insert(node->params[i]->nodes.begin(), eq);
                Literal destination = Literal("[rsp+" + std::to_string(node->f.params[i].numID) + "]");
                destination.type = node->f.params[i].type;
                node->params[i]->nodes.insert(node->params[i]->nodes.begin(), destination);

                node->params[i]->accept(this);
                align = 8;
                align -= node->f.params[i].type.size;
            }
            if (align == 0)
            {
                align = 8;
            }
        }
    }
	return;
}
void Resolver::visit(ReturnCall* node) 
{
    if (node->expr->nodes.empty()) return;
    if(node->expr->nodes.front().isRegister()) //Set correct type for the return value of a function
    {
        Register r = std::get<Register>(node->expr->nodes.front().value); 
        r.type = curFunc->retType;
        ExprNode temp = ExprNode(r);
        node->expr->nodes.front().value.swap(temp.value);
    }
    node->expr->accept(this);
	return;
}
void Resolver::visit(IfStmnt* node) 
{
    followerCount = 0;

    node->cond->accept(this);
    
    node->body.accept(this);

    if(node->next != nullptr)
    {
        node->next->accept(this);
    }
    node->followerCount = followerCount;
}
void Resolver::visit(ElIfStmnt* node)
{
    followerCount++;
    node->cond->accept(this);
    
    node->body.accept(this);

    if (node->next != nullptr)
    {
        node->next->accept(this);
    }
}
void Resolver::visit(ElseStmnt* node) 
{
    followerCount++;
    node->body.accept(this);
}
void Resolver::visit(WhileStmnt* node) 
{
    node->cond->accept(this);
    node->body.accept(this);
}
void Resolver::visit(ClassDefin* node) 
{
	return;
}
void Resolver::visit(VarDef* node) 
{
	auto r = st->find(node->var.t.value);
	if (r != st->end()) { std::cout << "Variable \"" + node->var.t.value + "\" already defined in scope\n"; return; }
	if (curFunc != nullptr)
	{
        if(reverseStackUsage)
        {
            int align = 8;
            if (align - node->var.type.getSize() >= 0)
            {
                curFunc->stackSize -= node->var.type.getSize();
                node->var.numID = curFunc->stackSize + scopeOffset;
                curFunc->varCount++;
                align -= node->var.type.getSize();
            }
            else
            {
                curFunc->stackSize -= align;
                align = 8;
                node->var.numID = curFunc->stackSize + scopeOffset;
                curFunc->varCount++;
                align -= node->var.type.getSize();
            }
            st->insert({ node->var.t.value, node->var });
            node->expr->accept(this);
            return;
        }

        if(spaceFor8ALign - node->var.type.getSize() >= 0)
        {
            node->var.numID = curFunc->stackSize + scopeOffset;
            curFunc->stackSize += node->var.type.getSize();
            curFunc->varCount++;
            spaceFor8ALign -= node->var.type.getSize();
        }
        else
        {
            curFunc->stackSize += spaceFor8ALign;
            spaceFor8ALign = 8;
            node->var.numID = curFunc->stackSize + scopeOffset;
            curFunc->stackSize += node->var.type.getSize();
            curFunc->varCount++;
            spaceFor8ALign -= node->var.type.getSize();
        }
	}
	else
	{
        node->var.numID = curClass->stackSize + scopeOffset;
		curClass->stackSize += node->var.type.getSize();
		curClass->varCount++;
	}
    if(spaceFor8ALign == 0)
    {
        spaceFor8ALign = 8;
    }
	st->insert({ node->var.t.value, node->var });
    if(node->expr != nullptr) node->expr->accept(this);
}
void Resolver::visit(FuncDef* node) 
{
    auto prevSt = *st;
    Register rcx = { "rcx", "ecx", "cx", "cl" };
    Register rdx = { "rdx", "edx", "dx", "dl" };
    Register r8 = { "r8", "r8d", "r8w", "r8b" };
    Register r9 = { "r9", "r9d", "r9w", "r9b" };

    Register param[4] = { rcx, rdx, r8, r9 };

	curFunc = &node->func;

    for(int i = 0; i< node->func.params.size(); i++)  
    {
        if(i < 4) //VarDefs for register params
        {
            Expression* e = new Expression();
            e->nodes.push_back(ExprNode(param[i]));
            VarDef vd = { node->func.params[i], {e}};
            vd.accept(this);
            node->func.params[i].numID = vd.var.numID;
        }
        else
        {
            //Calculate stack-size for callee (stack parameters)
            int align = 8;
            if (align - node->func.params[i].type.getSize() >= 0)
            {
                node->func.paramStackSpace += node->func.params[i].type.getSize();
                align -= node->func.params[i].type.getSize();
            }
            else
            {
                node->func.paramStackSpace += align;
                node->func.paramStackSpace += node->func.params[i].type.getSize();
                align = 8;
                align -= node->func.params[i].type.getSize();
            }
        }
    }

    
    int reqSizeParam = 0;
    
    //16 Align callee stack-size
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

    //Build VarDefs for stack param's so they can be used as variables
    if (node->func.params.size() > 4)
    {
        int prevStackSize = curFunc->stackSize;
        curFunc->stackSize += curFunc->paramStackSpace + 8;
        reverseStackUsage = true;
        for (int i = 4; i < node->func.params.size(); i++)
        {
            VarDef vd = { node->func.params[i], {new Expression()}};
            vd.accept(this);
            node->func.params[i].numID = vd.var.numID;
        }
        curFunc->stackSize = prevStackSize;
        reverseStackUsage = false;
    }

    if (node->func.body != nullptr) node->func.body->accept(this);

    //Calculate stack-size for function
    if (node->func.stackSize != 0)
    {
        int reqSize = 0;
        reqSize = 16 - (node->func.stackSize % 16);
        reqSize += node->func.stackSize;
        node->func.stackSize = reqSize;
    }

    auto it = ft->find(node->func.t.value);
    if (it != ft->end())
        it->second = node->func;

    spaceFor8ALign = 8;
    *st = prevSt;
}
void Resolver::visit(Body* node) 
{
    if (ft == nullptr)
    {
        ft = new std::unordered_map<std::string, Function>();
    }
    if (st == nullptr)
    {
        st = new std::unordered_map<std::string, Variable>();
    }
    

    if (curClass != nullptr)
    {
        ft = &curClass->body->functionTable;
        st = &curClass->body->symbolTable;
    }

    auto prevFT = *ft;
    auto prevST = *st;

    for(Statement* s : node->statements)
    {
        s->accept(this);
    }

    node->symbolTable = *st;
    node->functionTable = *ft;

    *st = prevST;
    *ft = prevFT;
}
void Resolver::visit(DefinBody* node)
{
    if (ft == nullptr)
    {
        ft = new std::unordered_map<std::string, Function>();
    }
    if (st == nullptr)
    {
        st = new std::unordered_map<std::string, Variable>();
    }
    auto prevFT = *ft;
    auto prevST = *st;

    for(Definition* d : node->definitions)
    {
        d->acceptSig(this);
    }

    if (curClass == nullptr)
    {
        if (ft == nullptr)
        {
            ft = new std::unordered_map<std::string, Function>();
        }
        if (st == nullptr)
        {
            st = new std::unordered_map<std::string, Variable>();
        }
    }
    else
    {
        ft = &curClass->body->functionTable;
        st = &curClass->body->symbolTable;
    }
    for (Definition* d : node->definitions)
    {
        d->accept(this);
    }

    node->symbolTable = *st;
    node->functionTable = *ft;

    *st = prevST;
    *ft = prevFT;
}

void Resolver::visitSignature(FuncDef* node)
{
    auto r = ft->find(node->func.t.value);
    if (r != ft->end()) { std::cout << "Function \"" + node->func.t.value + "\" already defined in scope\n"; return; }

    for (int i = 4; i < node->func.params.size(); i++)
    {
        //Calculate stack-size for callee (stack parameters)
        int align = 8;
        if (align - node->func.params[i].type.getSize() >= 0)
        {
            node->func.paramStackSpace += node->func.params[i].type.getSize();
            align -= node->func.params[i].type.getSize();
        }
        else
        {
            node->func.paramStackSpace += align;
            node->func.paramStackSpace += node->func.params[i].type.getSize();
            align = 8;
            align -= node->func.params[i].type.getSize();
        }
    }

    ft->insert({ node->func.t.value, node->func });
}

void Resolver::visitSignature(ClassDefin* node)
{
    return;
}