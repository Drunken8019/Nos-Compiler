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

void x86Generator::visit(Root* node)
{
    return;
}

void x86Generator::visit(Expression* node) //TODO: Clean up this mess, and let moving of mem values into regs be done in printInstr
{
    if (node->rpn.empty()) return;
    Expression* prev = curExpr;
    curExpr = node; //FIND OUT WHY COPY BY VALUE IS CORRUPTING *node
    curExprSize = node->calcType.size;
    std::vector<ExprNode>& tokens = node->rpn;

    if(tokens.size() == 1)
    {
        if(tokens.front().isFuncCall())
        {
            FuncCall fc = std::get<FuncCall>(tokens.front().value);
            fc.accept(this);
        }
    }

    for(int i = node->depth; i > 5; i--)
    {
        freeTemp.push(Literal("[rsp+" + std::to_string(curFunc.stackSize - i * 8) + "]"));
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
                printInstr(tokens[i], freeTemp.front(), tokens[i - 1]);
                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, freeTemp.front());
                inUseTemp.push_back(freeTemp.front()); freeTemp.pop();
                reduced = true;
                break;
            }

            if(op.tok.type == UAsteriks)
            {
                int tempSize = curExprSize;
                curExprSize = 8; //Mby this will be removed/changed when implementing type checker
                printInstr(tokens[i], r12, tokens[i - 1]);
                mov(freeTemp.front(), ptrR12);
                curExprSize = tempSize;

                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, freeTemp.front());
                inUseTemp.push_back(freeTemp.front()); freeTemp.pop();
                reduced = true;
                break;
            }
                           
            if (op.isUnary && i >= 1)
            {
                ExprNode operand = tokens[i - 1];
                mov(freeTemp.front(), operand);
                printInstr(tokens[i], freeTemp.front());
                auto last = tokens.erase(tokens.begin() + i - 1, tokens.begin() + i + 1);
                tokens.insert(last, freeTemp.front());
                reduced = true;
                inUseTemp.push_back(freeTemp.front()); freeTemp.pop();
                break;
            }
            else if (op.isBinary && i >= 2)
            {
                ExprNode l = tokens[i - 2];
                ExprNode r = tokens[i - 1]; 

                if(op.isAssign)
                {
                    mov(r10, l);
                    printInstr(tokens[i], r10, r);
                    mov(l, r10);
                    auto last = tokens.erase(tokens.begin() + i - 2, tokens.begin() + i + 1);
                    tokens.insert(last, l);
                }
                else
                {
                    mov(freeTemp.front(), l);
                    printInstr(tokens[i], freeTemp.front(), r);
                    auto last = tokens.erase(tokens.begin() + i - 2, tokens.begin() + i + 1);
                    tokens.insert(last, freeTemp.front());
                    inUseTemp.push_back(freeTemp.front()); freeTemp.pop();
                }

                if (isTempInUse(l) != -1)
                {
                    freeTemp.push(inUseTemp[isTempInUse(l)]);
                    inUseTemp.erase(inUseTemp.begin() + isTempInUse(l));
                }

                if (isTempInUse(r) != -1)
                {
                    freeTemp.push(inUseTemp[isTempInUse(r)]);
                    inUseTemp.erase(inUseTemp.begin() + isTempInUse(r));
                }
                
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
    if(prev != nullptr) curExprSize = prev->calcType.size;
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

        if(stackSpaceForCall != 0)
        {
            *out << "sub rsp, " << std::to_string(stackSpaceForCall) << "\n";
            blib::offset = stackSpaceForCall;
            subStack = true;
        }

        for(Expression* e : node->params)
        {
            e->accept(this);
        }
    }

    if (node->f.isExtern && subStack == false)
    {
        *out << "sub rsp, " << std::to_string(stackSpaceForCall) << "\n";
        subStack = true;
    }

    curExprSize = prevSize;
    *out << ("call " + node->f.t.value + "\n");
    
    if(subStack)
    {
        *out << "add rsp, " << std::to_string(stackSpaceForCall) << "\n";
    }
    blib::offset = 0;
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

    *out << "cmp r10b, 0\n"
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
    else
    {
        count--;
    }
    *out << ".L" + std::to_string(count) + ":\n";
        //lCount++;
}
void x86Generator::visit(ElIfStmnt* node)
{
    lCount++;
    //*out << ".L" + std::to_string(count) + ":\n";
    node->cond->accept(this);
    *out << "cmp r10b, 0\n"
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
    *out << "cmp r10b, 0\n"
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
    if (node->expr != nullptr) node->expr->accept(this);
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
            mov(VariableUse(node->func.params[i]), param[i]);
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
void x86Generator::mov(ExprNode des, ExprNode src)
{
    *out << "mov " << unwrap(des) << ", " << unwrap(src) << "\n";
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
std::string x86Generator::unwrap(ExprNode en, Type t)
{
    if (en.isFuncCall())
    {
        FuncCall fc = std::get<FuncCall>(en.value);
        auto fr = ft.find(fc.t.value);
        if (fr == ft.end())
        {
            std::cout << "Identifier '" << fc.t.value << "' can't be resolved\n";
            return "unwrap error";
        }
        fc.f = fr->second;
        fc.accept(this);
        return chooseReg(rax, t);
    }
    else if (en.isLiteral())
    {
        Literal l = std::get<Literal>(en.value);
        return l.val;
    }
    else if (en.isOperator())
    {
        Operator o = std::get<Operator>(en.value);
        return o.keyWord;
    }
    else if (en.isVariableUse())
    {
        VariableUse v = std::get<VariableUse>(en.value);
        auto vr = st.find(v.identifier);
        if (vr == st.end())
        {
            std::cout << "Identifier '" << v.identifier << "' can't be resolved\n";
            return "unwrap error"; //DEBUG
        }
        return blib::asmVar(vr->second);
    }
    else if (en.isRegister())
    {
        Register r = std::get<Register>(en.value);
        return chooseReg(r, t);
    }
    else
    {
        return "no unwrap";
    }
}
ExprNode x86Generator::printInstr(ExprNode instr, ExprNode l, ExprNode r)
{
    Operator o;
    if(instr.isOperator())
    {
        o = std::get<Operator>(instr.value);
    }

    if(o.tok.type == Div || o.tok.type == Modulo) //TODO: test div/modulo for safety with function calls
    {
        mov(rax, l);
        *out << "mov r10, rdx\n";
        *out << "mov rdx, 0\n";  //replace this with xor, if it doesnt mess with comparisons
        mov(rsi, r);
        *out << o.keyWord << " " << chooseReg(rsi) << "\n";
        
        if(o.tok.type == Div)
        {
            mov(l, rax);
        }
        else if(o.tok.type == Modulo)
        {
            if(curExprSize == 1)
            {
                mov(l, Literal("ah"));
            }
            else
            {
                mov(l, rdx);
            }
        }
        *out << "mov rdx, r10\n";
        return l;
    }
    else if(isCmp(o))
    {
        *out << "xor r10, r10\n";
        *out << "cmp " << sizeWord(getType(l)) << " " << unwrap(l) << ", " << unwrap(r) << "\n";
        *out << unwrap(instr) << " " << r10.bReg << "\n";
        mov(l, r10);
        return l;
    }
    else
    {
        //ExprNode res = l;
        ExprNode leftOperand = l;
        ExprNode rightOperand = r;
        ExprNode tempRes = freeTemp.front();
        Type tLeft = getType(l);
        Type tRight = getType(r);

        if (o.isAssign)
        {
            if(!rightOperand.isRegister() && !rightOperand.isLiteral() && !leftOperand.isRegister())
            {
                *out << "mov " << unwrap(r10, tRight) << ", " << sizeWord(tRight) << " " << unwrap(rightOperand);
                Register temp = r10;
                temp.type = tRight;
                rightOperand = temp;
            }
        }
        else
        {
            if (!leftOperand.isRegister())
            {
                if(!tempRes.isRegister())
                {
                    *out << "mov " << unwrap(r10, tLeft) << ", " << sizeWord(tLeft) << " " << unwrap(leftOperand);
                    Register temp = r10;
                    temp.type = tLeft;
                    leftOperand = temp;
                }
                else
                {
                    *out << "mov " << unwrap(tempRes, tLeft) << ", " << sizeWord(tLeft) << " " << unwrap(leftOperand);
                    Register temp = std::get<Register>(tempRes.value);
                    temp.type = tLeft;
                    leftOperand = temp;
                }
                inUseTemp.push_back(freeTemp.front()); freeTemp.pop();
            }
        }

        if(tLeft.size > tRight.size)
        {
            if(tRight.size < 4)
            {
                *out << "movzx " << unwrap(r10, tLeft) << ", " << sizeWord(tRight) << " " << unwrap(rightOperand) << "\n";
                Register temp = r10;
                temp.type = tLeft;
                rightOperand = temp;
            }
            else
            {
                *out << "mov " << unwrap(r10, tLeft) << ", " << sizeWord(tRight) << " " << unwrap(rightOperand) << "\n";
                Register temp = r10;
                temp.type = tLeft;
                rightOperand = temp;
            }
        }
        *out << unwrap(instr) << " " << sizeWord(getType(leftOperand)) << " " << unwrap(leftOperand) << ", " << sizeWord(getType(rightOperand)) << " " << unwrap(rightOperand) << "\n";
        return leftOperand;
    }
}
ExprNode x86Generator::printInstr(ExprNode instr, ExprNode l)
{
    *out << unwrap(instr) << " " << sizeWord(getType(l)) << " " << unwrap(l) << "\n";
}
int x86Generator::isTempInUse(ExprNode n)
{
    for(int i = 0; i<inUseTemp.size(); i++)
    {
        if(inUseTemp[i].isRegister())
        {
            if (n.isRegister())
            {
                Register other = std::get<Register>(n.value);
                Register toCheck = std::get<Register>(inUseTemp[i].value);
                if (other.qReg == toCheck.qReg) return i;
            }
        }
        else if (inUseTemp[i].isLiteral())
        {
            if (n.isLiteral())
            {
                Literal other = std::get<Literal>(n.value);
                Literal toCheck = std::get<Literal>(inUseTemp[i].value);
                if (other.val == toCheck.val) return i;
            }
        }
    }
    return -1;
}
std::string x86Generator::sizeWord(Type t)
{
    switch (t.size)
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
    switch (reg.type.size)
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

std::string x86Generator::chooseReg(Register reg, Type t)
{
    switch (t.size)
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
    case Div:
        return "idiv";
    case Modulo:
        return "idiv";
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
bool x86Generator::isCmp(Operator o)
{
    switch (o.tok.type)
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
    if (t.type == Number)
    {
        return t.value;
    }
    else if (t.type == EXPR_DEST)
    {
        if (t.value == "r10")
        {
            return chooseReg(r10);
        }
        else if (t.value == "r11")
        {
            return chooseReg(r11);
        }
        else if (t.value == "ptrR12")
        {
            return chooseReg(ptrR12);
        }
    }
    else if (t.type == EXPR_TMP)
    {
        return t.value;

    }
    else
    {
        auto s = st.find(t.value);
        if (curExpr != nullptr)
        {
            auto f = curExpr->exprFnTable.find(t.value);
            if (f != curExpr->exprFnTable.end())
            {
                f->second->accept(this);
                return chooseReg(rax);
            }
        }
        if (s != st.end())
        {
            return blib::asmVar(s->second);
        }
    }
    return "Undefined Identifier " + t.value;
}
Type x86Generator::getType(ExprNode n)
{
    if(n.isFuncCall())
    {
        FuncCall c = std::get<FuncCall>(n.value);
        return c.f.retType;
    }
    else if(n.isLiteral())
    {
        Literal l = std::get<Literal>(n.value);
        return l.type;
    }
    else if (n.isRegister())
    {
        Register r = std::get<Register>(n.value);
        return r.type;
    }
    else if (n.isVariableUse())
    {
        VariableUse var = std::get<VariableUse>(n.value);
        return var.v.type;
    }
}