#include "ast_visitor.hpp"
#include "AST.hpp"

using namespace pljit::semantic_analysis;

void FunctionNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}

void BinaryOperatorASTNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}

void UnaryOperatorASTNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}

void AssignmentNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}

void ReturnStatementNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}

void LiteralNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}

void IdentifierNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
