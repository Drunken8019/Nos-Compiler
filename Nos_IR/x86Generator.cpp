#include "x86Generator.h"

x86Generator::x86Generator()
{
	this->out = NULL;
}

x86Generator::x86Generator(std::ofstream* out)
{
	this->out = out;
}

int x86Generator::calcVarOffset(int offset, int scope, std::vector<int> scopes)
{
	int scopeOff = sumVector(scopes, scopes.size() - scope);
	return (offset * 8) + scopeOff;
}

int x86Generator::sumVector(std::vector<int> v, int end)
{
	int result = 0;
	for(int i=0;i<=end;i++)
	{
		result += v[i];
	}
	return result;
}

std::string x86Generator::resolveIdent(Token t, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	std::string result = "";
	auto f = varTable.find(t.value);
	if (f != varTable.end())
	{
		result.append("[rsp+");
		result.append(std::to_string(calcVarOffset(f->second.count, f->second.scopeElevation, scopes)));
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

void x86Generator::printMov(Token des, Token src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	if(src.type == TokenType::Number)
	{
		printMov(resolveIdent(des, varTable, scopes), src.value, "qword", varTable, scopes);
	}
	else if(src.type == TokenType::Identifier)
	{
		printMov("r11", resolveIdent(src, varTable, scopes), "", varTable, scopes);
		printMov(resolveIdent(des, varTable, scopes), "r11", "", varTable, scopes);
	}
	else if (src.type == TokenType::Function)
	{
		printMov(resolveIdent(des, varTable, scopes), "rax", "qword", varTable, scopes);
	}
}
void x86Generator::printMov(Token des, std::string src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	printMov(resolveIdent(des, varTable, scopes), src, "qword", varTable, scopes);
}
void x86Generator::printMov(std::string des, Token src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	if (src.type == TokenType::Identifier) printMov(des, resolveIdent(src, varTable, scopes), "qword", varTable, scopes);
	else if (src.type == TokenType::Function)
	{
		printMov(des, "rax", "qword", varTable, scopes);
	}
	else printMov(des, src.value, "qword", varTable, scopes);
}
void x86Generator::printMov(std::string des, std::string src, std::string type, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	if (!type.empty()) type.append(" ");
	*out << "mov " << type << des << ", " << src << std::endl;
}

void x86Generator::printAddSubMul(std::string x86Operand, Token des, Token src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	if (src.type == TokenType::Number)
	{
		printAddSubMul(x86Operand, resolveIdent(des, varTable, scopes), src.value, "qword", varTable, scopes);
	}
	else if (src.type == TokenType::Identifier)
	{
		printMov("r11", resolveIdent(src, varTable, scopes), "qword", varTable, scopes);
		printAddSubMul(x86Operand, resolveIdent(des, varTable, scopes), "r11", "qword", varTable, scopes);
		printMov(resolveIdent(des, varTable, scopes), "r11", "qword", varTable, scopes);
	}
	else if(src.type == TokenType::Function)
	{
		printAddSubMul(x86Operand, resolveIdent(des, varTable, scopes), "rax", "qword", varTable, scopes);
	}
}
void x86Generator::printAddSubMul(std::string x86Operand, Token des, std::string src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	printAddSubMul(x86Operand, resolveIdent(des, varTable, scopes), src, "qword", varTable, scopes);
}
void x86Generator::printAddSubMul(std::string x86Operand, std::string des, Token src, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	if (src.type == TokenType::Identifier) printAddSubMul(x86Operand, des, resolveIdent(src, varTable, scopes), "qword", varTable, scopes);
	else if (src.type == TokenType::Function)
	{
		printAddSubMul(x86Operand, des, "rax", "qword", varTable, scopes);
	}
	else printAddSubMul(x86Operand, des, src.value, "qword", varTable, scopes);
}
void x86Generator::printAddSubMul(std::string x86Operand, std::string des, std::string src, std::string type, std::unordered_map<std::string, Variable> varTable, std::vector<int> scopes)
{
	if (!type.empty()) type.append(" ");
	*out << x86Operand << " " << type << des << ", " << src << std::endl;
}