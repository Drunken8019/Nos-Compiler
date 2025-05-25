#pragma once
#include <string>
#include <queue>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

class Visitor;

enum TokenType
{
	COMPILER_EOF, COMPILER_EMPTY, COMPILER_ERROR,
	LCBrace, RCBrace, LParen, RParen, Equals, Semicolon, Comma, Plus, Minus, Mult, Div,
	ClassDef, Let, Define, Identifier, Return,
	Number
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
		return std::to_string(v.numID * 8);
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

	Expression()
	{}

	Expression(Token t) : ASTNode(t)
	{
	}

	Expression(std::vector<Token> t) : ASTNode(t.front()), tokens(t)
	{}

	void accept(Visitor* v);


	/*std::string res(std::string des, Function owner)
	{
		std::string res = "";
		switch (tokens.front().type)
		{
		case TokenType::Number:
			res = "mov qword " + des + ", " + tokens.front().value + "\n";
			break;
		case TokenType::Identifier:
		{
			auto ft = owner.functionTable->find(tokens.front().value);
			auto st = owner.symbolTable.find(tokens.front().value);
			if (ft != owner.functionTable->end()) 
			{
				res.append("call " + ft->second.t.value + "\n");
				res.append("mov qword " + des + ", rax\n");
			}
			else if (st != owner.symbolTable.end())
			{
				res = "mov qword r11, " + blib::asmVar(st->second) + "\n";
				res.append("mov qword " + des + ", r11\n");
			}
			break;
		}
		default:
			break;
		}
		return res;
	}*/
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

	virtual bool resolve(Class* owner) 
	{
		return false;
	};

	virtual bool resolve(Function* owner)
	{
		return false;
	};

	void accept(Visitor* v);

	virtual std::string getASM(Class owner)
	{
		return "";
	}

	virtual std::string getASM(Function owner)
	{
		return "";
	}
};

class VarAssign: public Statement
{
public:
	Expression expr;

	VarAssign()
	{}

	VarAssign(Token t, Expression e) : expr(e), Statement(t)
	{}

	bool resolve(Class* owner)
	{
		auto r = owner->classSymbolTable.find(t.value);
		if (r == owner->classSymbolTable.end()) { std::cout << "Couldn't resolve identifier \"" + t.value + "\"\n"; return false; }
		return true;
	}

	bool resolve(Function* owner)
	{
		auto r = owner->symbolTable.find(t.value);
		if (r == owner->symbolTable.end()) { std::cout << "Couldn't resolve identifier \"" + t.value + "\"\n"; return false; }
		return true;
	}

	void accept(Visitor* v);

	/*std::string getASM(Class owner)
	{
		return "";
	}

	std::string getASM(Function owner)
	{
		Variable var;
		auto f = owner.symbolTable.find(t.value);
		if (f == owner.symbolTable.end()) { std::cout << "Couldn't resolve identifier \"" + t.value + "\"\n"; return ""; }
		else var = f->second;
		std::string res = "";
		//res.append("mov qword [rsp+" + blib::varOffsetStr(var) + "], " + expr.res());
		res.append(expr.res(blib::asmVar(var), owner));
		return res;
	}*/
};

class FuncCall : public Statement
{
public:
	std::vector<Expression> params;

	FuncCall()
	{}

	FuncCall(Token t) : Statement(t)
	{}

	bool resolve(Class* owner)
	{
		return true;
	}

	bool resolve(Function* owner)
	{
		return true;
	}

	void accept(Visitor* v);

	/*std::string getASM(Class owner)
	{
		return "";
	}

	std::string getASM(Function owner)
	{
		Function func;
		auto f = owner.functionTable->find(t.value);
		if (f == owner.functionTable->end()) { std::cout << "Couldn't resolve identifier \"" + t.value + "\"\n"; return ""; }
		else func = f->second;
		std::string res = "";
		//res.append("mov qword [rsp+" + blib::varOffsetStr(var) + "], " + expr.res());
		res.append("call " + func.t.value + "\n");
		return res;
	}*/
};

class ReturnCall : public Statement
{
public:
	Expression expr;

	ReturnCall()
	{}

	ReturnCall(Token ret, Expression v): expr(v), Statement(ret)
	{}

	bool resolve(Class* owner)
	{
		return true;
	}

	bool resolve(Function* owner)
	{
		return true;
	}

	void accept(Visitor* v);

	/*std::string getASM(Class owner)
	{
		return "";
	}

	std::string getASM(Function owner)
	{
		if (expr.tokens.empty()) return "ret\n";
		std::string res = "";
		res.append(expr.res("[rsp]", owner));
		if (owner.t.value == "main")
		{
			res.append("mov rcx, [rsp]\n");
			res.append("call ExitProcess\n");
		}
		else res.append("mov rax, [rsp]\n");
		int reqSize = owner.stackSize % 16;
		reqSize += owner.stackSize;
		res.append("add rsp, " + std::to_string(reqSize) + "\n");
		res.append("ret\n");
		return res;
	}*/
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

	bool resolve(Class* owner)
	{
		auto r = owner->classSymbolTable.find(t.value);
		if (r != owner->classSymbolTable.end()) { std::cout << "Variable \"" + t.value + "\" already defined in scope\n"; return false; }
		owner->classSymbolTable.insert({ t.value, var });
		owner->stackSize += var.size;
		var.numID = owner->varCount;
		owner->varCount++;
		return true;
	}

	bool resolve(Function* owner)
	{
		auto r = owner->symbolTable.find(t.value);
		if (r != owner->symbolTable.end()) { std::cout << "Variable \"" + t.value + "\" already defined in scope\n"; return false; }
		var.numID = owner->varCount;
		owner->symbolTable.insert({ t.value, var });
		owner->stackSize += var.size;
		owner->varCount++;
		return true;
	}

	AST type()
	{
		return AST::ASTVarDef;
	}

	void accept(Visitor* v);

	/*std::string getASM(Class owner)
	{
		return "";
	}

	std::string getASM(Function owner)
	{
		std::string res = "";
		//res.append("mov qword [rsp+" + blib::varOffsetStr(var) + "], " + expr.res());
		res.append(expr.res(blib::asmVar(var), owner));
		return res;
	}*/
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

	bool resolve(Class* owner)
	{
		bool res = false;
		auto r = owner->functionTable.find(t.value);
		if (r != owner->functionTable.end()) { std::cout << "Function \"" + t.value + "\" already defined in scope\n"; return false; }
		owner->functionTable.insert({ t.value, func });
		func.symbolTable = owner->classSymbolTable;
		func.functionTable = &owner->functionTable;
		for(Statement *s : statements)
		{
			res = s->resolve(&func);
		}
		return res;
	}

	bool resolve(Function* owner)
	{
		std::cout << "Function cannot be defiened inside function.";
		return false;
	}

	void accept(Visitor* v);

	/*std::string getASM(Class owner)
	{
		std::string res = "";
		res.append(func.t.value + ":\n");
		int reqSize = func.stackSize % 16;
		reqSize += func.stackSize;
		if (func.t.value == "main") reqSize += 40;
		res.append("sub rsp, " + std::to_string(reqSize) + "\n");

		for(Statement* s : statements)
		{
			res.append(s->getASM(func));
		}
		return res;
	}

	std::string getASM(Function owner)
	{
		return "";
	}*/

	AST type()
	{
		return AST::ASTFuncDef;
	}
};