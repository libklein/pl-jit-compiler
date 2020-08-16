#include "optimization_pass.hpp"
#include "pljit/semantic_analysis/AST.hpp"

using namespace pljit::optimization;
/*
void optimization_pass::visit(pljit::semantic_analysis::FunctionNode& node) {
}
void optimization_pass::visit(pljit::semantic_analysis::IdentifierNode& node) {
}
void optimization_pass::visit(pljit::semantic_analysis::LiteralNode& node) {
}
void optimization_pass::visit(pljit::semantic_analysis::ReturnStatementNode& node) {
}
void optimization_pass::visit(pljit::semantic_analysis::AssignmentNode& node) {
}
void optimization_pass::visit(pljit::semantic_analysis::UnaryOperatorASTNode& node) {
    // Assume ownership of the current node
    auto this_node = std::move(current_node);
    // Set the new current node to the child
    current_node = std::move(node.releaseInput());
    // Trigger optimization of the child node
    current_node->accept(*this);
    // Child has set current_node pointer to it's replacement
    if(current_node) { // If there is an replacement, replace the current node
        // TODO
        //node.setInput(std::move(current_node));
        current_node = this->optimize(std::unique_ptr<std::remove_reference_t<decltype(node)>>(
            static_cast<std::remove_reference<decltype(node)>::type*>(this_node.release())));
    }
    // Current node is nullptr. Don't update with this_node, as having a unary operator without any
    // children does not make a lot of sense.
}
void optimization_pass::visit(pljit::semantic_analysis::BinaryOperatorASTNode& node) {
    **
      * Binary Node:
      * left_child->opt
      * Potentially replace/remove left child
      * right_child->opt
      * Potentially replace/remove right child
      *
      * 2 children left? Call optimize
      * 1 child left? Replace by child
      * 0 children left? Remove self
      *
    // This will optimize the left node
    node.getLeft().accept(*this);
    // This will optimize the right subtree
    node.getRight().accept(*this);
    this->optimize(node);
}*/


void optimization_pass::optimize_ast(std::unique_ptr<pljit::semantic_analysis::FunctionNode>& ast) {
    symbolTable = &(ast->getSymbolTable());
    while (initialize_pass()) {
        //ast->accept(*this);
        this->optimize(*ast);
        //this->optimize_function(*ast);
        if (!finalize_pass()) break;
    }
}

std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimization_pass::optimize(std::unique_ptr<pljit::semantic_analysis::IdentifierNode> node) {
    return node;
}
std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimization_pass::optimize(std::unique_ptr<pljit::semantic_analysis::LiteralNode> node) {
    return node;
}
std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimization_pass::optimize(std::unique_ptr<pljit::semantic_analysis::UnaryOperatorASTNode> node) {
    return node;
}
std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimization_pass::optimize(std::unique_ptr<pljit::semantic_analysis::BinaryOperatorASTNode> node) {
    return node;
}
std::unique_ptr<pljit::semantic_analysis::StatementNode> optimization_pass::optimize(std::unique_ptr<pljit::semantic_analysis::ReturnStatementNode> node) {
    return node;
}
std::unique_ptr<pljit::semantic_analysis::StatementNode> optimization_pass::optimize(std::unique_ptr<pljit::semantic_analysis::AssignmentNode> node) {
    return node;
}
