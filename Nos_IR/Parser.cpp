#include "Parser.h"

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

void Parser::parse() //When implementing OOP, this will be Class level... The parse Root will be moved 1 up then
{
	Root root;
	std::vector<int> scopes;
	std::vector<Token> stmnt = getStatement();
	if(stmnt.empty()) { printErrorMsg("Empty File", {TokenType::COMPILER_EOF, "", {0, 0}}); return; }
	else
	{
		root = parseRoot(stmnt);
	}
	
	//resolveAST(&root);
	res.resolveAST(&root);
	gen.printAST(root, externs);
}

Root Parser::parseRoot(std::vector<Token> stmnt)
{
	Root res = { stmnt[0] };
	std::vector<Token> nextStmnt = stmnt;
	while (!nextStmnt.empty())
	{
		if (nextStmnt[nextStmnt.size() - 1].type == TokenType::RCBrace) { break; }

		Definition* d = parseDefinition(nextStmnt);
		if (d->t.type != TokenType::COMPILER_ERROR) res.defs.push_back(d);
		nextStmnt = getStatement();
	}
	return res;
}

ClassDefin Parser::parseClassDef(std::vector<Token> stmnt)
{
	//-------------------- Syntax-Error handling --------------------
	if (stmnt[1].type != TokenType::Identifier) { printErrorMsg("Expected identifier after \"class\"", stmnt[1]); return {errTok}; }
	else if (stmnt[2].type != TokenType::LCBrace) { printErrorMsg("Expected \"{\"", stmnt[1]); return {errTok}; }
	//---------------------------- END ------------------------------

	ClassDefin res = { stmnt[1] };
	std::vector<Token> nextStmnt = getStatement();
	while (!nextStmnt.empty())
	{
		if (nextStmnt[nextStmnt.size() - 1].type == TokenType::RCBrace) { break; }

		Definition* d = parseDefinition(nextStmnt);
		//d->resolve(new Class()); SLICED
		if (d->t.type != TokenType::COMPILER_ERROR) res.defs.push_back(d);
		nextStmnt = getStatement();
	}
	return res;
}

Definition* Parser::parseDefinition(std::vector<Token> stmnt)
{
	switch (stmnt[0].type)
	{
	case TokenType::Let:
	{
		Definition* d = new VarDef(parseVarDef(stmnt));
		
		return d;
		break;
	}

	case TokenType::Define:
	{
		Definition* d = new FuncDef(parseFunctionDef(stmnt));
		//d->resolve(new Class()); SLICED
		return d;
		break;
	}
	default:
		//-------------------- Syntax-Error handling --------------------
		printErrorMsg("Only variable definitions or function definitions allowed", stmnt[0]);
		//---------------------------- END ------------------------------
		Definition* d = new Definition(errTok);
		return d;
		break;
	}
}

FuncDef Parser::parseFunctionDef(std::vector<Token> stmnt)
{
	//-------------------- Syntax-Error handling --------------------
	if (stmnt[1].type != TokenType::Identifier) { printErrorMsg("Expected identifier after \"def\"", stmnt[1]); return { errTok }; }
	else if (stmnt[2].type != TokenType::LParen) { printErrorMsg("Expected \"(\"", stmnt[1]); return { errTok }; }
	else if (stmnt[stmnt.size() - 2].type != TokenType::RParen) { printErrorMsg("Expected \")\"", stmnt[stmnt.size() - 2]); return { errTok }; }
	//---------------------------- END ------------------------------
	Expression e;
	FuncDef fd = { stmnt[1] };
	for (int i = 3; i < stmnt.size(); i++)
	{
		if (stmnt[i].type != TokenType::Comma && stmnt[i].type != TokenType::RParen)
		{
			e.tokens.push_back(stmnt[i]);
		}
		else
		{
			fd.params.push_back(e);
		}
		//fd.params.push_back(e);
	}

	std::vector<Token> nextStmnt = getStatement();
	while (!nextStmnt.empty())
	{
		if (nextStmnt.back().type == TokenType::RCBrace)
		{
			break;
		}
		Statement* s = parseStatement(nextStmnt);
		fd.statements.push_back(s);
		nextStmnt = getStatement();
	}
	//fd.resolve(new Class()); FINE
	return fd;
}

Statement* Parser::parseStatement(std::vector<Token> stmnt)
{
	switch (stmnt[0].type)
	{
	case TokenType::Let:
		return new VarDef(parseVarDef(stmnt));
		break;

	case TokenType::Identifier:
		if (stmnt[1].type == TokenType::LParen) return new FuncCall(parseFunctionCall(stmnt));
		else return new VarAssign(parseVarAsign(stmnt));
		break;

	case TokenType::Extern:
		return new FuncCall(parseFunctionCall(stmnt));
		break;
	case TokenType::Return:
		return new ReturnCall(parseFuncReturn(stmnt));
		break;
	case TokenType::If:
		return new IfStmnt(parseIfStmnt(stmnt));
		break;
	case TokenType::Elif:
		printErrorMsg("elif must be preceeded by if or elif", stmnt[0]);
		break;
	case TokenType::Else:
		printErrorMsg("else must be preceeded by if or elif", stmnt[0]);
		break;
	case TokenType::While:
		return new WhileStmnt(parseWhileStmnt(stmnt));
		break;
	default:
		printErrorMsg("\"" + stmnt[0].value + "\" is not a statement", stmnt[0]);
	}
	return new Statement(errTok);
}

VarDef Parser::parseVarDef(std::vector<Token> stmnt)
{
	//-------------------- Syntax-Error handling --------------------
	if (stmnt[1].type != TokenType::Identifier) { printErrorMsg("Expected identifier after \"let\"", stmnt[1]); return { {stmnt[1], 8}, {{errTok}} }; }
	else if (stmnt[2].type == TokenType::Semicolon) 
	{ 
		return { {stmnt[1], 8}, {{emptyTok}} };
	}
	else if (stmnt[2].type != TokenType::Equals) { printErrorMsg("Expected \"=\"", stmnt[2]); return { {stmnt[1], 8}, {{errTok}} }; }
	//---------------------------- END ------------------------------
	Expression e;
	int i = 3;
	for(; i<stmnt.size(); i++)
	{
		if (stmnt[i].type == TokenType::Semicolon) break;
		e.tokens.push_back(stmnt[i]);
	}
	return { {stmnt[1], 8}, e};
}

FuncCall Parser::parseFunctionCall(std::vector<Token> stmnt)
{
	FuncCall fc;
	if(stmnt[0].type == TokenType::Extern)
	{
		fc = { stmnt[1] };
		fc.isExtern = true;
		externs.push_back(stmnt[1].value);
		stmnt.erase(stmnt.begin());
	}
	else
	{
		fc = { stmnt[0] };
	}
	if (stmnt[2].type == TokenType::RParen) return fc;

	int i = 2;
	Expression e;
	for(; i<stmnt.size(); i++)
	{
		if(stmnt[i].type != TokenType::Comma)
		{
			e.tokens.push_back(stmnt[i]);
		}
		else
		{
			fc.params.push_back(e);
		}
	}
	return fc;
}

VarAssign Parser::parseVarAsign(std::vector<Token> stmnt)
{
	
	if (stmnt[1].type == TokenType::Semicolon) { printErrorMsg("Not a statement", stmnt[1]); return { errTok, {} }; }
	else if (stmnt[1].type != TokenType::Equals) { printErrorMsg("Expected \"=\"", stmnt[1]); return { errTok, {} }; }
	Expression e;
	VarAssign va = { stmnt[0], e };
	int i = 2;
	for(; i<stmnt.size(); i++)
	{
		if (stmnt[i].type == TokenType::Semicolon) break;
		e.tokens.push_back(stmnt[i]);
	}
	va.t = stmnt[0];
	va.expr = e;
	return va;
}

ReturnCall Parser::parseFuncReturn(std::vector<Token> stmnt)
{
	Expression e;
	ReturnCall rc = { stmnt[0], e };
	for(int i = 1; i<stmnt.size(); i++)
	{
		if (stmnt[i].type == TokenType::Semicolon) break;
		e.tokens.push_back(stmnt[i]);
	}
	rc.expr = e;
	return rc;
}

IfStmnt Parser::parseIfStmnt(std::vector<Token> stmnt)
{
	IfStmnt res = { stmnt[0] };
	if (stmnt[1].type != TokenType::LParen) { printErrorMsg("Expected \"(\" after if", stmnt[1]); return {errTok}; }
	int i = 2;
	for(; i<stmnt.size(); i++)
	{
		if(stmnt[i].type == TokenType::RParen)
		{
			break;
		}
		res.cond.tokens.push_back(stmnt[i]);
	}
	i++;
	if (stmnt[i].type != TokenType::LCBrace) { printErrorMsg("Expected \"{\" before if-body", stmnt[i]); return { errTok }; }
	std::vector<Token> nextStmnt = getStatement();
	while (!nextStmnt.empty())
	{
		if (nextStmnt.back().type == TokenType::RCBrace)
		{
			break;
		}
		Statement* s = parseStatement(nextStmnt);
		res.body.push_back(s);
		nextStmnt = getStatement();
	}
	if(lex.peek().type == TokenType::Elif)
	{
		nextStmnt = getStatement();
		res.next = new ElIfStmnt(parseElIfStmnt(nextStmnt));
	}
	else if(lex.peek().type == TokenType::Else)
	{
		nextStmnt = getStatement();
		res.next = new ElseStmnt(parseElseStmnt(nextStmnt));
	}
	return res;
}

ElIfStmnt Parser::parseElIfStmnt(std::vector<Token> stmnt)
{
	ElIfStmnt res = stmnt[0];
	if (stmnt[1].type != TokenType::LParen) { printErrorMsg("Expected \"(\" after elif", stmnt[1]); return { errTok }; }
	int i = 2;
	for (; i < stmnt.size(); i++)
	{
		if (stmnt[i].type == TokenType::RParen)
		{
			break;
		}
		res.cond.tokens.push_back(stmnt[i]);
	}
	i++;
	if (stmnt[i].type != TokenType::LCBrace) { printErrorMsg("Expected \"{\" before elif-body", stmnt[i]); return { errTok }; }
	std::vector<Token> nextStmnt = getStatement();
	while (!nextStmnt.empty())
	{
		if (nextStmnt.back().type == TokenType::RCBrace)
		{
			break;
		}
		Statement* s = parseStatement(nextStmnt);
		res.body.push_back(s);
		nextStmnt = getStatement();
	}

	if (lex.peek().type == TokenType::Elif)
	{
		nextStmnt = getStatement();
		res.next = new ElIfStmnt(parseElIfStmnt(nextStmnt));
	}
	else if (lex.peek().type == TokenType::Else)
	{
		nextStmnt = getStatement();
		res.next = new ElseStmnt(parseElseStmnt(nextStmnt));
	}
	return res;
}

ElseStmnt Parser::parseElseStmnt(std::vector<Token> stmnt)
{
	ElseStmnt res = stmnt[0];
	if(stmnt[1].type != TokenType::LCBrace) { printErrorMsg("Expected \"{\" before else-body", stmnt[1]); return { errTok }; }
	std::vector<Token> nextStmnt = getStatement();
	while (!nextStmnt.empty())
	{
		if (nextStmnt.back().type == TokenType::RCBrace)
		{
			break;
		}
		Statement* s = parseStatement(nextStmnt);
		res.body.push_back(s);
		nextStmnt = getStatement();
	}
	return res;
}

WhileStmnt Parser::parseWhileStmnt(std::vector<Token> stmnt)
{
	WhileStmnt res = { stmnt[0] };
	if (stmnt[1].type != TokenType::LParen) { printErrorMsg("Expected \"(\" after while", stmnt[1]); return { errTok }; }
	int i = 2;
	for (; i < stmnt.size(); i++)
	{
		if (stmnt[i].type == TokenType::RParen)
		{
			break;
		}
		res.cond.tokens.push_back(stmnt[i]);
	}
	i++;
	if (stmnt[i].type != TokenType::LCBrace) { printErrorMsg("Expected \"{\" before while-body", stmnt[i]); return { errTok }; }
	std::vector<Token> nextStmnt = getStatement();
	while (!nextStmnt.empty())
	{
		if (nextStmnt.back().type == TokenType::RCBrace)
		{
			break;
		}
		Statement* s = parseStatement(nextStmnt);
		res.body.push_back(s);
		nextStmnt = getStatement();
	}
	return res;
}

void Parser::printErrorMsg(std::string msg, Token t)
{
	std::cout << std::endl;
	std::cout << msg << std::endl;
	std::cout << "- Occured at line " << t.loc.line << " and column " << t.loc.column << std::endl;
}