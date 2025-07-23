#include "TypeChecker.h"

void TypeChecker::visit(Root* node)
{
	node->body.accept(this);
}
void TypeChecker::visit(Expression* node) 
{
	std::stack<Type> types;
	for(ExprNode en : node->rpn)
	{
		if(en.isLiteral() || en.isVariableUse() || en.isRegister())
		{
			types.push(getType(en));
		}
		else if(en.isFuncCall())
		{
			FuncCall fc = std::get<FuncCall>(en.value);
			fc.accept(this);
			types.push(getType(fc));
		}
		else if(en.isOperator())
		{
			Operator o = std::get<Operator>(en.value);
			if(o.isUnary)
			{
				Type l = types.top();  types.pop();
				types.push(checkUnaryOp(o, l)); 
			}
			else if (o.isBinary)
			{
				Type r = types.top();  types.pop();
				Type l = types.top();  types.pop();
				types.push(checkBinaryOp(o, l, r));
			}
		}
	}
	if(types.size() == 1) node->calcType = types.top();


	int depth = 1;
	int maxDepth = 1;
	for (int i = 0; i < node->rpn.size(); i++) //Determine depth of Expression -> how many temporary Results need to be stored
	{
		ExprNode en = node->rpn[i];
		if(en.isFuncCall() || en.isLiteral() || en.isRegister() || en.isVariableUse())
		{
			if(en.isVariableUse())
			{
				VariableUse vu = std::get<VariableUse>(en.value);
				auto fv = st.find(vu.identifier);
				if (fv == st.end())
				{
					std::cout << "Unknown identifier '" << vu.identifier << "'\n";
					return;
				}
				vu.v = fv->second;
				node->rpn[i] = vu;
			}
			else if(en.isFuncCall())
			{
				FuncCall fc = std::get<FuncCall>(en.value);
				auto ffc = ft.find(fc.t.value);
				if(ffc == ft.end())
				{
					std::cout << "Unknown identifier '" << fc.t.value << "'\n";
					return;
				}
				fc.f = ffc->second;
				node->rpn[i] = fc;
			}
			depth++;
		}
		else if(en.isOperator())
		{
			Operator o = std::get<Operator>(en.value);
			if(o.isBinary)
			{
				depth--;
			}
		}
		maxDepth = std::max(maxDepth, depth);
	}
	node->depth = maxDepth;
	curFunc->maxExprDepth = std::max(curFunc->maxExprDepth, maxDepth); 
}
void TypeChecker::visit(FuncCall* node) 
{
	auto r = ft.find(node->t.value);
	if (r == ft.end())
	{
		std::cout << "Unkown identifier '" << node->t.value << "'\n";
		return;
	}
	node->f = r->second;

	for(Expression *e : node->params)
	{
		e->accept(this);
	}
}
void TypeChecker::visit(ReturnCall* node) 
{
	node->expr->accept(this);
}
void TypeChecker::visit(IfStmnt* node) 
{
	node->cond->accept(this);
	node->body.accept(this);
	if (node->next != nullptr) node->next->accept(this);
}
void TypeChecker::visit(ElIfStmnt* node) 
{
	node->cond->accept(this);
	node->body.accept(this);
	if (node->next != nullptr) node->next->accept(this);
}
void TypeChecker::visit(ElseStmnt* node) 
{
	node->body.accept(this);
}
void TypeChecker::visit(WhileStmnt* node) 
{
	node->cond->accept(this);
	node->body.accept(this);
}
void TypeChecker::visit(ClassDefin* node) {}
void TypeChecker::visitSignature(ClassDefin* node) {}
void TypeChecker::visit(VarDef* node) 
{
	if (node->expr != nullptr) node->expr->accept(this);
}
void TypeChecker::visit(FuncDef* node) 
{
	curFunc = &node->func;
	if(node->func.body != nullptr) node->func.body->accept(this);
	if(node->func.maxExprDepth > 4)
	{
		node->func.stackSize += (node->func.maxExprDepth * 8) + (node->func.maxExprDepth % 16); 
		//For now, depth surpassing avalaible registers will just reserve 8 bytes for temp results (even if their smaller)
	}
}
void TypeChecker::visitSignature(FuncDef* node) 
{
}
void TypeChecker::visit(Body* node) 
{
	auto prevSt = st;
	auto prevFt = ft;
	st = node->symbolTable;
	ft = node->functionTable;
	for(Statement* s : node->statements)
	{
		s->accept(this);
	}
	st = prevSt;
	ft = prevFt;
}
void TypeChecker::visit(DefinBody* node) 
{
	auto prevSt = st;
	auto prevFt = ft;
	st = node->symbolTable;
	ft = node->functionTable;
	for(Definition* d : node->definitions)
	{
		d->accept(this);
	}
	st = prevSt;
	ft = prevFt;
}

Type TypeChecker::getType(ExprNode node)
{
	if(node.isVariableUse())
	{
		VariableUse vu = std::get<VariableUse>(node.value);
		auto fv = st.find(vu.identifier);
		if (fv != st.end())
		{
			return fv->second.type;
		}
	}
	else if(node.isLiteral())
	{
		Literal l = std::get<Literal>(node.value);
		return l.type;
	}
	else if(node.isFuncCall())
	{
		FuncCall fc = std::get<FuncCall>(node.value);
		auto ffc = ft.find(fc.t.value);
		if (ffc != ft.end())
		{
			return ffc->second.retType;
		}
	}
	else if(node.isRegister())
	{
		Register r = std::get<Register>(node.value);
		return r.type;
	}
	return Type();
}

Type TypeChecker::checkBinaryOp(Operator op, Type l, Type r)
{
	if(l.isCompatibleWith(r))
	{
		if(l.getSize() > r.getSize()) return l;
		return r;
	}

	std::cout << "'"  <<  l.name << "' (depth: " << std::to_string(l.ptrDepth) << ") is incompatible with '" << r.name << "' (depth: " << std::to_string(r.ptrDepth) << ")\n";
	return Type();
}

Type TypeChecker::checkUnaryOp(Operator op, Type l)
{
	if(op.tok.type == UAmpersand)
	{
		l.isPtr = true;
		l.ptrDepth++;
	}
	else if(op.tok.type == UAsteriks && l.isPtr)
	{
		l.ptrDepth--;
		if(l.ptrDepth == 0)
		{
			l.size = l.nonPointerSize;
			l.isPtr = false;
		}
	}

	return l;
}