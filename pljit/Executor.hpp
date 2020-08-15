
#ifndef PLJIT_EXECUTOR_HPP
#define PLJIT_EXECUTOR_HPP

#include "semantic_analysis/ast_visitor.hpp"
#include "semantic_analysis/AST.hpp"
#include <cstdint>
#include <vector>
#include <optional>

namespace pljit {

class Executor : public semantic_analysis::ast_visitor {
    bool execution_failed = false;
    int64_t result = 0;

    std::vector<int64_t> variables;

    public:
    template <typename... Args>
    static std::optional<int64_t> Execute(semantic_analysis::ASTRoot& ast, const semantic_analysis::symbol_table& symbols, Args&&... param) {
        auto execution_visitor = Executor(symbols, std::forward<Args>(param)...);
        ast.accept(execution_visitor);
        if(!execution_visitor.execution_failed) return execution_visitor.get_result();
        return std::nullopt;
    }

    static std::optional<int64_t> Execute(semantic_analysis::ASTRoot& ast, const semantic_analysis::symbol_table& symbols, const std::vector<int64_t>& parameters);

    private:
    template <typename... Args>
    explicit Executor(const semantic_analysis::symbol_table& symbols, Args... param) {
        static_assert(std::conjunction_v<std::is_convertible<Args, int64_t>...>, "Parameters must be integers!");
        assert(symbols.get_number_of_parameters() == sizeof...(param));
        variables.reserve(symbols.size()); // Avoid reallocation
        (variables.push_back(param), ...);
        variables.resize(symbols.size());
        initialize_constants(symbols);
    }

    Executor(const semantic_analysis::symbol_table& symbols, const std::vector<int64_t>& parameters);

    void initialize_constants(const semantic_analysis::symbol_table& symbols);

    // These can still be accessed through a pointer to the base class. However, it is impossible
    // to get such a pointer using conventional methods as this class cannot be constructed.
    // We hide the visit functions for the sake of conciseness. TODO Makes sense?
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
