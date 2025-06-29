#include "x86Generator.h"

void x86Generator::printDefaultHeader(std::vector<std::string> externs)
{
    *out <<
        "global main\n"
        "extern ExitProcess\n";

    for(std::string s : externs)
    {
        *out << "extern " + s + "\n";
    }

    *out << 		
        "section .bss\n"
		"section .data\n"
        "section .text\n";
}

void x86Generator::printAST(Root root, std::vector<std::string> externs)
{
	printDefaultHeader(externs);
	for(Definition *d : root.defs)
	{
		d->accept(this);
	}
}

std::string x86Generator::sizeWord(Token var)
{
    int size = 0;
    auto vl = st.find(var.value);
    if(vl != st.end())
    {
        size = vl->second.type.size;
    }

    switch(curExprResSize)
    {
    case 1:
        return "byte";
    case 2:
        return "word";
    case 4:
        return "dword";
    case 8:
        return "qword";
    default:
        return "sizeERR";
    }
}
std::string x86Generator::chooseReg(Register reg)
{
    switch (curExprResSize)
    {
    case 1:
        return reg.bReg;
    case 2:
        return reg.wReg;
    case 4:
        return reg.dReg;
    case 8:
        return reg.qReg;
    default:
        return "regErr";
    }
}
std::string x86Generator::keyWord(Token t)
{
    switch (t.type)
    {
    case Equals:
        return "mov";
    case UAmpersand:
        return "lea";
    case Plus:
        return "add";
    case Minus:
        return "sub";
    case Asteriks:
        return "imul";
    case DEquals:
        return "sete";
    case LDBracket:
        return "setl";
    case RDBracket:
        return "setg";
    case LDBEq:
        return "setle";
    case RDBEq:
        return "setge";
    case NotEq:
        return "setne";
    }
    return "KEYWORD not found";
}
bool x86Generator::isCmp(Token t)
{
    switch (t.type)
    {
    case DEquals:
        return true;
    case LDBracket:
        return true;
    case RDBracket:
        return true;
    case LDBEq:
        return true;
    case RDBEq:
        return true;
    case NotEq:
        return true;
    }
    return false;
}

bool x86Generator::isUnary(Token t)
{
    switch(t.type)
    {
    case UAmpersand:
        return true;
    case UAsteriks:
        return true;
    default:
        return false;

    }
}

std::string x86Generator::resName(Token t)
{
    if(t.type == Number)
    {
        return t.value;
    }
    else if(t.type == EXPR_DEST)
    {
        return t.value;
    }
    else if(t.type == EXPR_TMP)
    {
        return t.value;

    }
    else
    {
        auto s = st.find(t.value);
        auto f = curExpr->exprFnTable.find(t.value);
        if(s != st.end())
        {
            return blib::asmVar(s->second);
        }
        else if(f != curExpr->exprFnTable.end())
        {
            f->second.accept(this);
            return chooseReg(rax);
        }
    }
    return "Undefined Identifier";
}

void x86Generator::visit(Root* node)
{
    return;
}

void x86Generator::visit(Expression* node, std::string des) //Make functions for all this fuckass printing bs (mov, movxz, add, sub, mul, div...)
{
    Expression* prev = curExpr;
    curExpr = node; //FIND OUT WHY COPY BY VALUE IS CORRUPTING *node
    bool first = true;
    std::string res = "";
    std::string dest = chooseReg(r10);
    std::stack<Token> operands;
    if (node->rpn.size() == 1)
    {
        //res = "mov " + sizeWord(node->rpn.front()) + " " + dest + ", " + resName(node->rpn.front()) + "\n";
        mov(r10, node->rpn.front());
        node->rpn.pop();
    }
    while (!node->rpn.empty())
    {
        Token t = node->rpn.front();
        if (t.type == Identifier || t.type == Number)
        {
            operands.push(t);
        }
        else
        {
            if (isUnary(t))
            {
                res.append(keyWord(t) + " " + dest + ", " + resName(operands.top()) + "\n");
                operands.pop();
            }
            else if (first)
            {
                if (isCmp(t))
                {
                    Token r = operands.top(); operands.pop();
                    Token l = operands.top(); operands.pop();

                    res.append("cmp " + sizeWord(l) + " " + resName(l) + ", ");
                    res.append(resName(r) + "\n");
                    res.append(keyWord(t) + " r10b" + "\n");
                    res.append("movzx " + chooseReg(r10) + ", r10b\n");
                    operands.push({ EXPR_DEST, dest, t.loc });
                }
                else
                {
                    Token r = operands.top(); operands.pop();
                    Token l = operands.top(); operands.pop();
                    //res.append("mov " + sizeWord(operands.top()) + " " + dest + ", " + resName(operands.top()) + "\n");
                    mov(r10, l);
                    //operands.pop();
                    res.append(keyWord(t) + " " + dest + ", " + resName(r) + "\n");
                    //operands.pop();
                    operands.push({ EXPR_DEST, dest, t.loc });
                }
                first = false;
            }
            else
            {
                if (operands.top().type == EXPR_DEST)
                {
                    if (isCmp(t))
                    {
                        Token r = operands.top(); operands.pop();
                        Token l = operands.top(); operands.pop();
                        res.append("cmp " + sizeWord(l) + " " + resName(l) + ", ");
                        res.append(resName(r) + "\n");
                        res.append(keyWord(t) + " r10b" + "\n");
                        res.append("movzx " + chooseReg(r10) + ", r10b\n");
                        operands.push({ EXPR_DEST, dest, t.loc });
                    }
                    else
                    {
                        operands.pop();
                        res.append(keyWord(t) + " " + dest + ", " + resName(operands.top()) + "\n");
                        operands.pop();
                        operands.push({ EXPR_DEST, dest, t.loc });
                    }
                }
                else if (operands.top().type == EXPR_TMP)
                {
                    operands.pop();
                    if (isCmp(t))
                    {
                        res.append("cmp" + sizeWord(operands.top()) + " " + chooseReg(r11) + ", " + resName(operands.top()) + "\n");
                        res.append(keyWord(t) + " r10b" + "\n");
                        res.append("movzx " + chooseReg(r10) + ", r10b\n");
                    }
                    else
                    {
                        res.append(keyWord(t) + " " + resName(operands.top()) + ", " + chooseReg(r11) + "\n");
                    }
                }
                else
                {
                    std::string val = chooseReg(r11);
                    TokenType ttype = EXPR_TMP;

                    if (isCmp(t))
                    {
                        Token r = operands.top(); operands.pop();
                        Token l = operands.top();

                        res.append("cmp " + sizeWord(l) + " " + resName(l) + ", ");
                        res.append(resName(r) + "\n");
                        res.append(keyWord(t) + " r10b\n");
                        res.append("movzx " + chooseReg(r10) + ", r10b\n");
                        if (operands.top().type == EXPR_DEST) { val = dest; ttype = EXPR_DEST; }
                    }
                    else
                    {
                        if (operands.top().type != EXPR_TMP) mov(r11, operands.top());
                        operands.pop();
                        res.append(keyWord(t) + +" " + chooseReg(r11) + ", " + resName(operands.top()) + "\n");
                        if (operands.top().type == EXPR_DEST)
                        {
                            res.append("mov " + dest + ", " + chooseReg(r11) + "\n");
                            val = dest;
                            ttype = EXPR_DEST;
                        }
                    }
                    operands.pop();
                    operands.push({ ttype, val, {0, 0} });
                }
            }
        }
        node->rpn.pop();
    }
    if (!des.empty())
    {
        res.append(keyWord(node->resOperator) + " " + des + ", " + dest + "\n");
    }
    //std::cout << rrr << std::endl;
    curExpr = prev;
    *out << res;
}
void x86Generator::visit(VarAssign* node)
{
	Variable var;
	auto f = st.find(node->t.value);
	if (f == st.end()) { std::cout << "Couldn't resolve identifier \"" + node->t.value + "\"\n"; return; }
	else var = f->second;
	//res.append("mov qword [rsp+" + blib::varOffsetStr(var) + "], " + expr.res());
	node->expr.des = blib::asmVar(var);
    curExprResSize = var.type.size;
	node->expr.accept(this);
}
void x86Generator::visit(FuncCall* node)
{
	Function func;
    std::string res = "";
    int stackSpaceForCall = 0;
    auto f = ft.find(node->t.value);
    if (f == ft.end()) { std::cout << "Couldn't resolve identifier \"" + node->t.value + "\"\n"; return; }
    else func = f->second;

    if(!node->params.empty())
    {
        if (node->params.size() != func.params.size())
        {
            std::cout << "Function \"" << func.t.value << "\" takes " << std::to_string(func.params.size()) << " parameters, not " << std::to_string(node->params.size()) << "\n";
            return;
        }

        for(int i = 0; i < node->params.size(); i++)
        {
            if(i < 4)
            {
                curExprResSize = func.params[i].type.size;
                node->params[i].des = chooseReg(param[i]);
                node->params[i].accept(this);
            }
            else
            {

            }
        }
    }

	res.append("call " + func.t.value + "\n");
	*out << res;
}
void x86Generator::visit(ReturnCall* node)
{
    int reqSize = 16 - (curFunc.stackSize % 16);
    reqSize += curFunc.stackSize;
    //reqSize += 40;
    curExprResSize = curFunc.retType.size;

	if (node->expr.tokens.empty() && curFunc.stackSize != 0) { *out << "add rsp, " + std::to_string(reqSize) + "\nret\n";  return; }
	std::string res = "";
    //*out << "mov qword [rsp], 0\n";
    curExprResSize = curFunc.retType.size;
	node->expr.des = chooseReg(rax);
	node->expr.accept(this);

	if (curFunc.t.value == "main")
	{
		res.append("mov " + chooseReg(rcx) + ", " + chooseReg(rax) + "\n");
        res.append("sub rsp, 40\n");
		res.append("call ExitProcess\n");
        *out << res;
        return;
        //res.append("add rsp, 40");
	}

	res.append("add rsp, " + std::to_string(reqSize) + "\n");
	res.append("ret\n");
	*out << res;
}
void x86Generator::visit(IfStmnt* node)
{
    std::unordered_map<std::string, Variable> prevSymTable = st;
    st = node->symbolTable;
    node->cond.accept(this);

    lCount++;

    *out << "cmp r10, 0\n"
        "je .L" + std::to_string(lCount) + "\n";
    lCount++;
    int count = lCount;
    for(Statement *s : node->body)
    {
        s->accept(this);
    }
    if(node->next != nullptr)
    {
        *out << "jmp .L" + std::to_string(count) + "\n";
        *out << ".L" + std::to_string(count-1) + ":\n";
        node->next->endIndex = count;
        node->next->accept(this);
    }
    *out << ".L" + std::to_string(count) + ":\n";
        //lCount++;
    st = prevSymTable;
}
void x86Generator::visit(ElIfStmnt* node)
{
    std::unordered_map<std::string, Variable> prevSymTable = st;
    st = node->symbolTable;
    lCount++;
    //*out << ".L" + std::to_string(count) + ":\n";
    node->cond.accept(this);
    *out << "cmp r10, 0\n"
        "je .L" + std::to_string(lCount) + "\n";
    lCount++;
    int count = lCount;
    for (Statement* s : node->body)
    {
        s->accept(this);
    }
    if (node->next != nullptr)
    {
        *out << "jmp .L" + std::to_string(node->endIndex) + "\n";
        *out << ".L" + std::to_string(count-1) + ":\n";
        node->next->endIndex = node->endIndex;
        node->next->accept(this);
    }
    else
    {
        *out << ".L" + std::to_string(count-1) + ":\n";
    }
    st = prevSymTable;
}
void x86Generator::visit(ElseStmnt* node)
{
    std::unordered_map<std::string, Variable> prevSymTable = st;
    st = node->symbolTable;
    //lCount++;
    //*out << ".L" + std::to_string(lCount) + ":\n";
    for (Statement* s : node->body)
    {
        s->accept(this);
    }
    st = prevSymTable;
}
void x86Generator::visit(WhileStmnt* node)
{
    std::unordered_map<std::string, Variable> prevSymTable = st;
    st = node->symbolTable;
    int count = 0;

    wCount++;
    *out << ".W" + std::to_string(wCount) + ":\n";
    wCount++;
    count = wCount;

    node->cond.accept(this);
    *out << "cmp r10, 0\n"
        "je .W" + std::to_string(count) + "\n";
    for (Statement* s : node->body)
    {
        s->accept(this);
    }
    *out << "jmp .W" + std::to_string(count - 1) + "\n";
    *out << ".W" + std::to_string(count) + ":\n";
    st = prevSymTable;
}
void x86Generator::visit(ClassDefin* node)
{
	st = node->c.classSymbolTable;
	ft = node->c.functionTable;
	return;
}
void x86Generator::visit(VarDef* node)
{
	std::string res = "";
	node->expr.des = blib::asmVar(node->var);
    curExprResSize = node->var.type.size;
	node->expr.accept(this);
	return;
}
void x86Generator::visit(FuncDef* node)
{
    lCount = 0;
    wCount = 0;
    std::string res = "";
    res.append(node->func.t.value + ":\n");
    if(node->func.stackSize != 0)
    {
        int reqSize = 16 - (node->func.stackSize % 16);
        reqSize += node->func.stackSize;
        //reqSize += 40;
        res.append("sub rsp, " + std::to_string(reqSize) + "\n");
    }
    *out << res;
	st = node->func.symbolTable;
	ft = *node->func.functionTable;
	curFunc = node->func;
    for(int i = 0;i < node->func.params.size(); i++)
    {
        curExprResSize = node->func.params[i].type.size;
        //mov(param[i], node->func.params[i].t);
        *out << "mov " << blib::asmVar(node->func.params[i]) << ", " << chooseReg(param[i]) << "\n";
    }
	for (Statement* s : node->statements)
	{
		s->accept(this);
	}
}

void x86Generator::mov(Token des, Token src)
{}
void x86Generator::mov(Token des, Register src)
{
    *out << "mov " + sizeWord(des) << resName(des) << ", " << chooseReg(src) << "\n";
}
void x86Generator::mov(Register des, Token src)
{
    *out << "mov " + chooseReg(des) << ", " << resName(src) << "\n";
}
void x86Generator::mov(std::string type, std::string des, std::string src)
{
    if (!type.empty()) type.append(" ");
    *out << "mov " << type << des << ", " << src << "\n";
}

void x86Generator::arithOp(std::string x86Operand, std::string type, std::string des, std::string src)
{
    if (!type.empty()) type.append(" ");
    *out << x86Operand << " " << type << des << ", " << src << std::endl;
}