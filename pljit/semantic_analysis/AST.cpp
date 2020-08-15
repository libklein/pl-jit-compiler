#include "ast_visitor.hpp"
#include "AST.hpp"
#include "pljit/execution/ExecutionContext.hpp"

using namespace pljit::semantic_analysis;
using namespace pljit::execution;

void FunctionNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void FunctionNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}
std::optional<int64_t> FunctionNode::evaluate(ExecutionContext& context) const {
    for(const auto& statement : statements) {
        if(auto result = statement->evaluate(context);!result) {
            return {};
        } else if (statement->getType() == ReturnStatement) {
            context.set_result(result);
            return result;
        }
    }
    // Unreachable
    return {};
}

void BinaryOperatorASTNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void BinaryOperatorASTNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}
std::optional<int64_t> BinaryOperatorASTNode::evaluate(ExecutionContext& context) const {
    auto lhs_result = getLeft().evaluate(context);
    if(!lhs_result) return {};
    auto rhs_result = getRight().evaluate(context);
    if(!rhs_result) return {};
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
            if(rhs_result == 0) {
                // TODO Error... We need to know where each AST node begins/ends
                std::cerr << "Error: Division by zero at " << "TODO" << std::endl;
                return {};
            }
            return *lhs_result / *rhs_result;
        }
    }
    // Unreachable
    return {};
}

void UnaryOperatorASTNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void UnaryOperatorASTNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}
std::optional<int64_t> UnaryOperatorASTNode::evaluate(ExecutionContext& context) const {
    auto result = getInput().evaluate(context);
    if(!result) return {};
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

void AssignmentNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void AssignmentNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}
std::optional<int64_t> AssignmentNode::evaluate(ExecutionContext& context) const {
    auto expression_result = value->evaluate(context);
    if(!expression_result) return {};

    context.set_value(target->get_symbol_handle(), *expression_result);
    return expression_result;
}

void ReturnStatementNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}

void ReturnStatementNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}

std::optional<int64_t> ReturnStatementNode::evaluate(ExecutionContext& context) const {
    return return_expression->evaluate(context);
}


void LiteralNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void LiteralNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}
std::optional<int64_t> LiteralNode::evaluate(ExecutionContext&) const {
    return get_value();
}

void IdentifierNode::accept(ast_visitor& visitor) {
    visitor.visit(*this);
}
void IdentifierNode::accept(const_ast_visitor& visitor) const {
    visitor.visit(*this);
}
std::optional<int64_t> IdentifierNode::evaluate(ExecutionContext& context) const {
    return context.get_value(get_symbol_handle());
}
