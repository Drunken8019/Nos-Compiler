#include "x86Generator.h"

x86Generator::x86Generator()
{
	this->out = NULL;
}

x86Generator::x86Generator(std::ofstream* out)
{
	this->out = out;
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

void x86Generator::startMatching(Lexer l)
{
	while (l.hasNext)
	{
		std::string toPrint = "";
		Token curToken = l.nextToken();
		std::vector<Token> tkInCurExpr;
		bool syntaxMatching = true;
		for(SyntaxTemplate t : syntax)
		{
			for (Snippet s : t.def)
			{
				if (s.isToken)
				{
					if (curToken.type == s.getType())
					{
						if (curToken.type == TokenType::COMPILER_EOF) { std::cout << "Unexpected eof"; return; } //---DEBUG
						syntaxMatching = true;
						tkInCurExpr.push_back(curToken);
						curToken = l.nextToken();
					}
					else
					{
						if (curToken.type == TokenType::COMPILER_EOF) { std::cout << "Unexpected eof"; return; } //---DEBUG
						syntaxMatching = false;
						curToken = l.nextToken();
						break;
					}
				}
			}
			if (syntaxMatching) toPrint = t.x86Code(tkInCurExpr);
			tkInCurExpr.clear();
		}
		if (!toPrint.empty()) *out << toPrint;
	}
}