#include "Parser.h"
#include "x86Generator.h"

Parser::Parser(std::ifstream* in, std::ofstream* out)
{
	this->lex = { in };
	this->gen = { out };
}

std::vector<Token> Parser::getStatement()
{
	Token curToken = lex.nextToken();
	std::vector<Token> result;
	while (curToken.type != TokenType::COMPILER_EOF)
	{
		switch (curToken.type)
		{
		case TokenType::LCBrace:
			result.push_back(curToken);
			return result;

		case TokenType::RCBrace:
			result.push_back(curToken);
			return result;

		case TokenType::Semicolon:
			result.push_back(curToken);
			return result;

		default:
			result.push_back(curToken);
			//std::cout << curToken.value << "\t" << curToken.type << "\t" << curToken.loc.line << " : " << curToken.loc.column << std::endl; //----DEBUG
			curToken = lex.nextToken();
		}
	}
	return result;
}

void Parser::parse()
{
	gen.printDefaultHeader();
	//gen.startMatching(lex);
	bool identified = false;
	std::vector<Token> stmnt = getStatement();

	while(!stmnt.empty())
	{
		identified = parseStatement(stmnt);
		stmnt = getStatement();
	}
	
}

bool Parser::parseStatement(std::vector<Token> stmnt)
{
	bool identified = false;
	switch (stmnt[0].type)
	{
	case TokenType::Define:
		identified = parseFunctionDef(stmnt);
		break;

	case TokenType::Let:
		identified = parseVarDef(stmnt);
		break;
	}
	return identified;
}

bool Parser::parseFunctionDef(std::vector<Token> stmnt)
{
	bool paramListClosed = false;
	bool functionBodyClosed = false;
	
	if (stmnt[1].type != TokenType::Identifier) { std::cout << "Expected identifier after \"def\"" << std::endl; return false; }
	else if (stmnt[2].type != TokenType::LParen) { std::cout << "Expected \"(\"" << std::endl; return false; }
	int i = 3;
	for(; i<stmnt.size(); i++)
	{
		//TODO: Compute param expr
		if(stmnt[i].type == TokenType::RParen)
		{
			paramListClosed = true;
			break;
		}
	}
	if(!paramListClosed){ std::cout << "Expected \")\"" << std::endl; return false; }

	if(i+1 < stmnt.size()) i++;
	if (stmnt[i].type != TokenType::LCBrace) { std::cout << "Expected \"{\"" << std::endl; return false; }

	std::vector<Token> nextStmnt = getStatement();
	while(!nextStmnt.empty())
	{
		//TODO: Compute function body
		if(nextStmnt[0].type == TokenType::RCBrace)
		{
			functionBodyClosed = true;
			break;
		}
		nextStmnt = getStatement();
	}

	if (!functionBodyClosed) { std::cout << "Expected \"}\"" << std::endl; return false; }

	std::cout << stmnt[1].value << ":" << std::endl;

	return true;
}

bool Parser::parseVarDef(std::vector<Token> stmnt)
{
	if (stmnt[1].type != TokenType::Identifier) { std::cout << "Expected identifier after \"let\"" << std::endl; return false; }
	else if (stmnt[2].type != TokenType::Equals) { std::cout << "Expected \"=\"" << std::endl; return false; }
	int i = 3;
	for (; i < stmnt.size(); i++)
	{
		//TODO: Compute expr
	}
	return true;
}