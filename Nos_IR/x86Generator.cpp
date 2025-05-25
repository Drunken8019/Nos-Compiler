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

bool x86Generator::printAST(ClassDefin root)
{
	printDefaultHeader();
	for(Definition *d : root.defs)
	{
		d->accept(this);
	}
	return true;
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

void x86Generator::visit(Expression* node, std::string des)
{
	std::string res = "";
	switch (node->tokens.front().type)
	{
	case TokenType::Number:
		res = "mov qword " + des + ", " + node->tokens.front().value + "\n";
		break;
	case TokenType::Identifier:
	{
		auto f = ft.find(node->tokens.front().value);
		auto s = st.find(node->tokens.front().value);
		if (f != ft.end())
		{
			res.append("call " + f->second.t.value + "\n");
			res.append("mov qword " + des + ", rax\n");
		}
		else if (s != st.end())
		{
			res = "mov qword r11, " + blib::asmVar(s->second) + "\n";
			res.append("mov qword " + des + ", r11\n");
		}
		break;
	}
	default:
		break;
	}
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
	node->expr.des = "[rsp]";
	node->expr.accept(this);

	if (curFunc.t.value == "main")
	{
		res.append("mov rcx, [rsp]\n");
		res.append("call ExitProcess\n");
	}
	else res.append("mov rax, [rsp]\n");
	int reqSize = curFunc.stackSize % 16;
	reqSize += curFunc.stackSize;
	res.append("add rsp, " + std::to_string(reqSize) + "\n");
	res.append("ret\n");
	*out << res;
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
	node->expr.accept(this);
	return;
}
void x86Generator::visit(FuncDef* node)
{
	std::string res = "";
	res.append(node->func.t.value + ":\n");
	int reqSize = node->func.stackSize % 16;
	reqSize += node->func.stackSize;
	if (node->func.t.value == "main") reqSize += 40;
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