#include "optimization_pass.hpp"
#include "pljit/semantic_analysis/AST.hpp"

using namespace pljit::optimization;
using namespace pljit::semantic_analysis;

namespace pljit::optimization {

void optimization_pass::optimize_ast(std::unique_ptr<FunctionNode>& ast) {
    this->optimize(*ast);
}

std::unique_ptr<ExpressionNode> optimization_pass::optimize(std::unique_ptr<IdentifierNode> node) {
    return node;
}
std::unique_ptr<ExpressionNode> optimization_pass::optimize(std::unique_ptr<LiteralNode> node) {
    return node;
}
std::unique_ptr<ExpressionNode> optimization_pass::optimize(std::unique_ptr<UnaryOperatorASTNode> node) {
    return node;
}
std::unique_ptr<ExpressionNode> optimization_pass::optimize(std::unique_ptr<BinaryOperatorASTNode> node) {
    return node;
}
std::unique_ptr<StatementNode> optimization_pass::optimize(std::unique_ptr<ReturnStatementNode> node) {
    return node;
}
std::unique_ptr<StatementNode> optimization_pass::optimize(std::unique_ptr<AssignmentNode> node) {
    return node;
}

} // namespace pljit::optimization