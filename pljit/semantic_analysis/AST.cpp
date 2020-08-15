#include "ast_visitor.hpp"
#include "AST.hpp"

using namespace pljit::semantic_analysis;

void FunctionNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void FunctionNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}

void BinaryOperatorASTNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void BinaryOperatorASTNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}

void UnaryOperatorASTNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void UnaryOperatorASTNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}

void AssignmentNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void AssignmentNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}

void ReturnStatementNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void ReturnStatementNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}

void LiteralNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void LiteralNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}

void IdentifierNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void IdentifierNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}
