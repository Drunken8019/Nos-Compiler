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

    switch(curExprSize)
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
    switch (curExprSize)
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
    case Ampersand:
        return "and";
    case Pipe:
        return "or";
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
    case UMinus:
        return "neg";
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
    case UMinus:
        return true;
    default:
        return false;

    }
}
bool x86Generator::isBinary(Token t)
{
    switch (t.type)
    {
    case Plus:
        return true;
    case Minus:
        return true;
    case Ampersand:
        return true;
    case Pipe:
        return true;
    case Asteriks:
        return true;
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
        if (t.value == "r10")
        {
            return chooseReg(r10);
        }
        else if (t.value == "r11")
        {
            return chooseReg(r11);
        }
        else if(t.value == "ptrR12")
        {
            return chooseReg(ptrR12);
        }
    }
    else if(t.type == EXPR_TMP)
    {
        return t.value;

    }
    else
    {
        auto s = st.find(t.value);
        if(curExpr != nullptr)
        {
            auto f = curExpr->exprFnTable.find(t.value);
            if (f != curExpr->exprFnTable.end())
            {
                f->second.accept(this);
                return chooseReg(rax);
            }
        }
        if(s != st.end())
        {
            return blib::asmVar(s->second);
        }
    }
    return "Undefined Identifier " + t.value;
}

void x86Generator::visit(Root* node)
{
    return;
}

void x86Generator::visit(Expression* node, std::string des)
{
    if (node->rpn.empty()) return;
    Expression* prev = curExpr;
    curExpr = node; //FIND OUT WHY COPY BY VALUE IS CORRUPTING *node
    std::vector<Token> tokens;
    Token operand;
    Token tr10 = { EXPR_DEST, "r10", {} };
    Token tr12 = { EXPR_DEST, "ptrR12", {} };
    if (node->rpn.size() == 1)
    {
        if(node->desIsPtrDref)
        {
            mov(r10, node->rpn.front());
            *out << "mov r12, " << des << "\n";
            des = chooseReg(ptrR12);
            for (int i = 1; i < node->ptrDesDepth; i++)
            {
                *out << "mov r12, [r12]\n";
            }
        }
        else
        {
            mov(r10, node->rpn.front());
        }
        *out << keyWord(node->resOperator) + " " + des + ", " + chooseReg(r10) + "\n";
        return;
    }

    while (!node->rpn.empty())
    {
        tokens.push_back(node->rpn.front());
        node->rpn.pop();
    }

    while (tokens.size() > 1)
    {
        bool reduced = false;
        for (int i = 0; i < tokens.size(); i++)
        {
            if(tokens[i].type == UAmpersand)
            {
                *out << "lea " + chooseReg(r10) + ", " + resName(tokens[i-1]) + "\n"; 
                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, tr10);
                reduced = true;
                break;
            }

            if(tokens[i].type == UAsteriks)
            {
                int tempSize = curExprSize;
                curExprSize = 8;
                mov(r12, tokens[i - 1]);
                curExprSize = tempSize;

                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, tr12);
                reduced = true;
                break;
            }

            if (isUnary(tokens[i]) && i >= 1)
            {
                operand = tokens[i - 1];
                mov(r10, operand);
                *out << keyWord(tokens[i]) << " " << chooseReg(r10) << "\n";
                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, tr10);
                reduced = true;
                break;
            }
            else if (isBinary(tokens[i]) && i >= 2)
            {
                Token l = tokens[i - 2];
                Token r = tokens[i - 1];
                mov(r11, l);
                arithOp(tokens[i], r11, r);
                mov(r10, r11);
                auto last = tokens.erase(tokens.begin() + i - 2, tokens.begin() + i + 1);
                tokens.insert(last, tr10);
                reduced = true;
                break;
            }
        }
        if(!reduced)
        {
            std::cout << "Invalid or unsopported expression at line " << tokens.front().loc.line << "\n";
            return;
        }
    }

    curExpr = prev;

    if (!des.empty() && tokens.front().value == "ptrR12")
    {
        if (node->desIsPtrDref)
        {
            *out << "mov r10, [r12]\n";
            *out << "mov r12, " << des << "\n";
            for (int i = 1; i < node->ptrDesDepth; i++)
            {
                *out << "mov r12, [r12]\n";
            }
            *out << keyWord(node->resOperator) << " " << chooseReg(ptrR12) << ", " << chooseReg(r10) << "\n";
        }
        else
        {
            *out << "mov r10, [r12]\n";
            *out << (keyWord(node->resOperator) + " " + des + ", " + chooseReg(r10) + "\n");
        }
    }
    else if (!des.empty())
    {
        if(node->desIsPtrDref)
        {
            *out << "mov r12, " << des << "\n";
            des = chooseReg(ptrR12);
            for (int i = 1; i < node->ptrDesDepth; i++)
            {
                *out << "mov r12, [r12]\n";
            }
        }
        *out << (keyWord(node->resOperator) + " " + des + ", " + chooseReg(r10) + "\n");
    }
}
void x86Generator::visit(VarAssign* node)
{
	Variable var;
	auto f = st.find(node->t.value);
	if (f == st.end()) { std::cout << "Couldn't resolve identifier \"" + node->t.value + "\"\n"; return; }
	else var = f->second;
    if(node->isPtrAccess)
    {
        node->expr.desIsPtrDref = true;
        node->expr.ptrDesDepth = node->ptrAccessDepth;
        node->expr.des = blib::asmVar(var);
        curExprSize = var.type.nonPointerSize;
        node->expr.accept(this);
    }
    else
    {
        node->expr.des = blib::asmVar(var);
        curExprSize = var.type.size;
        node->expr.accept(this);
    }
}
void x86Generator::visit(FuncCall* node)
{
    bool subStack = false;
    int prevSize = curExprSize;
    auto f = ft.find(node->t.value);
    if (f == ft.end()) { std::cout << "Couldn't resolve identifier \"" + node->t.value + "\"\n"; return; }
    else node->f = f->second;

    int stackSpaceForCall = node->f.paramStackSpace;
    

    if(!node->params.empty())
    {
        if (node->params.size() != node->f.params.size())
        {
            std::cout << "Function \"" << node->f.t.value << "\" takes " << std::to_string(node->f.params.size()) << " parameters, not " << std::to_string(node->params.size()) << "\n";
            return;
        }

        for(int i = 0; i < node->params.size(); i++)
        {
            if(i == 4)
            {
                *out << "sub rsp, " << std::to_string(stackSpaceForCall) << "\n";
                blib::offset = stackSpaceForCall;
                subStack = true;
            }
            if(i < 4)
            {
                curExprSize = node->f.params[i].type.size;
                node->params[i].des = chooseReg(param[i]);
                node->params[i].accept(this);
            }
            else
            {
                int align = 8;
                if(align - node->f.params[i].type.size >= 0)
                {
                    node->f.paramStackSpace -= node->f.params[i].type.size;
                    node->params[i].des = "[rsp+" + std::to_string(node->f.paramStackSpace) + "]";
                    curExprSize = node->f.params[i].type.size;
                    node->params[i].accept(this);
                    align -= node->f.params[i].type.size;
                }
                else
                {
                    prevSize = curExprSize;
                    node->f.paramStackSpace -= align;
                    node->params[i].des = "[rsp+" + std::to_string(node->f.paramStackSpace) + "]";
                    curExprSize = node->f.params[i].type.size;
                    node->params[i].accept(this);
                    align = 8;
                    align -= node->f.params[i].type.size;
                }
                if(align == 0)
                {
                    align = 8;
                }
            }
        }

        if (node->f.isExtern && subStack == false)
        {
            *out << "sub rsp, " << std::to_string(stackSpaceForCall) << "\n";
            subStack = true;
        }
    }
    curExprSize = prevSize;
    *out << ("call " + node->f.t.value + "\n");
    blib::offset = 0;
    if(subStack)
    {
        *out << "add rsp, " << std::to_string(stackSpaceForCall) << "\n";
    }
}
void x86Generator::visit(ReturnCall* node)
{
    curExprSize = curFunc.retType.size;

	if (node->expr.tokens.empty() && curFunc.stackSize == 0) { *out << "ret\n";  return; }
	std::string res = "";
    //*out << "mov qword [rsp], 0\n";
    if(curFunc.retType.name != "void" || curFunc.retType.isPtr)
    {
        node->expr.des = chooseReg(rax);
        node->expr.accept(this);
    }

	if (curFunc.t.value == "main")
	{
		res.append("mov " + chooseReg(rcx) + ", " + chooseReg(rax) + "\n");
        res.append("sub rsp, 40\n");
		res.append("call ExitProcess\n");
        *out << res;
        return;
        //res.append("add rsp, 40");
	}

	res.append("add rsp, " + std::to_string(curFunc.stackSize) + "\n");
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
    if(node->var.type.name == "void" && node->var.type.isPtr == false)
    {
        std::cout << "Cannot define variable of type void\n";
    }
	std::string res = "";
	node->expr.des = blib::asmVar(node->var);
    curExprSize = node->var.type.size;
	node->expr.accept(this);
	return;
}
void x86Generator::visit(FuncDef* node)
{
    if(node->func.isExtern)
    {
        return;
    }

    lCount = 0;
    wCount = 0;
    std::string res = "";
    res.append(node->func.t.value + ":\n");
    if(node->func.stackSize != 0)
    {
        /*int reqSize = 16 - (node->func.stackSize % 16);
        reqSize += node->func.stackSize;*/
        //reqSize += 40;
        res.append("sub rsp, " + std::to_string(node->func.stackSize) + "\n");
    }
    *out << res;
	st = node->func.symbolTable;
	ft = *node->func.functionTable;
	curFunc = node->func;
    for(int i = 0;i < node->func.params.size(); i++)
    {
        if(i<4)
        {
            curExprSize = node->func.params[i].type.size;
            mov(node->func.params[i].t, param[i]);
        }
        //*out << "mov " << blib::asmVar(node->func.params[i]) << ", " << chooseReg(param[i]) << "\n";
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
    *out << "mov " << resName(des) << ", " << chooseReg(src) << "\n";
}
void x86Generator::mov(Register des, Token src)
{
    *out << "mov " + chooseReg(des) << ", " << resName(src) << "\n";
}
void x86Generator::mov(Register des, Register src)
{
    *out << "mov " + chooseReg(des) << ", " << chooseReg(src) << "\n";
}
void x86Generator::mov(std::string type, std::string des, std::string src)
{
    if (!type.empty()) type.append(" ");
    *out << "mov " << type << des << ", " << src << "\n";
}

void x86Generator::arithOp(Token op, Token des, Token src)
{}
void x86Generator::arithOp(Token op, Token des, Register src)
{
    if(isCmp(op))
    {
        *out << "cmp " << resName(des) << ", " << chooseReg(src) << "\n";
        *out << "mov " << src.qReg << ", 0\n";
        *out << keyWord(op) << " " << src.bReg << "\n";
        mov(des, src);
    }
    else
    {
        *out << keyWord(op) << " " << resName(des) << ", " << chooseReg(src) << "\n";
    }
}
void x86Generator::arithOp(Token op, Register des, Token src)
{
    if (isCmp(op))
    {
        *out << "cmp " << chooseReg(des) << ", " << resName(src) << "\n";
        *out << "mov " << des.qReg << ", 0\n";
        *out << keyWord(op) << " " << des.bReg << "\n";
    }
    else
    {
        *out << keyWord(op) << " " << chooseReg(des) << ", " << resName(src) << "\n";
    }
}
void x86Generator::arithOp(std::string x86Operand, std::string type, std::string des, std::string src)
{
    if (!type.empty()) type.append(" ");
    *out << x86Operand << " " << type << des << ", " << src << std::endl;
}