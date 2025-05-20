#pragma once

enum TokenType
{
	COMPILER_EOF, COMPILER_EMPTY,
	LCBrace, RCBrace, LParen, RParen, Equals, Semicolon, Plus, Minus, Mult, Div,
	Let, Define, Identifier, Exit, Return,
	Function, Number
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

struct Variable
{
	Token val;
	int count;
	int scopeElevation;
	//std::vector<int> scopes;
};

template<typename T> class Expression
{
public:
	bool lSet = false, rSet = false, opSet = false;
	T left;
	Token operand;
	T right;

	void setLeft(T l) { left = l; lSet = true; }
	void setRight(T r) { left = r; rSet = true; }
	void setOp(Token o) { operand = o; opSet = true; }
};

template<typename L, typename R> class Option
{
private:
	L opt1;
	R opt2;

public:
	const bool isLeft, isRight;
	Option(L o) : isLeft(true), isRight(false)
	{
		opt1 = o;
		opt2 = {};
	}

	Option(R o) : isLeft(false), isRight(true)
	{
		opt1 = {};
		opt2 = o;
	}

	L getLeft() { return opt1; }
	R getRight() { return opt2; }
};