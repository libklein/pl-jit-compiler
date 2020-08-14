
#ifndef PLJIT_EXECUTOR_HPP
#define PLJIT_EXECUTOR_HPP

#include "semantic_analysis/ast_visitor.hpp"
#include <cstdint>
#include <vector>

namespace pljit {

class Executor : public semantic_analysis::ast_visitor {
    bool execution_failed = false;
    int64_t result = 0;

    std::vector<int64_t> variables;
    public:
    template <typename... Args>
    explicit Executor(unsigned number_of_variables, Args... param) {
        // TODO Make sure that there are as many parameters as parameters
        static_assert(std::conjunction_v<std::is_convertible<Args, int64_t>...>, "Parameters must be integers!");
        variables.reserve(number_of_variables);
        (variables.push_back(param), ...);
        variables.resize(number_of_variables);
    }

    void visit(semantic_analysis::FunctionNode& node) override;
    void visit(semantic_analysis::IdentifierNode& node) override;
    void visit(semantic_analysis::LiteralNode& node) override;
    void visit(semantic_analysis::ReturnStatementNode& node) override;
    void visit(semantic_analysis::AssignmentNode& node) override;
    void visit(semantic_analysis::UnaryOperatorASTNode& node) override;
    void visit(semantic_analysis::BinaryOperatorASTNode& node) override;

    int64_t get_result() const {
        return result;
    }
};

} // namespace pljit

#endif //PLJIT_EXECUTOR_HPP
