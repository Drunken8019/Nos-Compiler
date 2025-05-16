#include "Parser.h"
#include "x86Generator.h"

Parser::Parser(Lexer l)
{
	this->lex = l;
}

void Parser::parse()
{
	while(lex.hasNext)
	{
		Token curToken = lex.nextToken();
		switch(curToken.type)
		{
		case TokenType::Identifier:
			if(lex.hasNext)
			{
				if(lex.nextToken().type == TokenType::Colon)
				{
				}
			}
			break;
		}
	}
}