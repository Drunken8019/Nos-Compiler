#pragma once
#include "Visitor.h"
class TypeChecker :
    public Visitor
{
public:
	Function* curFunc = nullptr;
	std::unordered_map<std::string, Variable> st;
	std::unordered_map<std::string, Function> ft;

	Type getType(ExprNode node);
	Type checkBinaryOp(Operator op, Type l, Type r);
	Type checkUnaryOp(Operator op, Type l);

	void visit(Root* node) override;
	void visit(Expression* node) override;
	void visit(FuncCall* node) override;
	void visit(ReturnCall* node) override;
	void visit(IfStmnt* node) override;
	void visit(ElIfStmnt* node) override;
	void visit(ElseStmnt* node) override;
	void visit(WhileStmnt* node) override;
	void visit(ClassDefin* node) override;
	void visitSignature(ClassDefin* node) override;
	void visit(VarDef* node) override;
	void visit(FuncDef* node) override;
	void visitSignature(FuncDef* node) override;
	void visit(Body* node) override;
	void visit(DefinBody* node) override;
};

