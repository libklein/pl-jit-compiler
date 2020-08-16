
#ifndef PLJIT_CONSTANT_PROPAGATION_HPP
#define PLJIT_CONSTANT_PROPAGATION_HPP

#include <unordered_map>
#include "pljit/semantic_analysis/symbol_table.hpp"
#include "pljit/semantic_analysis/AST.hpp"
#include "pljit/optimization/optimization_pass.hpp"
#include "pljit/execution/ExecutionContext.hpp"

namespace pljit::optimization::passes {

class constant_propagation : public optimization_pass {
    std::unordered_map<pljit::semantic_analysis::ASTNode*, int64_t> expression_mapping;
    std::vector<std::optional<int64_t>> constant_variables;
    std::unique_ptr<pljit::execution::ExecutionContext> executionContext;
    public:
    //void optimize_function(semantic_analysis::FunctionNode& node) override;
    void optimize(semantic_analysis::FunctionNode& node) override;
    std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::IdentifierNode> node) override;
    std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::LiteralNode> node) override;
    std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::UnaryOperatorASTNode> node) override;
    std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::BinaryOperatorASTNode> node) override;
    std::unique_ptr<pljit::semantic_analysis::StatementNode> optimize(std::unique_ptr<pljit::semantic_analysis::ReturnStatementNode> node) override;
    std::unique_ptr<pljit::semantic_analysis::StatementNode> optimize(std::unique_ptr<pljit::semantic_analysis::AssignmentNode> node) override;

    private:
    std::optional<int64_t> get_value(semantic_analysis::ASTNode* expression) const;
    std::unique_ptr<semantic_analysis::ExpressionNode> replace_expression(std::unique_ptr<semantic_analysis::ExpressionNode> expression);
    public:
};

} // namespace pljit::optimization::passes

#endif //PLJIT_CONSTANT_PROPAGATION_HPP
