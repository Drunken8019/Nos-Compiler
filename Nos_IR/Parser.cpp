#include "Parser.h"
#include "x86Generator.h"

Parser::Parser(std::ifstream* in, std::ofstream* out)
{
	this->lex = { in };
	this->gen = { out };
}

int Parser::calcVarOffset(int offset)
{
	return (offset * 8);
}

std::string Parser::resolveIdent(Token t)
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
		std::cout << "Symbol not recognized. At line " << t.loc.line << ", col " << t.loc.column << std::endl;
	}
	return result;
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
	bool valid = true;
	switch (stmnt[0].type)
	{
	case TokenType::Define:
		valid = parseFunctionDef(stmnt);
		break;

	case TokenType::Let:
		valid = parseVarDef(stmnt);
		break;

	case TokenType::Identifier:
		if (parseFunctionCall(stmnt)) valid = true;
		else if (parseVarAsign(stmnt)) valid = true;
		else valid = false;
		break;

	case TokenType::Exit:
		valid = parseExit(stmnt);
		break;
	}
	return valid;
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

	gen.x86Code({ { {stmnt[1]}, {":\n"} } }); //TESTING
	if (stmnt[1].value == "main") //DEBUG, will be rmeoved
	{
		gen.printAsm("\tsub rsp, 360\n"); 
	} 

	std::vector<Token> nextStmnt = getStatement();
	while(!nextStmnt.empty())
	{
		if(nextStmnt.back().type == TokenType::RCBrace)
		{
			functionBodyClosed = true;
			break;
		}
		parseStatement(nextStmnt);
		nextStmnt = getStatement();
	}

	if (!functionBodyClosed) { std::cout << "Expected \"}\"" << std::endl; return false; }
	//std::cout << stmnt[1].value << ":" << std::endl;
	return true;
}

bool Parser::parseVarDef(std::vector<Token> stmnt)
{
	if (stmnt[1].type != TokenType::Identifier) { std::cout << "Expected identifier after \"let\"" << std::endl; return false; }
	else if(varTable.find(stmnt[1].value) != varTable.end()) { std::cout << "\"" << stmnt[1].value << "\" is already defined" << std::endl; return false; }
	else if (stmnt[2].type == TokenType::Semicolon) 
	{ 
		varTable.insert({ stmnt[1].value, varCount });
		varCount++;
		return true; 
	}
	else if (stmnt[2].type != TokenType::Equals) { std::cout << "Expected \"=\"" << std::endl; return false; }

	varTable.insert({ stmnt[1].value, varCount });

	std::vector<Token> expr;
	expr.assign(stmnt.begin() + 3, stmnt.end() - 1);


	compExpr(expr, resolveIdent(stmnt[1]));
	varCount++;
	
	//TODO: Compute expr

	return true;
}

bool Parser::parseFunctionCall(std::vector<Token> stmnt)
{
	if (stmnt[1].type != TokenType::LParen) return false;
	else if(stmnt[2].type != TokenType::RParen) return false;
	gen.printAsm("call " + stmnt[0].value);
}

bool Parser::parseVarAsign(std::vector<Token> stmnt)
{
	if (stmnt[1].type == TokenType::Semicolon) { std::cout << "Not a statement" << std::endl; return false; }
	else if (stmnt[1].type != TokenType::Equals) { std::cout << "Expected \"=\"" << std::endl; return false; }

	std::vector<Token> expr;
	expr.assign(stmnt.begin()+2, stmnt.end()-1);
	compExpr(expr, resolveIdent(stmnt[0]));
	varCount++;
}

bool Parser::compExpr(std::vector<Token> expr, std::string x86Dest)
{
	if(expr.size() == 1)
	{
		switch (expr[0].type)
		{
		case TokenType::Number:
			gen.printAsm("\tmov qword " + x86Dest + ", " + expr[0].value + "\n");
			break;
		case TokenType::Identifier:
			gen.printAsm("\tmov r11, " + resolveIdent(expr[0]) + "\n");
			gen.printAsm("\tmov " + x86Dest + ", r11\n");
			break;
		}
		return true;
	}
	//ONLY 3 Operator expr supported rn: left OPERATOR right
	if (expr.size() > 3) { std::cout << "ONLY 3 Operator expr supported: left OPERATOR right"; return false; }
	switch(expr[1].type)
	{
	case TokenType::Plus:
		compSMA(expr[0], expr[2], x86Dest, "add");
		break;
	case TokenType::Minus:
		compSMA(expr[0], expr[2], x86Dest, "sub");
		break;
	case TokenType::Mult:
		compSMA(expr[0], expr[2], x86Dest, "imul");
		break;
	case TokenType::Div:
		break;
	}
	return true;
}

bool Parser::compSMA(Token l, Token r, std::string x86Dest, std::string x86Operand)
{
	//Potential optimization: e.g. 5+3 = 8, jus to fucking lazy rn
	/*if (l.type == TokenType::Number && l.type == TokenType::Number)
	{
		long int t = std::stol(l.value) + std::stol(r.value);
		gen.printAsm("mov " + x86Dest + ", " + std::to_string(t));
	}
	else
	{
		std::string lRes = l.value, rRes = r.value;
		if (l.type == TokenType::Identifier) lRes = resolveIdent(l);
		if (r.type == TokenType::Identifier) rRes = resolveIdent(r);

		gen.printAsm("mov rdx, " + lRes);
		gen.printAsm("add rdx, " + rRes);
		gen.printAsm("mov " + x86Dest + "rdx");
	}*/

	std::string lRes = l.value, rRes = r.value;
	if (l.type == TokenType::Identifier) lRes = resolveIdent(l);
	if (r.type == TokenType::Identifier) rRes = resolveIdent(r);

	gen.printAsm("\tmov rdx, " + lRes + "\n");
	gen.printAsm("\t" + x86Operand + " rdx, " + rRes + "\n");
	gen.printAsm("\tmov " + x86Dest + ", rdx" + "\n");
	return true;
}

bool Parser::parseExit(std::vector<Token> stmnt)
{
	if (stmnt[1].type == TokenType::Number) 
	{
		gen.printAsm("\tmov rcx, " + stmnt[1].value + "\n\tcall ExitProcess\n"); //TESTING
	}
	else if(stmnt[1].type == TokenType::Identifier)
	{
		gen.printAsm("\tmov rcx, " + resolveIdent(stmnt[1]) + "\n\tcall ExitProcess\n");
	}
	return true;
}