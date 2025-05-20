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
		std::cout << curToken.value << "\t" << curToken.type << "\t" << curToken.loc.line << " : " << curToken.loc.column << std::endl; //----DEBUG
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
			curToken = lex.nextToken();
		}
	}
	return result;
}

void Parser::parse()
{
	gen.printDefaultHeader();
	bool identified = false;
	std::vector<Token> stmnt = getStatement();

	while(!stmnt.empty())
	{
		identified = parseStatement(stmnt, {}, nullptr);
		stmnt = getStatement();
	}
	
}

bool Parser::parseStatement(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope, int *parCount)
{
	bool valid = true;
	switch (stmnt[0].type)
	{
	case TokenType::Define:
		valid = parseFunctionDef(stmnt, parScope, parCount);
		break;

	case TokenType::Let:
		valid = parseVarDef(stmnt, parScope, parCount);
		break;

	case TokenType::Identifier:
		valid = parseVarAsign(stmnt, parScope);
		break;

	case TokenType::Function:
		valid = parseFunctionCall(stmnt[0], *parScope);
		break;

	case TokenType::Return:
		valid = parseFuncReturn(stmnt, *parScope);
		break;

	case TokenType::Exit:
		valid = parseExit(stmnt, parScope);
		break;
	}
	return valid;
}

bool Parser::parseFunctionDef(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope, int *parCount)
{
	bool paramListClosed = false;
	bool functionBodyClosed = false;
	std::unordered_map<std::string, int> varTable;
	int varCount = 1;
	if (parCount != nullptr) { varCount = *parCount; }
	if (parScope != nullptr) { varTable = *parScope; }

	if (stmnt[1].type != TokenType::Function) { printErrorMsg("Expected identifier after \"def\"", stmnt[1]); return false; }
	//else if (stmnt[2].type != TokenType::LParen) { printErrorMsg("Expected \"(\"", stmnt[1]); return false; }

	//int i = 3;
	int i = 2;
	for(; i<stmnt.size(); i++)
	{
		//TODO: Compute param expr
		if(stmnt[i].type == TokenType::RParen)
		{
			paramListClosed = true;
			break;
		}
	}
	if(!paramListClosed){ printErrorMsg("Expected \")\"", stmnt[i]); return false; }

	if(i+1 < stmnt.size()) i++;
	if (stmnt[i].type != TokenType::LCBrace) { printErrorMsg("Expected \"{\"", stmnt[i]); return false; }

	gen.printAsm(stmnt[1].value + ":\n"); //TESTING
	if (stmnt[1].value == "main") //DEBUG, will be rmeoved
	{
		gen.printAsm("sub rsp, 360\n"); 
	}
	else { gen.printAsm("sub rsp, 320\n"); }

	std::vector<Token> nextStmnt = getStatement();
	while(!nextStmnt.empty())
	{
		if(nextStmnt.back().type == TokenType::RCBrace)
		{
			functionBodyClosed = true;
			break;
		}
		parseStatement(nextStmnt, &varTable, &varCount);
		nextStmnt = getStatement();
	}

	if (!functionBodyClosed) { printErrorMsg("Expected \"}\"", nextStmnt.back()); return false; }
	gen.printAsm("add rsp, 320\nret\n\n");
	return true;
}

bool Parser::parseVarDef(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope, int *varCount)
{
	if (stmnt[1].type != TokenType::Identifier) { printErrorMsg("Expected identifier after \"let\"", stmnt[1]); return false; }
	else if(parScope->find(stmnt[1].value) != parScope->end()) { printErrorMsg("\"" + stmnt[1].value + "\" is already defined", stmnt[1]); return false; }
	else if (stmnt[2].type == TokenType::Semicolon) 
	{ 
		parScope->insert({ stmnt[1].value, *varCount });
		*varCount += 1;
		return true; 
	}
	else if (stmnt[2].type != TokenType::Equals) { printErrorMsg("Expected \"=\"", stmnt[2]); return false; }

	parScope->insert({ stmnt[1].value, *varCount });

	std::vector<Token> expr;
	expr.assign(stmnt.begin() + 3, stmnt.end() - 1);

	compExpr(expr, "", stmnt[1], *parScope);
	*varCount += 1;

	return true;
}

bool Parser::parseFunctionCall(Token call, std::unordered_map<std::string, int> parScope)
{
	//TODO add a function Table
	gen.printAsm("call " + call.value + "\n");
	return true;
}

bool Parser::parseVarAsign(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope)
{
	if (stmnt[1].type == TokenType::Semicolon) { printErrorMsg("Not a statement", stmnt[1]); return false; }
	else if (stmnt[1].type != TokenType::Equals) { printErrorMsg("Expected \"=\"", stmnt[1]); return false; }

	std::vector<Token> expr;
	expr.assign(stmnt.begin()+2, stmnt.end()-1);
	compExpr(expr, "", stmnt[0], *parScope);
	//varCount++; why????
}

bool Parser::parseFuncReturn(std::vector<Token> stmnt, std::unordered_map<std::string, int> parScope)
{
	std::vector<Token> expr;
	expr.assign(stmnt.begin() + 1, stmnt.end() - 1);

	compExpr(expr, "[rsp]", emptyTok, parScope);
	gen.printMov("rax", "[rsp]", "qword", parScope);
	return true;
}

bool Parser::compExpr(std::vector<Token> expr, std::string x86Dest, Token dest, std::unordered_map<std::string, int> parScope)
{
	if (expr[0].type == TokenType::Function && expr.size() == 2)
	{
		parseFunctionCall(expr[0], parScope);
		//gen.printAsm("call " + expr[0].value + "\n");
		if (!x86Dest.empty()) gen.printMov(x86Dest, "rax", "qword", parScope);
		else gen.printMov(dest, "rax", parScope);
		return true;
	}

	if(expr.size() == 1)
	{
		if(!x86Dest.empty()) gen.printMov(x86Dest, expr[0], parScope);
		else gen.printMov(dest, expr[0], parScope);
		return true;
	}

	//ONLY 3 Operator expr supported rn: left OPERATOR right
	//if (expr.size() > 3) { printErrorMsg("ONLY 3 Operator expr supported: left OPERATOR right", expr[0]); return false; }
	std::vector<Token> lr;
	Token op = emptyTok;
	for(Token t : expr)
	{
		if(t.type == TokenType::Function && lr.size() < 2)
		{
			lr.push_back(t);
		}
		else if (t.type == TokenType::Identifier && lr.size() < 2)
		{
			lr.push_back(t);
		}
		else if (t.type == TokenType::Number && lr.size() < 2)
		{
			lr.push_back(t);
		}
		else if (t.type == TokenType::Plus || t.type == TokenType::Minus || t.type == TokenType::Mult || t.type == TokenType::Div)
		{
			if(op.type == TokenType::COMPILER_EMPTY) op = t;
			else
			{
				printErrorMsg("Only 1 Operand per expressions supported for now ", expr.front());
				return false;
			}
		}
	}

	if (lr.size() != 2) { printErrorMsg("Only 2 Values and 1 Operand expressions supported for now ", expr.front()); return false; }
	switch (op.type)
	{
	case TokenType::Plus:
		compTAC(lr[0], lr[1], x86Dest, dest, "add", parScope);
		break;
	case TokenType::Minus:
		compTAC(lr[0], lr[1], x86Dest, dest, "sub", parScope);
		break;
	case TokenType::Mult:
		compTAC(lr[0], lr[1], x86Dest, dest, "imul", parScope);
		break;
	case TokenType::Div:
		break;
	}
	return true;
}

bool Parser::compTAC(Token l, Token r, std::string x86Dest, Token dest, std::string x86Operand, std::unordered_map<std::string, int> parScope)
{
	if (!x86Dest.empty())
	{
		if (l.type == TokenType::Function) parseFunctionCall(l, parScope);
		gen.printMov(x86Dest, l, parScope);
		if (r.type == TokenType::Function) parseFunctionCall(r, parScope);
		gen.printAddSubMul(x86Operand, x86Dest, r, parScope); 
	}
	else 
	{ 
		if (l.type == TokenType::Function) parseFunctionCall(l, parScope);
		gen.printMov(dest, l, parScope);
		if (r.type == TokenType::Function) parseFunctionCall(r, parScope);
		gen.printAddSubMul(x86Operand, dest, r, parScope); 
	}

	return true;
}

bool Parser::parseExit(std::vector<Token> stmnt, std::unordered_map<std::string, int> *parScope)
{
	std::vector<Token> expr;
	expr.assign(stmnt.begin() + 1, stmnt.end() - 1);
	compExpr({ expr }, "rcx", {}, *parScope);
	gen.printAsm("call ExitProcess\n");
	return true;
}

void Parser::printErrorMsg(std::string msg, Token t)
{
	std::cout << std::endl;
	std::cout << msg << std::endl;
	std::cout << "- Occured at line " << t.loc.line << " and column " << t.loc.column << std::endl;
}