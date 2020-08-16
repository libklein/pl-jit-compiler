#include "UnaryPlusRemoval.hpp"
#include "pljit/semantic_analysis/AST.hpp"

using namespace pljit::semantic_analysis;
using namespace pljit::optimization;

namespace pljit::optimization::passes {
void UnaryPlusRemoval::optimize(FunctionNode& node) {
    for (unsigned i = 0; i < node.get_number_of_statements(); ++i) {
        node.get_statement(i)->optimize(node.get_statement(i), *this);
    }
}

std::unique_ptr<pljit::semantic_analysis::ExpressionNode> UnaryPlusRemoval::optimize(std::unique_ptr<pljit::semantic_analysis::UnaryOperatorASTNode> node) {
    node->getInput().optimize(node->releaseInput(), *this);
    if (node->get_operator() == UnaryOperatorASTNode::OperatorType::PLUS) {
        return std::move(node->releaseInput());
    }
    return node;
}

std::unique_ptr<pljit::semantic_analysis::ExpressionNode> UnaryPlusRemoval::optimize(std::unique_ptr<pljit::semantic_analysis::BinaryOperatorASTNode> node) {
    node->getLeft().optimize(node->releaseLeft(), *this);
    node->getRight().optimize(node->releaseRight(), *this);
    return node;
}

std::unique_ptr<pljit::semantic_analysis::StatementNode> UnaryPlusRemoval::optimize(std::unique_ptr<pljit::semantic_analysis::ReturnStatementNode> node) {
    node->get_expression().optimize(node->releaseExpression(), *this);
    return node;
}

std::unique_ptr<pljit::semantic_analysis::StatementNode> UnaryPlusRemoval::optimize(std::unique_ptr<pljit::semantic_analysis::AssignmentNode> node) {
    node->get_expression().optimize(node->releaseExpression(), *this);
    return node;
}
} // namespace pljit::optimization::passes