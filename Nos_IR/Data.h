#pragma once
#include <string>
#include <queue>
#include <stack>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <variant>

class Visitor;
class FuncCall;
class Expression;
class ExprNode;
class Body;
class DefinBody;
class VarDef;
class VariableUse;
class Literal;
class Operator;
class Register;
class Class;
class Statement;
class Definition;
class FuncDef;
class Variable;
class Function;
class ASTNode;
class VarAsign;
class IfStatement;
class ElIfStmnt;
class ElseStmnt;
class WhileStmnt;
class Root;

enum TokenType
{
	COMPILER_EOF, COMPILER_EMPTY, COMPILER_ERROR,
	EXPR_DEST, EXPR_TMP, EXPR_FN,
	/*Symbols*/
	LCBrace, RCBrace, LParen, RParen, LSqParen, RSqParen, Equals, Semicolon, Comma,
	Plus, Minus, Asteriks, Div, LDBracket, RDBracket, DEquals, LDBEq, RDBEq, NotEq, Colon, 
	PlusEq, MinusEq, MultEq, DivEq, Ampersand, DAmpersand, Pipe, DPipe, UAmpersand, UAsteriks, UMinus, BoolNeg,
	/*Keywords*/
	ClassDef, Let, Define, Identifier, Return, If, Elif, Else, While, Number, Extern, Character, Short, Integer, Long, Void,
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

class ASTNode
{
public:
	Token t;
	ASTNode() : t()
	{
	}

	ASTNode(Token tok) : t(tok)
	{
	}

	virtual void accept(Visitor* v) = 0;
};

class Statement : public ASTNode
{
public:
	Statement()
	{
	}
	//std::vector<Token> value;
	Statement(Token t) : ASTNode(t)
	{
	}

	virtual AST type()
	{
		return AST::ASTStatement;
	}

	void accept(Visitor* v);
};

//--------------------------- Types -----------------------------


class Operator
{
public:
	std::string keyWord = "";
	Token tok;
	bool isUnary = false;
	bool isBinary = true;
	bool isMemOp = false;

	std::unordered_map<TokenType, std::string> operatorKeyword = {
	{TokenType::Equals, "mov"},
	{TokenType::Plus, "add"},
	{TokenType::Minus, "sub"},
	{TokenType::Asteriks, "imul"},
	{TokenType::Div, "div"},
	{TokenType::Ampersand, "and"},
	{TokenType::Pipe, "or"},
	{TokenType::DEquals, "sete"},
	{TokenType::LDBracket, "setl"},
	{TokenType::RDBracket, "setg"},
	{TokenType::LDBEq, "setle"},
	{TokenType::RDBEq, "setge"},
	{TokenType::NotEq, "setne"},
	{TokenType::UMinus, "neg"},
	{TokenType::UAmpersand, "lea"},
	{TokenType::UAsteriks, "mov"}, //Hopefully this doesnt lead to problems
	};

	std::unordered_map<TokenType, TokenType> binToUnary = {
		{TokenType::Asteriks, TokenType::UAsteriks},
		{TokenType::Ampersand, TokenType::UAmpersand},
		{TokenType::Minus, TokenType::UMinus},
	};

	Operator(Token t) : tok(t)
	{
		auto opKey = operatorKeyword.find(t.type);
		if(opKey != operatorKeyword.end())
		{
			keyWord = opKey->second;
		}
		else
		{
			keyWord = "Unrecognized Operator";
		}
	}

	void setUnary(bool state)
	{
		isUnary = state;
		isBinary = !state;
		isMemOp = !state;
		tok.type = binToUnary[tok.type];
		auto opKey = operatorKeyword.find(tok.type);
		if (opKey != operatorKeyword.end())
		{
			keyWord = opKey->second;
		}
		else
		{
			keyWord = "Unrecognized unary Operator";
		}
	}

	void setMemOp(bool state)
	{
		isUnary = !state;
		isBinary = !state;
		isMemOp = state;
	}

	Operator()
	{}
};

class LeftParen
{
public:
	Token tok;
	LeftParen(Token t) : tok(t)
	{}
};

class RightParen
{
public:
	Token tok;
	RightParen(Token t) : tok(t)
	{
	}
};

class Literal
{
public:
	std::string val = "";

	Literal(Token t) : val(t.value){}
	Literal(std::string s) : val(s){}
	Literal(){}
};

class Register
{
public:
	std::string qReg = "";
	std::string dReg = "";
	std::string wReg = "";
	std::string bReg = "";

	Register(std::string q, std::string d, std::string w, std::string b) :
		qReg(q), dReg(d), wReg(w), bReg(b)
	{}

	Register()
	{}
};

class Type {
public:
	int size;
	int nonPointerSize;
	std::string name;
	bool isPtr = false;
	int ptrDepth = 0;
	bool isArray = false;
	int arraySize = 0;

	int getSize()
	{
		if(arraySize == 0)
		{
			return size;
		}
		return size * arraySize;
	}

	Type() : size(4), name("int"), nonPointerSize(4)
	{
	}

	Type(int s, std::string n) : size(s), name(n), nonPointerSize(s)
	{
	}
};

class Variable
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
};

class VariableUse
{
public:
	Variable v;
	std::string identifier = "";

	VariableUse()
	{}

	VariableUse(Token t) : identifier(t.value)
	{}

	VariableUse(Variable var) : v(var)
	{}
};

class Function
{
public:
	Body* body = nullptr;
	Token t;
	Type retType;
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
	DefinBody* body;
	Token t;
	int stackSize = 8;
	int varCount = 1;

	Class()
	{}

	Class(Token tok) : t(tok)
	{
	}
};

//------------------------- AST-Nodes ---------------------------

class Body : ASTNode
{
public:
	std::vector<Statement*> statements;
	std::unordered_map<std::string, Variable> symbolTable;
	std::unordered_map<std::string, Function> functionTable;

	Body(){}

	Body(Token t) : ASTNode(t)
	{
	}

	void accept(Visitor* v) override;
};

class DefinBody : ASTNode
{
public:
	std::vector<Definition*> definitions;
	std::unordered_map<std::string, Variable> symbolTable;
	std::unordered_map<std::string, Function> functionTable;

	DefinBody() {}

	DefinBody(Token t) : ASTNode(t)
	{
	}

	void accept(Visitor* v) override;
};

//------------------------- Statements --------------------------

class FuncCall : public Statement
{
private:
	Register rax = { "rax", "eax", "ax", "al" };
public:
	Function f;
	std::vector<Expression*> params;


	FuncCall()
	{}

	FuncCall(Token t) : Statement(t), f(t)
	{}

	void accept(Visitor* v);
};

class ReturnCall : public Statement
{
public:
	Expression* expr;

	ReturnCall()
	{}

	ReturnCall(Token ret, Expression* v): expr(v), Statement(ret)
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
	Expression* cond;
	Body body;
	std::unordered_map<std::string, Variable> symbolTable;
	IfFollower *next = nullptr;
	int followerCount = 0;

	IfStmnt(Token t) : Statement(t), body(t) {}

	void accept(Visitor* v);
};

class ElIfStmnt : public IfFollower
{
public:
	Expression* cond;
	Body body;
	IfFollower* next = nullptr;


	ElIfStmnt(Token t) : IfFollower(t)
	{}

	void accept(Visitor* v);
};

class ElseStmnt : public IfFollower
{
public:
	Body body;

	ElseStmnt(Token t) : IfFollower(t), body(t)
	{}

	void accept(Visitor* v);
};

//---------------------------------------------------------------

class WhileStmnt : public Statement
{
public:
	Expression* cond;
	Body body;

	WhileStmnt(Token t) : Statement(t), body(t){}

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
	virtual void acceptSig(Visitor* v);
};

class ClassDefin : public Definition
{
public:
	Class c;

	ClassDefin(Token t) : c(t), Definition(t)
	{}

	AST type()
	{
		return AST::ASTClassDef;
	}

	void accept(Visitor* v);
	void acceptSig(Visitor* v);
};

class VarDef : public Definition
{
public:
	Variable var;
	Expression* expr;
	Operator eq = Operator(Token(Equals, "=", Location(0, 0)));

	VarDef(Variable v, Expression* e) : Definition(v.t), var(v), expr(e)
	{
	}

	AST type() //TODO: remove ast type(), not needed
	{
		return AST::ASTVarDef;
	}
	void accept(Visitor* v);
	void acceptSig(Visitor* v);
};

class FuncDef : public Definition
{
public:
	Function func;

	int stackSpace = 0;

	FuncDef()
	{}
	
	FuncDef(Token t) : func(t), Definition(t)
	{}

	void accept(Visitor* v);
	void acceptSig(Visitor* v);

	AST type()
	{
		return AST::ASTFuncDef;
	}
};

//--------------------------- Root ------------------------------
class Root : public ASTNode
{
public:
	DefinBody body;

	Root(Token t) : ASTNode(t), body(t)
	{
	}

	void accept(Visitor* v);
};

class ExprNode
{
public:
	using ExprVariant = std::variant<Literal, VariableUse, FuncCall, Operator, Register, LeftParen, RightParen>;
	ExprVariant value;
	bool isEmpty = false;

	ExprNode() : isEmpty(true)
	{
	}

	ExprNode(Literal l) : value(l) {}
	ExprNode(VariableUse v) : value(v) {}
	ExprNode(FuncCall f) : value(f) {}
	ExprNode(Operator o) : value(o) {}
	ExprNode(Register r) : value(r) {}
	ExprNode(LeftParen lp) : value(lp) {}
	ExprNode(RightParen rp) : value(rp) {}

	bool isLiteral() const { return std::holds_alternative<Literal>(value); }
	bool isVariableUse() const { return std::holds_alternative<VariableUse>(value); }
	bool isFuncCall() const { return std::holds_alternative<FuncCall>(value); }
	bool isOperator() const { return std::holds_alternative<Operator>(value); }
	bool isRegister() const { return std::holds_alternative<Register>(value); }
	bool isLeftParen() const { return std::holds_alternative<LeftParen>(value); }
	bool isRightParen() const { return std::holds_alternative<RightParen>(value); }
};

class Expression : public Statement
{
public:
	std::vector<ExprNode> nodes;
	std::vector<ExprNode> rpn;
	std::unordered_map<std::string, FuncCall*> exprFnTable;

	Expression()
	{
	}

	Expression(Token t) : Statement(t)
	{
	}

	Expression(std::vector<ExprNode> n, Token t) : Statement(t), nodes(n)
	{
	}

	void accept(Visitor* v);
};

//------------------------ Basic Library ------------------------
class blib
{
public:
	static int offset;

	using ExprVariant = std::variant<Literal, VariableUse, FuncCall, Operator, Register, LeftParen, RightParen>;

	static std::string varOffsetStr(Variable v)
	{
		return std::to_string(v.numID + offset);
	}

	static std::string asmVar(Variable v)
	{
		return "[rsp+" + varOffsetStr(v) + "]";
	}

	static bool canBeUnary(ExprVariant ev)
	{
		Operator o = std::get<Operator>(ev);
		switch (o.tok.type)
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