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
    root.body.accept(this);
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
                f->second->accept(this);
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

void x86Generator::visit(Expression* node) //TODO: new expression system needs a lot of work...
{
    if (node->rpn.empty()) return;
    Expression* prev = curExpr;
    curExpr = node; //FIND OUT WHY COPY BY VALUE IS CORRUPTING *node
    std::vector<ExprNode>& tokens = node->rpn;

    if(tokens.size() == 1)
    {
        if(tokens.front().isFuncCall())
        {
            FuncCall fc = std::get<FuncCall>(tokens.front().value);
            fc.accept(this);
        }
    }

    while (tokens.size() > 1)
    {
        bool reduced = false;
        for (int i = 0; i < tokens.size(); i++)
        {
            if (!tokens[i].isOperator()) continue;
            Operator op = std::get<Operator>(tokens[i].value);
            if(op.tok.type == UAmpersand)
            {
                printInstr(tokens[i], r10, tokens[i - 1]);
                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, r10);
                reduced = true;
                break;
            }

            if(op.tok.type == UAsteriks)
            {
                int tempSize = curExprSize;
                curExprSize = 8;
                printInstr(tokens[i], r12, tokens[i - 1]);
                curExprSize = tempSize;

                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, ptrR12);
                reduced = true;
                break;
            }
                           
            if (op.isUnary && i >= 1)
            {
                ExprNode operand = tokens[i - 1];
                mov(r10, operand);
                printInstr(tokens[i], r10);
                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, r10);
                reduced = true;
                break;
            }
            else if (op.isBinary && i >= 2)
            {
                ExprNode l = tokens[i - 2];
                ExprNode r = tokens[i - 1];
                mov(r11, l);
                printInstr(tokens[i], r11, r);
                mov(r10, r11);
                auto last = tokens.erase(tokens.begin() + i - 2, tokens.begin() + i + 1);
                tokens.insert(last, r10);
                reduced = true;
                break;
            }
        }
        if(!reduced)
        {
            std::cout << "Invalid or unsopported expression at line " << node->t.loc.line << "\n";
            return;
        }
    }

    curExpr = prev;
}

void x86Generator::visit(FuncCall* node)
{
    bool subStack = false;
    int prevSize = curExprSize;
    auto f = ft.find(node->t.value);
    if (f == ft.end()) { std::cout << "Couldn't resolve identifier \"" + node->t.value + "\"\n"; return; }
    else node->f = f->second;

    int stackSpaceForCall = node->f.paramStackSpace;
    
    if(node->f.isExtern)
    {
        stackSpaceForCall += 40;
        node->f.paramStackSpace += 40;
    }

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
                //node->params[i].des = chooseReg(param[i]);
                node->params[i]->accept(this);
                mov(param[i], r10);
            }
            else
            {
                int align = 8;
                if(align - node->f.params[i].type.size >= 0)
                {
                    node->f.paramStackSpace -= node->f.params[i].type.size;
                    //node->params[i].des = "[rsp+" + std::to_string(node->f.paramStackSpace) + "]";
                    node->f.params[i].numID = node->f.paramStackSpace;
                    curExprSize = node->f.params[i].type.size;
                    node->params[i]->accept(this);
                    mov(Literal("[rsp+" + std::to_string(node->f.params[i].numID) + "]"), r10);
                    align -= node->f.params[i].type.size;
                }
                else
                {
                    prevSize = curExprSize;
                    node->f.paramStackSpace -= align;
                    //node->params[i].des = "[rsp+" + std::to_string(node->f.paramStackSpace) + "]";
                    node->f.params[i].numID = node->f.paramStackSpace;
                    curExprSize = node->f.params[i].type.size;
                    node->params[i]->accept(this);
                    mov(Literal("[rsp+" + std::to_string(node->f.params[i].numID) + "]"), r10);
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

	if (node->expr->nodes.empty() && curFunc.stackSize == 0) { *out << "ret\n";  return; }
	std::string res = "";
    //*out << "mov qword [rsp], 0\n";
    if(curFunc.retType.name != "void" || curFunc.retType.isPtr)
    {
        node->expr->accept(this);
        mov(rax, r10);
    }

	if (curFunc.t.value == "main")
	{
		//res.append("mov " + chooseReg(rcx) + ", " + chooseReg(rax) + "\n");
        mov(rcx, rax);
        res.append("sub rsp, 40\n");
		res.append("call ExitProcess\n");
        *out << res;
        return;
        //res.append("add rsp, 40");
	}

	if(curFunc.stackSize != 0) res.append("add rsp, " + std::to_string(curFunc.stackSize) + "\n");
	res.append("ret\n");
	*out << res;
}
void x86Generator::visit(IfStmnt* node)
{
    node->cond->accept(this);

    lCount++;

    *out << "cmp r10, 0\n"
        "je .L" + std::to_string(lCount) + "\n";
    lCount++;
    int count = lCount;
    
    node->body.accept(this);

    if(node->next != nullptr)
    {
        *out << "jmp .L" + std::to_string(count) + "\n";
        *out << ".L" + std::to_string(count-1) + ":\n";
        node->next->endIndex = count;
        node->next->accept(this);
    }
    *out << ".L" + std::to_string(count) + ":\n";
        //lCount++;
}
void x86Generator::visit(ElIfStmnt* node)
{
    lCount++;
    //*out << ".L" + std::to_string(count) + ":\n";
    node->cond->accept(this);
    *out << "cmp r10, 0\n"
        "je .L" + std::to_string(lCount) + "\n";
    lCount++;
    int count = lCount;
   
    node->body.accept(this);

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
}
void x86Generator::visit(ElseStmnt* node)
{
    //lCount++;
    //*out << ".L" + std::to_string(lCount) + ":\n";
    node->body.accept(this);
}
void x86Generator::visit(WhileStmnt* node)
{
    int count = 0;

    wCount++;
    *out << ".W" + std::to_string(wCount) + ":\n";
    wCount++;
    count = wCount;

    node->cond->accept(this);
    *out << "cmp r10, 0\n"
        "je .W" + std::to_string(count) + "\n";
    
    node->body.accept(this);

    *out << "jmp .W" + std::to_string(count - 1) + "\n";
    *out << ".W" + std::to_string(count) + ":\n";
}
void x86Generator::visit(ClassDefin* node)
{
	return;
}
void x86Generator::visit(VarDef* node)
{
    if(node->var.type.name == "void" && node->var.type.isPtr == false)
    {
        std::cout << "Cannot define variable of type void\n";
    }
	std::string res = "";
    curExprSize = node->var.type.size;
	node->expr->accept(this);
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

    st = node->func.body->symbolTable;

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

    if (node->func.body != nullptr) node->func.body->accept(this);
}

void x86Generator::visit(Body* node)
{
    auto prevST = st;
    auto prevFT = ft;
    ft = node->functionTable;
    st = node->symbolTable;

    for(Statement* s : node->statements)
    {
        s->accept(this);
    }

    st = prevST;
    ft = prevFT;
}
void x86Generator::visit(DefinBody* node)
{
    auto prevST = st;
    auto prevFT = ft;
    ft = node->functionTable;
    st = node->symbolTable;

    for (Definition* d : node->definitions)
    {
        d->accept(this);
    }

    st = prevST;
    ft = prevFT;
}

void x86Generator::visitSignature(FuncDef* node)
{
    return;
}

void x86Generator::visitSignature(ClassDefin* node)
{
    return;
}

//HELPER-Functions
void x86Generator::mov(Token des, Token src)
{}

void x86Generator::mov(ExprNode des, ExprNode src)
{
    *out << "mov " << unwrap(des) << ", " << unwrap(src) << "\n";
}
void x86Generator::mov(Token des, Register src)
{
    *out << "mov " + resName(des) + ", " + chooseReg(src) + "\n";
}
void x86Generator::mov(Register des, Token src)
{
    *out << "mov " + chooseReg(des) + ", " + resName(src) + "\n";
}
void x86Generator::mov(Register des, Register src)
{
    *out << "mov " + chooseReg(des) + ", " + chooseReg(src) + "\n";
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
        *out << "cmp " + resName(des) + ", " + chooseReg(src) + "\n";
        *out << "mov " + src.qReg + ", 0\n";
        *out << keyWord(op) + " " + src.bReg + "\n";
        mov(des, src);
    }
    else
    {
        *out << keyWord(op) + " " + resName(des) + ", " + chooseReg(src) << "\n";
    }
}
void x86Generator::arithOp(Token op, Register des, Token src)
{
    if (isCmp(op))
    {
        *out << "cmp " + chooseReg(des) + ", " + resName(src) + "\n";
        *out << "mov " + des.qReg + ", 0\n";
        *out << keyWord(op) + " " + des.bReg + "\n";
    }
    else
    {
        *out << keyWord(op) << " " + chooseReg(des) + ", " + resName(src) + "\n";
    }
}
void x86Generator::arithOp(std::string x86Operand, std::string type, std::string des, std::string src)
{
    if (!type.empty()) type.append(" ");
    *out << x86Operand + " " + type + des + ", " + src << std::endl;
}


std::string x86Generator::unwrap(ExprNode en)
{
    if(en.isFuncCall())
    {
        FuncCall fc = std::get<FuncCall>(en.value);
        auto fr = ft.find(fc.t.value);
        if(fr == ft.end()) 
        {
            std::cout << "Identifier '" << fc.t.value << "' can't be resolved\n";
            return "unwrap error";
        }
        fc.f = fr->second;
        fc.accept(this);
        return chooseReg(rax);
    }
    else if(en.isLiteral())
    {
        Literal l = std::get<Literal>(en.value);
        return l.val;
    }
    else if(en.isOperator())
    {
        Operator o = std::get<Operator>(en.value);
        return o.keyWord;
    }
    else if(en.isVariableUse())
    {
        VariableUse v = std::get<VariableUse>(en.value);
        auto vr = st.find(v.identifier);
        if(vr == st.end()) 
        {
            std::cout << "Identifier '" << v.identifier << "' can't be resolved\n";
            return "unwrap error"; //DEBUG
        }
        return blib::asmVar(vr->second);
    }
    else if(en.isRegister())
    {
        Register r = std::get<Register>(en.value);
        return chooseReg(r);
    }
    else
    {
        return "no unwrap";
    }
}

void x86Generator::printInstr(ExprNode instr, ExprNode l, ExprNode r)
{
    *out << unwrap(instr) << " " << unwrap(l) << ", " << unwrap(r) << "\n";
}

void x86Generator::printInstr(ExprNode instr, ExprNode l)
{
    *out << unwrap(instr) << " " << unwrap(l) << "\n";
}