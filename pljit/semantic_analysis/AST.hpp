#pragma once
//---------------------------------------------------------------------------
#include "ast_visitor.hpp"
#include "pljit/parser/parser_fwd.hpp"
#include "pljit/semantic_analysis/symbol_table.hpp"
#include <memory>
#include <ostream>
#include <string_view>
#include <vector>

namespace pljit::execution {
    class ExecutionContext; // Forward declare execution context
} // namespace pljit::execution
namespace pljit::optimization {
    class optimization_pass;
}
namespace pljit::semantic_analysis {
class ast_visitor; // Forward declare visitor
class FunctionNode;
using ASTRoot = FunctionNode;
//---------------------------------------------------------------------------
/// Base class for AST nodes
class ASTNode {
    public:
    /// All possible types of ASTNodes
    enum Type {
        Function,
        Assignment,
        ReturnStatement,
        Identifier,
        Literal,
        BinaryOperation,
        UnaryOperation
    };

    private:
    Type type;

    protected:
    explicit ASTNode(Type type) : type(type) {};

    public:
    Type getType() const {
        return type;
    };

    virtual std::unique_ptr<ASTNode> optimize(std::unique_ptr<ASTNode>& self, optimization::optimization_pass& optimizer) { return nullptr; };
    virtual std::optional<int64_t> evaluate(execution::ExecutionContext& context) const = 0;
    virtual void accept(ast_visitor& visitor) = 0;
    virtual void accept(const_ast_visitor& visitor) const = 0;

    virtual ~ASTNode() = default;
};

class StatementNode : public ASTNode {
    protected:
    using ASTNode::ASTNode;
};

class FunctionNode : public ASTNode {
    std::vector<std::unique_ptr<StatementNode>> statements;

    public:
    explicit FunctionNode(std::vector<std::unique_ptr<StatementNode>> statements) : ASTNode(ASTNode::Function)
        , statements(std::move(statements)) {}

    auto get_number_of_statements() const {
        return statements.size();
    }

    StatementNode& get_statement(unsigned int id) {
        assert(id < statements.size());
        return *statements[id];
    }

    /*static std::unique_ptr<FunctionNode> analyzeFunction(const parser::function_defition_node& node) {
        std::vector<std::unique_ptr<StatementNode>> statements;
        for(const auto& next_statement : node.get_children()) {
            assert(next_statement->get_type() == parser::STATEMENT);
        }
    }*/

    void accept(ast_visitor& visitor) override;
    std::optional<int64_t> evaluate(execution::ExecutionContext& context) const override;
    void accept(const_ast_visitor& visitor) const override;
};

class ExpressionNode : public ASTNode {
    protected:
    using ASTNode::ASTNode;
};

class ValueNode : public ExpressionNode {
    protected:
    using ExpressionNode::ExpressionNode;
};

class IdentifierNode : public ValueNode {
    symbol_table::symbol_handle symbol;
    public:
    explicit IdentifierNode(symbol_table::symbol_handle symbol_handle) : ValueNode(ASTNode::Identifier), symbol(symbol_handle) {};

    symbol_table::symbol_handle get_symbol_handle() const {
        return symbol;
    }
    std::optional<int64_t> evaluate(execution::ExecutionContext& context) const override;

    void accept(ast_visitor& visitor) override;
    void accept(const_ast_visitor& visitor) const override;
};

class LiteralNode : public ValueNode {
    int64_t value;
    public:
    explicit LiteralNode(int64_t value) : ValueNode(ASTNode::Literal), value(value) {};

    int64_t get_value() const {
        return value;
    }
    std::optional<int64_t> evaluate(execution::ExecutionContext& context) const override;
    void accept(ast_visitor& visitor) override;
    void accept(const_ast_visitor& visitor) const override;
};

class ReturnStatementNode : public StatementNode {
    std::unique_ptr<ExpressionNode> return_expression;

    public:
    explicit ReturnStatementNode(std::unique_ptr<ExpressionNode> return_expression)
        : StatementNode(ASTNode::ReturnStatement), return_expression(std::move(return_expression)) {}

    ExpressionNode& get_expression() {
        return *return_expression;
    }
    std::optional<int64_t> evaluate(execution::ExecutionContext& context) const override;
    void accept(ast_visitor& visitor) override;
    void accept(const_ast_visitor& visitor) const override;
};

class AssignmentNode : public StatementNode {
    std::unique_ptr<IdentifierNode> target;
    std::unique_ptr<ExpressionNode> value;

    public:
    AssignmentNode(std::unique_ptr<IdentifierNode> target, std::unique_ptr<ExpressionNode> value)
        : StatementNode(ASTNode::Assignment), target(std::move(target)), value(std::move(value)) {}

    IdentifierNode& get_identifier() {
        return *target;
    }

    ExpressionNode& get_expression() {
        return *value;
    }
    std::optional<int64_t> evaluate(execution::ExecutionContext& context) const override;
    void accept(ast_visitor& visitor) override;
    void accept(const_ast_visitor& visitor) const override;
};

class UnaryOperatorASTNode : public ExpressionNode {
    public:
    enum class OperatorType {
        PLUS, // Actually, i believe we can get rid of this
        MINUS
    };

    private:
    std::unique_ptr<ExpressionNode> child;
    OperatorType operation;

    protected:
    //virtual double op(double value) const = 0;
    //virtual std::unique_ptr<ASTNode> optimize_impl() = 0;
    //virtual void accept_impl(ASTVisitor& visitor) = 0;

    public:
    explicit UnaryOperatorASTNode(std::unique_ptr<ExpressionNode> child, OperatorType operator_type)
        : ExpressionNode(ASTNode::UnaryOperation)
          , child(std::move(child))
          , operation(operator_type) {};

    ExpressionNode& getInput() { return *child; };
    const ExpressionNode& getInput() const { return *child; };
    std::unique_ptr<ExpressionNode> releaseInput() { return std::move(child); };

    OperatorType get_operator() const {
        return operation;
    }
    std::optional<int64_t> evaluate(execution::ExecutionContext& context) const override;
    void accept(ast_visitor& visitor) override;
    void accept(const_ast_visitor& visitor) const override;
};

class BinaryOperatorASTNode : public ExpressionNode {
    public:
    enum class OperatorType {
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE
    };

    private:
    std::unique_ptr<ExpressionNode> left_child;
    OperatorType operation;
    std::unique_ptr<ExpressionNode> right_child;

    protected:
    //virtual double op(double lhs, double rhs) const = 0;
    //virtual std::unique_ptr<ASTNode> optimize_impl() = 0;
    //virtual void accept_impl(ASTVisitor& visitor) = 0;
    //virtual void accept(ASTVisitor& visitor) = 0;

    public:
    BinaryOperatorASTNode(std::unique_ptr<ExpressionNode> leftChild,
                          OperatorType operation,
                          std::unique_ptr<ExpressionNode> rightChild)
        : ExpressionNode(ASTNode::BinaryOperation),
          left_child(std::move(leftChild)),
          operation(operation),
          right_child(std::move(rightChild)) {
              assert(this->left_child);
              assert(this->right_child);
          };

    OperatorType get_operator() const {
        return operation;
    }

    ExpressionNode& getLeft() { return *left_child; };
    ExpressionNode& getRight() { return *right_child; };
    const ExpressionNode& getLeft() const { return *left_child; };
    const ExpressionNode& getRight() const { return *right_child; };

    std::unique_ptr<ExpressionNode> releaseLeft() { return std::move(left_child); };
    std::unique_ptr<ExpressionNode> releaseRight() { return std::move(right_child); };
    std::optional<int64_t> evaluate(execution::ExecutionContext& context) const override;
    void accept(ast_visitor& visitor) override;
    void accept(const_ast_visitor& visitor) const override;
};
//---------------------------------------------------------------------------
class ASTCreator {
    private:
    symbol_table symbols;
    using symbol_handle = symbol_table::size_type;
    std::unordered_map<std::string_view, symbol_handle> identifier_mapping;

    // Helpers for the symbol table
    std::pair<symbol_handle, bool> register_symbol(const parser::identifier_node& node, symbol::symbol_type type
        , std::optional<int64_t> value);

    bool analyze_declarations(const parser::declarator_list_node& node, symbol::symbol_type symbolType);
    bool analyze_declarations(const parser::init_declarator_list_node& node, symbol::symbol_type symbolType);

    std::unique_ptr<IdentifierNode> analyze_identifier(const parser::identifier_node& node);
    std::unique_ptr<LiteralNode> analyze_literal(const parser::literal_node& node);
    std::unique_ptr<ReturnStatementNode> analyze_return_statement(const parser::statement_node& node);
    std::unique_ptr<AssignmentNode> analyze_assignment_node(const parser::assignment_expression_node& node);

    std::unique_ptr<UnaryOperatorASTNode> analyze_expression(const parser::unary_expression_node& node);
    std::unique_ptr<ExpressionNode> analyze_expression(const parser::multiplicative_expression_node& node);
    std::unique_ptr<ExpressionNode> analyze_expression(const parser::additive_expression_node& node);
    std::unique_ptr<ExpressionNode> analyze_expression(const parser::primary_expression_node& node);
    std::unique_ptr<FunctionNode> analyze_function(const parser::function_definition_node& parseTree);
    public:
    static std::pair<std::unique_ptr<FunctionNode>, symbol_table> CreateAST(const parser::function_definition_node& parseTree);
};
} // namespace pljit::semantic_analysis
//---------------------------------------------------------------------------
