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
		//std::cout << curToken.value << "\t" << curToken.type << "\t" << curToken.loc.line << " : " << curToken.loc.column << std::endl; //----DEBUG
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
		return d;
		break;
	}
	case TokenType::Extern:
	{
		Definition* d = new FuncDef(parseExternDef(stmnt));
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
	Type t;
	//-------------------- Syntax-Error handling --------------------
	if (stmnt[1].type != TokenType::Colon) { printErrorMsg("Expected \":\" after \"def\"", stmnt[1]); return { errTok }; }
	else
	{
		t = getType(stmnt[2]);
	}
	if (stmnt[3].type != TokenType::Identifier) { printErrorMsg("Expected identifier", stmnt[3]); return { errTok }; }
	else if (stmnt[4].type != TokenType::LParen) { printErrorMsg("Expected \"(\"", stmnt[4]); return { errTok }; }
	else if (stmnt[stmnt.size() - 2].type != TokenType::RParen) { printErrorMsg("Expected \")\"", stmnt[stmnt.size() - 2]); return { errTok }; }
	if(stmnt.back().type != TokenType::LCBrace) { printErrorMsg("Expected \"{\"", stmnt.back()); return { errTok }; }
	//---------------------------- END ------------------------------
	Variable v = { emptyTok, {} };
	FuncDef fd = { stmnt[3] };
	fd.func.retType = t;
	for (int i = 5; i < stmnt.size()-1; i++)
	{
		if (stmnt[i].type != TokenType::Comma && stmnt[i].type != TokenType::RParen)
		{
			v.type = getType(stmnt[i]);
			if(stmnt[i + 1].type == Asteriks)
			{
				v.type.isPtr = true;
				v.type.size = 8;
				stmnt.erase(stmnt.begin() + i + 1);
			}
			v.t = stmnt[i + 1];
			i++;
		}
		else
		{
			if(v.t.type != COMPILER_EMPTY)
			{
				fd.func.params.push_back(v);
			}
		}
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
	return fd;
}

FuncDef Parser::parseExternDef(std::vector<Token> stmnt)
{
	Type t;
	//-------------------- Syntax-Error handling --------------------
	if (stmnt[1].type != TokenType::Colon) { printErrorMsg("Expected \":\" after \"extern\"", stmnt[1]); return { errTok }; }
	else
	{
		t = getType(stmnt[2]);
	}
	if (stmnt[3].type != TokenType::Identifier) { printErrorMsg("Expected identifier", stmnt[3]); return { errTok }; }
	else if (stmnt[4].type != TokenType::LParen) { printErrorMsg("Expected \"(\"", stmnt[4]); return { errTok }; }
	else if (stmnt[stmnt.size() - 2].type != TokenType::RParen) { printErrorMsg("Expected \")\"", stmnt[stmnt.size() - 2]); return { errTok }; }
	if (stmnt.back().type != TokenType::Semicolon) { printErrorMsg("Expected \";\"", stmnt.back()); return { errTok }; }
	//---------------------------- END ------------------------------
	Variable v = { emptyTok, {} };
	FuncDef fd = { stmnt[3] };
	fd.func.retType = t;
	for (int i = 5; i < stmnt.size() - 1; i++)
	{
		if (stmnt[i].type != TokenType::Comma && stmnt[i].type != TokenType::RParen)
		{
			v.type = getType(stmnt[i]);
			if (stmnt[i + 1].type == Asteriks)
			{
				v.type.isPtr = true;
				v.type.size = 8;
				stmnt.erase(stmnt.begin() + i + 1);
			}
			v.t = stmnt[i + 1];
			i++;
		}
		else
		{
			if (v.t.type != COMPILER_EMPTY)
			{
				fd.func.params.push_back(v);
			}
		}
	}
	fd.func.isExtern = true;
	externs.push_back(fd.t.value);
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
	case TokenType::Asteriks:
		return new VarAssign(parseVarAsign(stmnt));
		break;
	default:
		printErrorMsg("\"" + stmnt[0].value + "\" is not a statement", stmnt[0]);
	}
	return new Statement(errTok);
}

VarDef Parser::parseVarDef(std::vector<Token> stmnt)
{
	Type t;
	//-------------------- Syntax-Error handling --------------------
	if (stmnt[1].type != TokenType::Colon) { printErrorMsg("Expected \":\" after \"let\"", stmnt[1]); return { {stmnt[3], {}}, {{errTok}} }; }
	else	
	{
		switch(stmnt[2].type)
		{
		case TokenType::Character:
			t.size = 1;
			t.name = "char";
			break;
		case TokenType::Short:
			t.size = 2;
			t.name = "short";
			break;
		case TokenType::Integer:
			t.size = 4;
			t.name = "int";
			break;
		case TokenType::Long:
			t.size = 8;
			t.name = "long";
			break;
		}
	}
	Variable v = { stmnt[3], {t} };
	Expression e;
	int i = 4;
	if (stmnt[3].type == TokenType::Asteriks) 
	{ 
		v.type.isPtr = true;
		v.type.size = 8;
		i++;
		v.t = stmnt[4];
		if (stmnt[4].type != TokenType::Identifier)
		{
			printErrorMsg("Expected identifier", stmnt[3]); 
			return { {stmnt[3], {}}, {{errTok}} };
		}
	}
	else if (stmnt[3].type != TokenType::Identifier) { printErrorMsg("Expected identifier", stmnt[3]); return { {stmnt[3], {}}, {{errTok}} }; }

	else if (stmnt[i].type == TokenType::Semicolon) 
	{ 
		return { {stmnt[i-1], {}}, {{emptyTok}}};
	}
	else if (stmnt[i].type != TokenType::Equals) { printErrorMsg("Expected \"=\" or \";\"", stmnt[i]); return { {stmnt[i-1], {}}, {{errTok}} }; }
	//---------------------------- END ------------------------------
	
	i++;
	for(; i<stmnt.size(); i++)
	{
		if (stmnt[i].type == TokenType::Semicolon) break;
		e.tokens.push_back(stmnt[i]);
	}
	return { v, e };
}

FuncCall Parser::parseFunctionCall(std::vector<Token> stmnt)
{
	FuncCall fc;
	fc = { stmnt[0] };
	if (stmnt[1].type ==TokenType::LParen && stmnt[2].type == TokenType::RParen) return fc;

	int i = 2;
	Expression e = {stmnt[i]};
	for (; i < stmnt.size(); i++)
	{
		if (stmnt[i].type == TokenType::RParen) 
		{
			fc.params.push_back(e);
			break;
		}
		if(stmnt[i].type != TokenType::Comma)
		{
			e.tokens.push_back(stmnt[i]);
		}
		else
		{
			fc.params.push_back(e);
			e.tokens.clear();
		}
	}
	return fc;
}

VarAssign Parser::parseVarAsign(std::vector<Token> stmnt)
{
	Expression e;
	VarAssign va = { stmnt[0], {} };
	if (stmnt[0].type == TokenType::Asteriks)
	{
		va.t = stmnt[1];
		va.isPtrAccess = true;
		stmnt.erase(stmnt.begin());
	}
	if (stmnt[1].type == TokenType::Semicolon) { printErrorMsg("Not a statement", stmnt[1]); return { errTok, {} }; }
	//else if (stmnt[1].type != TokenType::Equals) { printErrorMsg("Expected \"=\"", stmnt[1]); return { errTok, {} }; }
	switch(stmnt[1].type)
	{
	case Equals:
		e.resOperator = { Equals, "=", {} };
		break;
	case PlusEq:
		e.resOperator = { Plus, "+=", {} };
		break;
	case MinusEq:
		e.resOperator = { Minus, "-=", {} };
		break;
	case MultEq:
		e.resOperator = { Asteriks, "*=", {} };
		break;
	case DivEq:
		e.resOperator = { Div, "/=", {} };
		break;
	default:
		printErrorMsg("Expected assignment operator", stmnt[1]); 
		return { errTok, {} };
	}
	
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

Type Parser::getType(Token tok)
{
	Type t;
	switch (tok.type)
	{
	case TokenType::Character:
		t.size = 1;
		t.nonPointerSize = 1;
		t.name = "char";
		break;
	case TokenType::Short:
		t.size = 2;
		t.nonPointerSize = 2;
		t.name = "short";
		break;
	case TokenType::Integer:
		t.size = 4;
		t.nonPointerSize = 4;
		t.name = "int";
		break;
	case TokenType::Long:
		t.size = 8;
		t.nonPointerSize = 8;
		t.name = "long";
		break;
	}
	return t;
}