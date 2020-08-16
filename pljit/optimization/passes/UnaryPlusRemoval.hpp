
#ifndef PLJIT_UNARYPLUSREMOVAL_HPP
#define PLJIT_UNARYPLUSREMOVAL_HPP

#include "pljit/optimization/optimization_pass.hpp"

namespace pljit::optimization::passes {

    class UnaryPlusRemoval : public pljit::optimization::optimization_pass {
        void optimize(semantic_analysis::FunctionNode& node) override;

        public:
        std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::UnaryOperatorASTNode> node) override;
        std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::BinaryOperatorASTNode> node) override;
        std::unique_ptr<pljit::semantic_analysis::StatementNode> optimize(std::unique_ptr<pljit::semantic_analysis::ReturnStatementNode> node) override;
        std::unique_ptr<pljit::semantic_analysis::StatementNode> optimize(std::unique_ptr<pljit::semantic_analysis::AssignmentNode> node) override;
    };

} // namespace pljit::optimization::passes

#endif //PLJIT_UNARYPLUSREMOVAL_HPP
