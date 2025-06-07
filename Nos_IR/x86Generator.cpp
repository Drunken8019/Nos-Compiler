#include "x86Generator.h"

void x86Generator::printDefaultHeader()
{
	*out <<
		"global main\n"
		"extern ExitProcess\n"
		"section .bss\n"
		"section .data\n"
		"section .text\n";
}

void x86Generator::printAST(Root root)
{
	printDefaultHeader();
	for(Definition *d : root.defs)
	{
		d->accept(this);
	}
}

std::string x86Generator::keyWord(Token t)
{
    switch (t.type)
    {
    case Plus:
        return "add qword";
    case Minus:
        return "sub qword";
    case Mult:
        return "imul qword";
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
    return "ERROR";
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
        if(s != st.end())
        {
            return blib::asmVar(s->second);
        }
    }
    return "not found";
}

void x86Generator::printMov(std::string type, std::string des, std::string src)
{
	if (!type.empty()) type.append(" ");
	*out << "mov " << type << des << ", " << src << std::endl;
}

void x86Generator::printAddSubMul(std::string x86Operand, std::string type, std::string des, std::string src)
{
	if (!type.empty()) type.append(" ");
	*out << x86Operand << " " << type << des << ", " << src << std::endl;
}

void x86Generator::visit(Root* node)
{
    return;
}

void x86Generator::visit(Expression* node, std::string des)
{
    bool first = true;
    std::string res = "";
    std::string rrr = "";
    std::string dest = "r10";
    //std::string res = "";
    std::stack<Token> operands;
    if (node->rpn.size() == 1)
    {
        res = "mov qword " + dest + ", " + resName(node->rpn.front()) + "\n";
        node->rpn.pop();
    }
    while (!node->rpn.empty()) //STOP APPENDING TO RES. WRITE STRAIGHT TO *OUT
    {
        Token t = node->rpn.front();
        rrr.append(t.value);
        if (t.type == Identifier || t.type == Number)
        {
            operands.push(t);
        }
        else
        {
            if (first)
            {
                if (isCmp(t))
                {

                    res.append("cmp qword " + resName(operands.top()) + ", ");
                    operands.pop();
                    res.append(resName(operands.top()) + "\n");
                    res.append(keyWord(t) + " r10b" + "\n");
                    res.append("movzx r10, r10b\n");
                }
                else
                {
                    res.append("mov qword " + dest + ", " + resName(operands.top()) + "\n");
                    operands.pop();
                    res.append(keyWord(t) + " " + dest + ", " + resName(operands.top()) + "\n");
                    operands.pop();
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
                        res.append("cmp qword " + resName(operands.top()) + ", ");
                        operands.pop();
                        res.append(resName(operands.top()) + "\n");
                        res.append(keyWord(t) + " r10b" + "\n");
                        res.append("movzx r10, r10b\n");
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
                        res.append("cmp qword r11, " + resName(operands.top()) + "\n");
                        res.append(keyWord(t) + " r10b" + "\n");
                        res.append("movzx r10, r10b\n");
                    }
                    else
                    {
                        res.append(keyWord(t) + " " + resName(operands.top()) + ", r11\n");
                    }
                }
                else
                {
                    std::string val = "r11";
                    TokenType ttype = EXPR_TMP;

                    if (isCmp(t))
                    {
                        res.append("cmp qword " + resName(operands.top()) + ", ");
                        operands.pop();
                        res.append(resName(operands.top()) + "\n");
                        res.append(keyWord(t) + " r11b\n");
                        res.append("movzx r11, r11b\n");
                        if (operands.top().type == EXPR_DEST) { res.append("mov qword " + dest + ", r11\n"); val = dest; ttype = EXPR_DEST; }
                    }
                    else
                    {
                        if (operands.top().type != EXPR_TMP) res.append("mov qword r11, " + resName(operands.top()) + "\n");
                        operands.pop();
                        res.append(keyWord(t) + " r11, " + resName(operands.top()) + "\n");
                        if (operands.top().type == EXPR_DEST) { res.append("mov qword " + dest + ", r11\n"); val = dest; ttype = EXPR_DEST; }
                    }
                    operands.pop();
                    operands.push({ ttype, val, {0, 0} });
                }
            }
        }
        node->rpn.pop();
    }
    if(!des.empty())
    {
        res.append("mov " + des + ", r10\n");
    }
    //std::cout << rrr << std::endl;
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
	node->expr.accept(this);
}
void x86Generator::visit(FuncCall* node)
{
	Function func;
	auto f = ft.find(node->t.value);
	if (f == ft.end()) { std::cout << "Couldn't resolve identifier \"" + node->t.value + "\"\n"; return; }
	else func = f->second;
	std::string res = "";
	res.append("call " + func.t.value + "\n");
	*out << res;
}
void x86Generator::visit(ReturnCall* node)
{
	if (node->expr.tokens.empty()) { *out << "ret\n";  return; }
	std::string res = "";
	node->expr.des = "[rsp+32]";
	node->expr.accept(this);

	if (curFunc.t.value == "main")
	{
		res.append("mov rcx, [rsp+32]\n");
		res.append("call ExitProcess\n");
	}
	else res.append("mov rax, [rsp+32]\n");
	int reqSize = curFunc.stackSize % 16;
	reqSize += curFunc.stackSize;
	reqSize += 40;
	res.append("add rsp, " + std::to_string(reqSize) + "\n");
	res.append("ret\n");
	*out << res;
}
void x86Generator::visit(IfStmnt* node)
{
    node->cond.accept(this);
    *out << "cmp r10, 1\n"
        "jne .L" + std::to_string(lCount) + "\n";
    for(Statement *s : node->body)
    {
        s->accept(this);
    }
    *out << ".L" + std::to_string(lCount) + ":\n";
    lCount++;
}
void x86Generator::visit(ElseStmnt* node)
{}
void x86Generator::visit(WhileStmnt* node)
{}
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
	node->expr.accept(this);
	return;
}
void x86Generator::visit(FuncDef* node)
{
	std::string res = "";
	res.append(node->func.t.value + ":\n");
	int reqSize = node->func.stackSize % 16;
	reqSize += node->func.stackSize;
	reqSize += 40;
	res.append("sub rsp, " + std::to_string(reqSize) + "\n");
	*out << res;
	st = node->func.symbolTable;
	ft = *node->func.functionTable;
	curFunc = node->func;
	for (Statement* s : node->statements)
	{
		s->accept(this);
	}
}