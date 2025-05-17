#include "Parser.h"
#include "x86Generator.h"

Parser::Parser(std::ifstream* in, std::ofstream* out)
{
	this->lex = { in };
	this->gen = { out };
}

void Parser::parse()
{
	gen.printDefaultHeader();
	gen.startMatching(lex);
	/*while (lex.hasNext)
	{
		Token curToken = lex.nextToken();
		Token aheadToken;
		if (lex.hasNext) aheadToken = lex.peek();

		switch(curToken.type)
		{
		case TokenType::Identifier:
			switch(aheadToken.type)
			{
			case TokenType::Colon:
				gen.printAsm(curToken.value + ":\n");
				break;

			case TokenType::Equals:
				break;
			}
			break;

		case TokenType::Return:
			gen.printAsm("sub rsp, 40\n");
			gen.printAsm("mov rcx, " + aheadToken.value + "\n");
			gen.printAsm("call ExitProcess");
			break;
		}
		std::cout << curToken.value << "\t" << curToken.type << "\t" << curToken.loc.line << " : " << curToken.loc.column << std::endl; //----DEBUG
	}*/
}