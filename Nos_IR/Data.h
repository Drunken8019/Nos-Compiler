#pragma once
#include <string>
#include <queue>
#include <stack>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

class Visitor;
class FuncCall;
class Expression;

enum TokenType
{
	COMPILER_EOF, COMPILER_EMPTY, COMPILER_ERROR,
	EXPR_DEST, EXPR_TMP, EXPR_FN,
	/*Symbols*/
	LCBrace, RCBrace, LParen, RParen, Equals, Semicolon, Comma,
	Plus, Minus, Asteriks, Div, LDBracket, RDBracket, DEquals, LDBEq, RDBEq, NotEq, Colon, 
	PlusEq, MinusEq, MultEq, DivEq, Ampersand, DAmpersand, Pipe, DPipe, UAmpersand, UAsteriks, UMinus,
	/*Keywords*/
	ClassDef, Let, Define, Identifier, Return, If, Elif, Else, While, Number, Extern, Character, Short, Integer, Long,
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

//Mby ExprNode will see some use... else expr will just be kept using tokens
class ExprNode
{
public:
	virtual std::string getVal() = 0;

	ExprNode()
	{}
};

class Literal : public ExprNode //Operators will be saved as literals
{
public:
	std::string val = "";

	Literal(std::string s) : val(s){}
	Literal(){}

	std::string getVal() override
	{
		return val;
	}
};

class Register : public ExprNode
{
public:
	int reqSize = 4;
	std::string qReg;
	std::string dReg;
	std::string wReg;
	std::string bReg;

	Register(std::string q, std::string d, std::string w, std::string b) :
		qReg(q), dReg(d), wReg(w), bReg(b)
	{}

	std::string getVal() override
	{
		switch(reqSize)
		{
		case 1:
			return bReg;
		case 2:
			return wReg;
		case 4:
			return dReg;
		case 8:
			return qReg;
		}
	}
};

class Type {
public:
	int size;
	int nonPointerSize;
	std::string name;
	bool isPtr = false;

	Type() : size(4), name("int"), nonPointerSize(4)
	{
	}

	Type(int s, std::string n) : size(s), name(n), nonPointerSize(s)
	{
	}
};

class Variable : public ExprNode
{
public:
	Type type;
	Token t;
	int numID = 0;

	Variable()
	{}

	Variable(Token tok, Type t) : type(t), t(tok)
	{
	}

	std::string getVal() override
	{
		return t.value;
	}
};

class Function
{
public:
	Token t;
	Type retType;
	std::unordered_map<std::string, Variable> symbolTable;
	std::unordered_map<std::string, Function>* functionTable;
	std::vector<Variable> params;
	int paramStackSpace = 0;
	int stackSize = 0;
	int varCount = 1;
	bool isExtern = false;

	Function()
	{}

	Function(Token tok) : t(tok)
	{}

	Function(Token tok, Type ret) : t(tok), retType(ret)
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
	static int offset;

	static std::string varOffsetStr(Variable v)
	{
		return std::to_string(v.numID + offset);
	}

	static std::string asmVar(Variable v)
	{
		return "[rsp+" + varOffsetStr(v) + "]";
	}

	static bool canBeUnary(TokenType t)
	{
		switch(t)
		{
		case Ampersand:
			return true;
		case Asteriks:
			return true;
		case Minus:
			return true;
		default:
			return false;
		}
	}

	static TokenType getUnary(TokenType t)
	{
		switch (t)
		{
		case Ampersand:
			return UAmpersand;
		case Asteriks:
			return UAsteriks;
		case Minus:
			return UMinus;
		default:
			return COMPILER_ERROR;
		}
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
	bool desIsPtrDref = false;
	std::queue<Token> rpn;
	std::unordered_map<std::string, FuncCall> exprFnTable;
	Token resOperator = { Equals, "=", {} };

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
	bool isPtrAccess = false;

	VarAssign()
	{}

	VarAssign(Token t, Expression e) : expr(e), Statement(t)
	{}

	void accept(Visitor* v);
};

class FuncCall : public Statement, ExprNode
{
private:
	Register rax = { "rax", "eax", "ax", "al" };
public:
	Function f;
	std::vector<Expression> params;


	FuncCall()
	{}

	FuncCall(Token t) : Statement(t), f(t)
	{}

	void accept(Visitor* v);

	std::string getVal() override
	{
		switch(f.retType.size)
		{
		case 1:
			return rax.bReg;
		case 2:
			return rax.wReg;
		case 4:
			return rax.dReg;
		case 8:
			return rax.qReg;
		default:
			return rax.dReg;
		}
	}
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

//------------------------- If-Followers -------------------------

class IfFollower : public Statement
{
public:

	IfFollower(Token t) : Statement(t)
	{
	}

	int endIndex;

	void accept(Visitor* v);
};

class IfStmnt : public Statement
{
public:
	Expression cond;
	std::vector<Statement*> body;
	std::unordered_map<std::string, Variable> symbolTable;
	IfFollower *next = nullptr;
	int followerCount = 0;

	IfStmnt(Token t) : Statement(t) {}

	void accept(Visitor* v);
};

class ElIfStmnt : public IfFollower
{
public:
	Expression cond;
	std::vector<Statement*> body;
	std::unordered_map<std::string, Variable> symbolTable;
	IfFollower* next = nullptr;


	ElIfStmnt(Token t) : IfFollower(t)
	{}

	void accept(Visitor* v);
};

class ElseStmnt : public IfFollower
{
public:
	std::vector<Statement*> body;
	std::unordered_map<std::string, Variable> symbolTable;

	ElseStmnt(Token t) : IfFollower(t){}

	void accept(Visitor* v);
};

//---------------------------------------------------------------

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