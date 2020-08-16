#include "optimization_pass.hpp"
#include "pljit/semantic_analysis/AST.hpp"

using namespace pljit::optimization;

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
