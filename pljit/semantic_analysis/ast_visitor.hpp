#pragma once

#include "ast_fwd.hpp"

//---------------------------------------------------------------------------
namespace pljit::semantic_analysis {

class ast_visitor {
    public:
    virtual void visit(FunctionNode&) = 0;
    virtual void visit(IdentifierNode&) = 0;
    virtual void visit(LiteralNode&) = 0;
    virtual void visit(ReturnStatementNode&) = 0;
    virtual void visit(AssignmentNode&) = 0;
    virtual void visit(UnaryOperatorASTNode&) = 0;
    virtual void visit(BinaryOperatorASTNode&) = 0;

    virtual ~ast_visitor() = default;
};

class const_ast_visitor {
    public:
    virtual void visit(const FunctionNode&) = 0;
    virtual void visit(const IdentifierNode&) = 0;
    virtual void visit(const LiteralNode&) = 0;
    virtual void visit(const ReturnStatementNode&) = 0;
    virtual void visit(const AssignmentNode&) = 0;
    virtual void visit(const UnaryOperatorASTNode&) = 0;
    virtual void visit(const BinaryOperatorASTNode&) = 0;

    virtual ~const_ast_visitor() = default;
};

} // namespace pljit::semantic_analysis
//---------------------------------------------------------------------------
