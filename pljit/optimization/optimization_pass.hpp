
#ifndef PLJIT_OPTIMIZATION_PASS_HPP
#define PLJIT_OPTIMIZATION_PASS_HPP

#include <memory>
#include <pljit/semantic_analysis/AST.hpp>
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
    private:
    unsigned passes = 0;
    //std::unique_ptr<semantic_analysis::ASTNode> current_node;

    protected:
    pljit::semantic_analysis::symbol_table* symbolTable;

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

    //virtual void optimize_function(pljit::semantic_analysis::FunctionNode&) = 0;

    virtual bool initialize_pass() {
        return true;
    }

    /// Multiple passes may be required. Returns true if another pass is nessesary
    virtual bool finalize_pass() {
        return false;
    }

    unsigned getNumberOfPasses() const {
        return passes;
    }

    void optimize_ast(std::unique_ptr<pljit::semantic_analysis::FunctionNode>& ast);

    virtual ~optimization_pass() = default;
};

} // namespace pljit::optimization

#endif //PLJIT_OPTIMIZATION_PASS_HPP
