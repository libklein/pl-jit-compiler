#pragma once
//---------------------------------------------------------------------------
namespace pljit::semantic_analysis {

class ASTNode;
class FunctionNode;
class IdentifierNode;
class LiteralNode;
class ReturnStatementNode;
class AssignmentNode;
class UnaryOperatorASTNode;
class BinaryOperatorASTNode;

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

} // namespace pljit::semantic_analysis
//---------------------------------------------------------------------------
