#pragma once
#include "Lexer.h"

class ArithmeticExpression
{
public:
	Token lOp, aOp, rOp;
	std::string result;
	ArithmeticExpression(){}

	ArithmeticExpression(Token num)
	{
		result = num.value;
	}

	ArithmeticExpression(Token left, Token op, Token right)
	{
		lOp = left;
		aOp = op;
		rOp = right;
		result = "NOT IMPLEMENTED";
	}
};

