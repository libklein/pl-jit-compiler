
#ifndef PLJIT_OPTIMIZATION_PASS_HPP
#define PLJIT_OPTIMIZATION_PASS_HPP

#include <memory>
#include <pljit/semantic_analysis/ast_fwd.hpp>
#include <pljit/semantic_analysis/symbol_table.hpp>

namespace pljit::optimization {

class optimization_pass {
    /**
     * Requirements:
     *      Control flow (tree traversal) should be controllable by the actual optimizer implementations
     *      Optimizers need to be able to remove and/or replace nodes of the ast
     *          This should be done in a safe fashion
     */
    public:
    optimization_pass() = default;

    protected:
    // By default, these just transfer ownership back
    /***
     *
     * @return The (new/modified) node or nullptr if the node shall be removed
     */
    private:
    virtual void optimize(pljit::semantic_analysis::FunctionNode& node) = 0;

    public:
    virtual std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::IdentifierNode> node);
    virtual std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::LiteralNode> node);
    virtual std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::UnaryOperatorASTNode> node);
    virtual std::unique_ptr<pljit::semantic_analysis::ExpressionNode> optimize(std::unique_ptr<pljit::semantic_analysis::BinaryOperatorASTNode> node);
    virtual std::unique_ptr<pljit::semantic_analysis::StatementNode> optimize(std::unique_ptr<pljit::semantic_analysis::ReturnStatementNode> node);
    virtual std::unique_ptr<pljit::semantic_analysis::StatementNode> optimize(std::unique_ptr<pljit::semantic_analysis::AssignmentNode> node);

    void optimize_ast(std::unique_ptr<pljit::semantic_analysis::FunctionNode>& ast);

    virtual ~optimization_pass() = default;
};

} // namespace pljit::optimization

#endif //PLJIT_OPTIMIZATION_PASS_HPP
