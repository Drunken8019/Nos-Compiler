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
	lex.initialize();
	if(lex.peek().type == COMPILER_EOF) { printErrorMsg("Empty File", {TokenType::COMPILER_EOF, "", {0, 0}}); return; }
	
	Root root = parseRoot();
	
	res.resolveAST(&root);
	tcheck.visit(&root);
	gen.printAST(root, externs);
}

Root Parser::parseRoot()
{
	Root res = Root(lex.peek());
	res.body = parseDefinBodyHeadless();
	return res;
}

Body Parser::parseBody()
{
	Body res = Body(consume(LCBrace, "Expected '{'"));
	
	while(!check(RCBrace))
	{
		res.statements.push_back(parseStatement());
	}
	consume(RCBrace, "Expected '}'");
	return res;
}

DefinBody Parser::parseDefinBody()
{
	DefinBody res = DefinBody(consume(LCBrace, "Expected '{'"));

	while (!check(RCBrace))
	{
		res.definitions.push_back(parseDefinition());
	}
	consume(RCBrace, "Expected '}'");
	return res;
}

DefinBody Parser::parseDefinBodyHeadless()
{
	DefinBody res;
	while(!check(COMPILER_EOF))
	{
		res.definitions.push_back(parseDefinition());
	}
	return res;
}

ClassDefin Parser::parseClassDef()
{
	ClassDefin res = consume(Identifier, "Expected identifier after 'class'");
	//TODO: complete class parsing
	return res;
}

Definition* Parser::parseDefinition()
{
	switch (lex.peek().type)
	{
	case TokenType::Let:
	{
		consume(Let, "");
		Definition* d = new VarDef(parseVarDef());
		
		return d;
		break;
	}
	case TokenType::Define:
	{
		consume(Define, "");
		Definition* d = new FuncDef(parseFunctionDef());
		return d;
		break;
	}
	case TokenType::Extern:
	{
		consume(Extern, "");
		Definition* d = new FuncDef(parseExternDef());
		return d;
		break;
	}
	default:
		printErrorMsg("Only variable definitions or function definitions allowed", lex.peek());
		Definition* d = new Definition(errTok);
		return d;
		break;
	}
}

FuncDef Parser::parseFunctionDef()
{
	FuncDef fd;
	consume(Colon, "Expected ':' after 'def'");
	fd.func.retType = getType(consume(Identifier, "Expected type identifier"));
	while (match(Asteriks))
	{
		fd.func.retType.isPtr = true;
		fd.func.retType.size = 8;
		fd.func.retType.ptrDepth++;
	}

	fd.func.t = consume(Identifier, "Expected identifier");
	consume(LParen, "Expected '('");
	while (!check(RParen))
	{
		fd.func.params.push_back(parseParamDef());
	}
	consume(RParen, "Expected ')'");
	fd.func.body = new Body(parseBody());
	return fd;
}

FuncDef Parser::parseExternDef()
{
	FuncDef fd;
	consume(Colon, "Expected ':' after 'extern'");
	fd.func.retType = getType(consume(Identifier, "Expected type identifier"));
	while (match(Asteriks))
	{
		fd.func.retType.isPtr = true;
		fd.func.retType.size = 8;
		fd.func.retType.ptrDepth++;
	}

	fd.func.t = consume(Identifier, "Expected identifier");
	consume(LParen, "Expected '('");
	while (!check(RParen))
	{
		fd.func.params.push_back(parseParamDef());
	}
	consume(RParen, "Expected ')'");
	consume(Semicolon, "Expected ';");
	fd.func.isExtern = true;
	externs.push_back(fd.func.t.value);
	return fd;
}

Variable Parser::parseParamDef()
{
	Variable v;
	v.type = getType(consume(Identifier, "Expected type identifier"));
	while (match(Asteriks))
	{
		v.type.isPtr = true;
		v.type.size = 8;
		v.type.ptrDepth++;
	}
	v.t = consume(Identifier, "Expected identifier");
	match(Comma); //Only steps over the comma if one is found, if not, lexer doesnt change position
	return v;
}

Statement* Parser::parseStatement()
{
	Statement* res = nullptr;
	switch (lex.peek().type)
	{
	case TokenType::Let:
		consume(Let, "");
		res =  new VarDef(parseVarDef());
		break;

	case TokenType::Return:
		consume(Return, "");
		return new ReturnCall(parseFuncReturn());
		break;
	case TokenType::If:
		consume(If, "");
		return new IfStmnt(parseIfStmnt());
		break;
	case TokenType::Elif:
		consume(Elif, "");
		printErrorMsg("'elif' must be preceeded by 'if' or 'elif'", lex.peek());
		break;
	case TokenType::Else:
		consume(Else, "");
		printErrorMsg("'else' must be preceeded by 'if' or 'elif'", lex.peek());
		break;
	case TokenType::While:
		consume(While, "");
		return new WhileStmnt(parseWhileStmnt());
		break;
	default:
		return new Expression(parseExpression());
	}

	return res;
}

VarDef Parser::parseVarDef()
{
	Variable v;
	Type type;
	Token matchedTok;

	consume(Colon, "Expected ':' after 'let'");
	type = getType(consume(Identifier, "Expected type identifier"));
	v.type = type;

	while(match(Asteriks))
	{
		v.type.isPtr = true;
		v.type.size = 8;
		v.type.ptrDepth++;
	}

	v.t = consume(Identifier, "Expected identifier");

	if(match(LSqParen))
	{
		v.type.isArray = true;
		v.type.arraySize = std::stoi(consume(Number, "Expected array size").value); //TODO: make this an expression
		consume(RSqParen, "Expected ']'");
	}
	
	Expression* e = nullptr;;
	if(match(Equals))
	{
		e = new Expression(parseExpression());
		e->nodes.insert(e->nodes.begin(), ExprNode(eq));
		e->nodes.insert(e->nodes.begin(),  ExprNode(VariableUse(v)));
	}
	
	VarDef res = VarDef(v, e);

	return res;
}

FuncCall Parser::parseFunctionCall()
{
	FuncCall fc;
	fc.t = consume(Identifier, "Expected Identifier");
	consume(LParen, "Expected '('");
	while(!check(RParen))
	{
		fc.params.push_back(new Expression(parseExpression()));
	}
	consume(RParen, "Expected ')'");
	return fc;
}

ReturnCall Parser::parseFuncReturn()
{
	ReturnCall rc;
	rc.expr = new Expression(parseExpression());
	if(!rc.expr->nodes.empty())
	{
		rc.expr->nodes.insert(rc.expr->nodes.begin(), ExprNode(eq));
		rc.expr->nodes.insert(rc.expr->nodes.begin(), ExprNode(Register("rax", "eax", "ax", "al")));
	}
	return rc;
}

IfStmnt Parser::parseIfStmnt()
{
	IfStmnt res = IfStmnt(lex.peek_behind());
	consume(LParen, "Expected '('");
	res.cond = new Expression(parseExpression());
	if (!res.cond->nodes.empty())
	{
		res.cond->nodes.insert(res.cond->nodes.begin(), ExprNode(eq));
		Register r = Register("r10", "r10d", "r10w", "r10b");
		r.type = primTypes["char"];
		res.cond->nodes.insert(res.cond->nodes.begin(), ExprNode(r));
	}

	consume(RParen, "Expected ')'");
	res.body = parseBody();
	if(match(Elif))
	{
		res.next = new ElIfStmnt(parseElIfStmnt());
	}
	else if(match(Else))
	{
		res.next = new ElseStmnt(parseElseStmnt());
	}
	return res;
}

ElIfStmnt Parser::parseElIfStmnt()
{
	ElIfStmnt res = ElIfStmnt(lex.peek_behind());
	consume(LParen, "Expected '('");
	res.cond = new Expression(parseExpression());
	if (!res.cond->nodes.empty())
	{
		res.cond->nodes.insert(res.cond->nodes.begin(), ExprNode(eq));
		Register r = Register("r10", "r10d", "r10w", "r10b");
		r.type = primTypes["char"];
		res.cond->nodes.insert(res.cond->nodes.begin(), ExprNode(r));
	}

	consume(RParen, "Expected ')'");
	res.body = parseBody();
	if (match(Elif))
	{
		res.next = new ElIfStmnt(parseElIfStmnt());
	}
	else if (match(Else))
	{
		res.next = new ElseStmnt(parseElseStmnt());
	}
	return res;
}

ElseStmnt Parser::parseElseStmnt()
{
	ElseStmnt res = ElseStmnt(lex.peek_behind());
	res.body = parseBody();
	return res;
}

WhileStmnt Parser::parseWhileStmnt()
{
	WhileStmnt res = WhileStmnt(lex.peek_behind());
	consume(LParen, "Expected '('");
	res.cond = new Expression(parseExpression());
	if (!res.cond->nodes.empty())
	{
		res.cond->nodes.insert(res.cond->nodes.begin(), ExprNode(eq));
		Register r = Register("r10", "r10d", "r10w", "r10b");
		r.type = primTypes["char"];
		res.cond->nodes.insert(res.cond->nodes.begin(), ExprNode(r));
	}
	consume(RParen, "Expected ')'");
	res.body = parseBody();
	return res;
}

Expression Parser::parseExpression()
{
	Expression res = Expression(lex.peek());
	while(!match(Semicolon) && !match(Comma))
	{
		ExprNode en = parseExprNode();
		if (!en.isEmpty)
		{
			res.nodes.push_back(en);
		}
		else break;
	}
	return res;
}

ExprNode Parser::parseExprNode()
{
	if(matchAny(Equals, Plus, Minus, Asteriks, Div, Modulo, LDBracket, RDBracket, Ampersand, Pipe, DEquals, LDBEq, RDBEq, NotEq, PlusEq, MinusEq, DivEq, MultEq, DAmpersand, DPipe, BoolNeg))
	{
		return ExprNode(Operator(lex.peek_behind()));
		//return parseOperator();
	}
	else if(check(LParen))
	{
		return ExprNode(LeftParen(open(LParen, "Expected '('")));
		
	}
	else if(check(RParen))
	{
		if(close(RParen, "Expected ')'").type == COMPILER_ERROR)
		{
			return ExprNode();
		}
		else
		{
			return ExprNode(RightParen(lex.peek_behind()));
		}
	}
	else if(matchAny(Number))
	{
		Literal res = Literal(lex.peek_behind());
		long numericalValue = std::stol(res.val);
		if(numericalValue <= std::numeric_limits<char>::max() && numericalValue >= std::numeric_limits<char>::min())
		{
			res.type = primTypes["char"];
		}
		else if (numericalValue <= std::numeric_limits<short>::max() && numericalValue >= std::numeric_limits<short>::min())
		{
			res.type = primTypes["short"];
		}
		else if (numericalValue <= std::numeric_limits<int>::max() && numericalValue >= std::numeric_limits<int>::min())
		{
			res.type = primTypes["int"];
		}
		else if (numericalValue <= std::numeric_limits<long>::max() && numericalValue >= std::numeric_limits<long>::min())
		{
			res.type = primTypes["long"];
		}

		return ExprNode(res);
	}
	else if(match(Identifier))
	{
		if(match(LParen))
		{
			lex.stepBack(2);
			return ExprNode(parseFunctionCall());
		}
		else
		{
			return ExprNode(VariableUse(lex.peek_behind()));
		}
	}
	else if(match(NullPtr))
	{
		Literal res = Literal(lex.peek_behind());
		res.val = "0";
		res.type = primTypes["void"];
		res.type.isPtr = true;
		res.type.ptrDepth = -1;
		return res;
	}
	else
	{
		printErrorMsg("'" + lex.peek().value + "' is not valid inside an expression", lex.peek());
		lex.nextToken();
		return ExprNode();
	}
}


//HELPER-Functions
void Parser::printErrorMsg(std::string msg, Token t)
{
	std::cout << std::endl;
	std::cout << msg << std::endl;
	std::cout << "- Occured at line " << t.loc.line << " and column " << t.loc.column << std::endl;
}

Type Parser::getType(Token tok)
{
	auto fPrim = primTypes.find(tok.value);
	if (fPrim != primTypes.end()) return fPrim->second;
	else if (tok.type == Identifier) 
	{
		return Type(0, tok.value);
	}
	else 
	{
		printErrorMsg("\"" + tok.value + "\" is not a valid type identifier", tok);
	}
	return Type();
}

bool Parser::check(TokenType expected)
{
	return expected == lex.peek().type;
}

bool Parser::match(TokenType expected)
{
	if(check(expected))
	{
		lex.nextToken();
		return true;
	}
	return false;;
}

template<typename... TokenTypes>
bool Parser::matchAny(TokenTypes... types)
{
	return (... || match(types));
}

Token Parser::consume(TokenType t, const std::string& errorMsg)
{
	if(!match(t))
	{
		printErrorMsg(errorMsg, lex.peek());
		return errTok;
	}
	return lex.peek_behind();
}

Token Parser::open(TokenType t, const std::string& errorMsg)
{
	openGroups.push(t);
	return consume(t, errorMsg);
}

Token Parser::close(TokenType t, const std::string& errorMsg)
{
	if (openGroups.size() == 0) return errTok;
	else if (openGroups.top() + 1 == t)  //Takes advantage of opening Parenthesis always being 1 ahead in the enum e.g. '(' = 9 and ')' = 10
	{
		openGroups.pop();
		return consume(t, errorMsg);
	}
	else return errTok;
}
