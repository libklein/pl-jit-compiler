#include "AST.hpp"
#include "ASTCreator.hpp"
#include "ast_visitor.hpp"
#include "pljit/execution/ExecutionContext.hpp"
#include "pljit/optimization/optimization_pass.hpp"
#include "pljit/parser/parse_tree_nodes.hpp"

using namespace pljit::execution;
using namespace pljit;

namespace pljit::semantic_analysis {

void FunctionNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
std::optional<int64_t> FunctionNode::evaluate(ExecutionContext& context) const {
    for (const auto& statement : statements) {
        if (auto result = statement->evaluate(context); !result) {
            return {};
        } else if (statement->getType() == ReturnStatement) {
            context.set_result(result);
            return result;
        }
    }
    // Unreachable
    return {};
}
FunctionNode::FunctionNode(std::vector<std::unique_ptr<StatementNode>> statements, symbol_table symbols)
    : ASTNode(ASTNode::Function), statements(std::move(statements)), symbols(std::move(symbols)) {}
auto FunctionNode::get_number_of_statements() const -> std::vector<std::unique_ptr<StatementNode>>::size_type {
    return statements.size();
}
std::unique_ptr<StatementNode>& FunctionNode::get_statement(unsigned int id) {
    assert(id < statements.size());
    return statements[id];
}
const symbol_table& FunctionNode::getSymbolTable() const {
    return symbols;
}
symbol_table& FunctionNode::getSymbolTable() {
    return symbols;
}
void FunctionNode::removeStatement(unsigned int id) {
    statements.erase(statements.begin() + id);
}
std::unique_ptr<StatementNode> FunctionNode::releaseStatement(unsigned int id) {
    return std::move(statements[id]);
}

/*void FunctionNode::optimize(std::unique_ptr<ASTNode> self, optimization::optimization_pass& optimizer) {
    std::unique_ptr<FunctionNode> casted_self(static_cast<FunctionNode*>(self.release()));
    casted_self = optimizer.optimize(std::move(casted_self));
}*/

void BinaryOperatorASTNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
std::optional<int64_t> BinaryOperatorASTNode::evaluate(ExecutionContext& context) const {
    auto lhs_result = getLeft().evaluate(context);
    if (!lhs_result) return {};
    auto rhs_result = getRight().evaluate(context);
    if (!rhs_result) return {};
    switch (get_operator()) {
        case semantic_analysis::BinaryOperatorASTNode::OperatorType::PLUS: {
            return *lhs_result + *rhs_result;
        }
        case semantic_analysis::BinaryOperatorASTNode::OperatorType::MINUS: {
            return *lhs_result - *rhs_result;
        }
        case semantic_analysis::BinaryOperatorASTNode::OperatorType::MULTIPLY: {
            return *lhs_result * *rhs_result;
        }
        case semantic_analysis::BinaryOperatorASTNode::OperatorType::DIVIDE: {
            if (rhs_result == 0) {
                std::cerr << "Error: Division by zero at " << std::endl;
                return {};
            }
            return *lhs_result / *rhs_result;
        }
    }
    // Unreachable
    return {};
}
void BinaryOperatorASTNode::optimize(std::unique_ptr<ExpressionNode>& self, optimization::optimization_pass& optimizer) {
    if (self.get() == this) {
        // Temporarily assume ownership - safe as this == self.get, so the type is correct
        std::unique_ptr<BinaryOperatorASTNode> typed_self(static_cast<BinaryOperatorASTNode*>(self.release()));
        self = optimizer.optimize(std::move(typed_self));
    }
}
BinaryOperatorASTNode::BinaryOperatorASTNode(std::unique_ptr<ExpressionNode> leftChild, BinaryOperatorASTNode::OperatorType operation, std::unique_ptr<ExpressionNode> rightChild)
    : ExpressionNode(ASTNode::BinaryOperation),
      left_child(std::move(leftChild)),
      operation(operation),
      right_child(std::move(rightChild)) {
    assert(this->left_child);
    assert(this->right_child);
}

void UnaryOperatorASTNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
std::optional<int64_t> UnaryOperatorASTNode::evaluate(ExecutionContext& context) const {
    auto result = getInput().evaluate(context);
    if (!result) return {};
    switch (get_operator()) {
        case semantic_analysis::UnaryOperatorASTNode::OperatorType::PLUS: {
            return result;
        }
        case semantic_analysis::UnaryOperatorASTNode::OperatorType::MINUS: {
            return -*result;
        }
    }
    // Unreachable
    return {};
}
void UnaryOperatorASTNode::optimize(std::unique_ptr<ExpressionNode>& self, optimization::optimization_pass& optimizer) {
    if (self.get() == this) {
        // Temporarily assume ownership - safe as this == self.get, so the type is correct
        std::unique_ptr<UnaryOperatorASTNode> typed_self(static_cast<UnaryOperatorASTNode*>(self.release()));
        self = optimizer.optimize(std::move(typed_self));
    }
}

void AssignmentNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
std::optional<int64_t> AssignmentNode::evaluate(ExecutionContext& context) const {
    auto expression_result = value->evaluate(context);
    if (!expression_result) return {};

    context.set_value(target->get_symbol_handle(), *expression_result);
    return expression_result;
}
void AssignmentNode::optimize(std::unique_ptr<StatementNode>& self, optimization::optimization_pass& optimizer) {
    if (self.get() == this) {
        // Temporarily assume ownership - safe as this == self.get, so the type is correct
        std::unique_ptr<AssignmentNode> typed_self(static_cast<AssignmentNode*>(self.release()));
        self = optimizer.optimize(std::move(typed_self));
    }
}

void ReturnStatementNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}

std::optional<int64_t> ReturnStatementNode::evaluate(ExecutionContext& context) const {
    return return_expression->evaluate(context);
}
void ReturnStatementNode::optimize(std::unique_ptr<StatementNode>& self, optimization::optimization_pass& optimizer) {
    if (self.get() == this) {
        // Temporarily assume ownership - safe as this == self.get, so the type is correct
        std::unique_ptr<ReturnStatementNode> typed_self(static_cast<ReturnStatementNode*>(self.release()));
        self = optimizer.optimize(std::move(typed_self));
    }
}

void LiteralNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
std::optional<int64_t> LiteralNode::evaluate(ExecutionContext&) const {
    return get_value();
}
void LiteralNode::optimize(std::unique_ptr<ExpressionNode>& self, optimization::optimization_pass& optimizer) {
    if (self.get() == this) {
        // Temporarily assume ownership - safe as this == self.get, so the type is correct
        std::unique_ptr<LiteralNode> typed_self(static_cast<LiteralNode*>(self.release()));
        self = optimizer.optimize(std::move(typed_self));
    }
}

void IdentifierNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
std::optional<int64_t> IdentifierNode::evaluate(ExecutionContext& context) const {
    return context.get_value(get_symbol_handle());
}
void IdentifierNode::optimize(std::unique_ptr<ExpressionNode>& self, optimization::optimization_pass& optimizer) {
    if (self.get() == this) {
        // Temporarily assume ownership - safe as this == self.get, so the type is correct
        std::unique_ptr<IdentifierNode> typed_self(static_cast<IdentifierNode*>(self.release()));
        self = optimizer.optimize(std::move(typed_self));
    }
}
symbol_table::symbol_handle IdentifierNode::get_symbol_handle() const {
    return symbol;
}

ASTNode::Type ASTNode::getType() const {
    return type;
}
} // namespace pljit::semantic_analysis