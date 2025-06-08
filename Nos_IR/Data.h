#pragma once
#include <string>
#include <queue>
#include <stack>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

class Visitor;

enum TokenType
{
	COMPILER_EOF, COMPILER_EMPTY, COMPILER_ERROR,
	EXPR_DEST, EXPR_TMP,
	LCBrace, RCBrace, LParen, RParen, Equals, Semicolon, Comma, Plus, Minus, Mult, Div, LDBracket, RDBracket, DEquals, LDBEq, RDBEq, NotEq,
	ClassDef, Let, Define, Identifier, Return, If, Else, While, Number
};

enum AST
{
	ASTStatement, ASTDefinition, ASTFuncDef, ASTVarDef, ASTClassDef
};

struct Location
{
	int line, column;
};

struct Token
{
	TokenType type;
	std::string value;
	Location loc;
};

//--------------------------- Types -----------------------------
class Variable
{
public:
	Token t;
	int size;
	int numID;

	Variable()
	{}

	Variable(Token tok, int s) : size(s), t(tok)
	{
	}
};

class Function
{
public:
	Token t;
	std::unordered_map<std::string, Variable> symbolTable;
	std::unordered_map<std::string, Function>* functionTable;
	int retSize;
	int stackSize = 8;
	int varCount = 1;

	Function()
	{}

	Function(Token tok) : retSize(0), t(tok)
	{}

	Function(Token tok, int size) : retSize(size), t(tok)
	{}
};

class Class
{
public:
	Token t;
	std::unordered_map<std::string, Variable> classSymbolTable;
	std::unordered_map<std::string, Function> functionTable;
	int stackSize = 8;
	int varCount = 1;

	Class()
	{}

	Class(Token tok) : t(tok)
	{
	}
};

//------------------------ Basic Library ------------------------
class blib
{
public:
	static std::string varOffsetStr(Variable v)
	{
		return std::to_string(v.numID * 8 + 32);
	}

	static std::string asmVar(Variable v)
	{
		return "[rsp+" + varOffsetStr(v) + "]";
	}
};

//------------------------- AST-Nodes ---------------------------
class ASTNode
{
public:
	Token t;
	ASTNode() : t()
	{}

	ASTNode(Token tok) : t(tok)
	{}

	virtual void accept(Visitor* v) = 0;
};

class Expression : public ASTNode
{
public:
	std::vector<Token> tokens;
	std::string des = "";
	std::queue<Token> rpn;

	Expression()
	{}

	Expression(Token t) : ASTNode(t)
	{
	}

	Expression(std::vector<Token> t) : ASTNode(t.front()), tokens(t)
	{}

	void accept(Visitor* v);
};

//------------------------- Statements --------------------------
class Statement : public ASTNode
{
public:
	Statement()
	{}
	//std::vector<Token> value;
	Statement(Token t) : ASTNode(t)
	{}

	virtual AST type()
	{
		return AST::ASTStatement;
	}

	void accept(Visitor* v);
};

class VarAssign: public Statement
{
public:
	Expression expr;

	VarAssign()
	{}

	VarAssign(Token t, Expression e) : expr(e), Statement(t)
	{}

	void accept(Visitor* v);
};

class FuncCall : public Statement
{
public:
	std::vector<Expression> params;

	FuncCall()
	{}

	FuncCall(Token t) : Statement(t)
	{}

	void accept(Visitor* v);
};

class ReturnCall : public Statement
{
public:
	Expression expr;

	ReturnCall()
	{}

	ReturnCall(Token ret, Expression v): expr(v), Statement(ret)
	{}

	void accept(Visitor* v);
};

class IfStmnt : public Statement
{
public:
	Expression cond;
	std::vector<Statement*> body;
	std::unordered_map<std::string, Variable> symbolTable;

	IfStmnt(Token t) : Statement(t) {}

	void accept(Visitor* v);
};

class ElseStmnt : public Statement
{
public:
	IfStmnt prec;
	std::vector<Statement*> body;
	std::unordered_map<std::string, Variable> symbolTable;

	ElseStmnt(Token t) : Statement(t), prec(t){}

	void accept(Visitor* v);
};

class WhileStmnt : public Statement
{
public:
	Expression cond;
	std::vector<Statement*> body;
	std::unordered_map<std::string, Variable> symbolTable;

	WhileStmnt(Token t) : Statement(t){}

	void accept(Visitor* v);
};

//------------------------ Definitions --------------------------
class Definition : public Statement
{
public:
	Definition()
	{}

	Definition(Token t) : Statement(t)
	{}

	AST type()
	{
		return AST::ASTDefinition;
	}

	void accept(Visitor* v);
};

class ClassDefin : public Definition
{
public:
	Class c;
	std::vector<Definition*> defs;

	ClassDefin()
	{}

	ClassDefin(Token t) : c(t), Definition(t)
	{}

	AST type()
	{
		return AST::ASTClassDef;
	}

	void accept(Visitor* v);
};

class VarDef : public Definition
{
public:
	Variable var;
	Expression expr;

	VarDef(Variable v, Expression e) : Definition(v.t), var(v), expr(e)
	{
	}

	AST type()
	{
		return AST::ASTVarDef;
	}

	void accept(Visitor* v);
};

class FuncDef : public Definition
{
public:
	Function func;
	std::vector<Statement*> statements;
	std::vector<Expression> params;
	int stackSpace = 0;

	FuncDef()
	{}
	
	FuncDef(Token t) : func(t), Definition(t)
	{}

	void accept(Visitor* v);

	AST type()
	{
		return AST::ASTFuncDef;
	}
};

//--------------------------- Root ------------------------------
class Root : public ASTNode
{
public:
	std::vector<Definition*> defs;
	Root()
	{
	}

	Root(Token t) : ASTNode(t)
	{
	}

	void accept(Visitor* v);
};