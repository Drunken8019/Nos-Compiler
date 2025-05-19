#include "x86Generator.h"

x86Generator::x86Generator()
{
	this->out = NULL;
}

x86Generator::x86Generator(std::ofstream* out)
{
	this->out = out;
}

int x86Generator::calcVarOffset(int offset)
{
	return (offset * 8);
}

std::string x86Generator::resolveIdent(Token t)
{
	std::string result = "";
	auto f = varTable.find(t.value);
	if (f != varTable.end())
	{
		result.append("[rsp+");
		result.append(std::to_string(calcVarOffset(f->second)));
		result.append("]");
	}
	else
	{
		std::cout << std::endl;
		std::cout << "Symbol \"" << t.value << "\" not recognized" << std::endl;
		std::cout << "- Occured at line " << t.loc.line << ", column " << t.loc.column << std::endl;
	}
	return result;
}

void x86Generator::printDefaultHeader()
{
	*out <<
		"global main\n"
		"extern ExitProcess\n"
		"section .bss\n"
		"section .data\n"
		"section .text\n";
}

void x86Generator::printAsm(std::string s)
{
	*out << s;
	//out->write(s.c_str(), s.length()-1);
}

void x86Generator::printMov(Token des, Token src)
{
	if(src.type == TokenType::Number)
	{
		printMov(resolveIdent(des), src.value, "qword");
	}
	else if(src.type == TokenType::Identifier)
	{
		printMov("r11", resolveIdent(src), "");
		printMov(resolveIdent(des), "r11", "");
	}
}
void x86Generator::printMov(Token des, std::string src)
{
	printMov(resolveIdent(des), src, "qword");
}
void x86Generator::printMov(std::string des, Token src)
{
	if (src.type == TokenType::Identifier) printMov(des, resolveIdent(src), "qword");
	else printMov(des, src.value, "qword");
}
void x86Generator::printMov(std::string des, std::string src, std::string type)
{
	if (!type.empty()) type.append(" ");
	*out << "mov " << type << des << ", " << src << std::endl;
}

void x86Generator::printAddSubMul(std::string x86Operand, Token des, Token src)
{
	if (src.type == TokenType::Number)
	{
		printAddSubMul(x86Operand, resolveIdent(des), src.value, "qword");
	}
	else if (src.type == TokenType::Identifier)
	{
		printMov("r11", resolveIdent(src), "qword");
		printAddSubMul(x86Operand, resolveIdent(des), "r11", "qword");
		printMov(resolveIdent(des), "r11", "qword");
	}
}
void x86Generator::printAddSubMul(std::string x86Operand, Token des, std::string src)
{
	printAddSubMul(x86Operand, resolveIdent(des), src, "qword");
}
void x86Generator::printAddSubMul(std::string x86Operand, std::string des, Token src)
{
	if (src.type == TokenType::Identifier) printAddSubMul(x86Operand, des, resolveIdent(src), "qword");
	else printAddSubMul(x86Operand, des, src.value, "qword");
}
void x86Generator::printAddSubMul(std::string x86Operand, std::string des, std::string src, std::string type)
{
	if (!type.empty()) type.append(" ");
	*out << x86Operand << " " << type << des << ", " << src << std::endl;
}