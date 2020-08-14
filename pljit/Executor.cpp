#include "Executor.hpp"
#include "semantic_analysis/AST.hpp"

namespace pljit {

void Executor::visit(semantic_analysis::FunctionNode& node) {
    for(unsigned i = 0; i < node.get_number_of_statements(); ++i) {
        node.get_statement(i).accept(*this);
    }
}

void Executor::visit(semantic_analysis::IdentifierNode& node) {
    result = variables[node.get_symbol_handle()];
}

void Executor::visit(semantic_analysis::LiteralNode& node) {
    result = node.get_value();
}

void Executor::visit(semantic_analysis::ReturnStatementNode& node) {
    node.get_expression().accept(*this);
}

void Executor::visit(semantic_analysis::AssignmentNode& node) {
    result = 0;
    node.get_expression().accept(*this);
    variables[node.get_identifier().get_symbol_handle()] = result;
}

void Executor::visit(semantic_analysis::UnaryOperatorASTNode& node) {
    result = 0;
    node.getInput().accept(*this);
    switch (node.get_operator()) {
        case semantic_analysis::UnaryOperatorASTNode::OperatorType::PLUS: {
            break;
        }
        case semantic_analysis::UnaryOperatorASTNode::OperatorType::MINUS: {
            result = -result;
            break;
        }
    }
}

void Executor::visit(semantic_analysis::BinaryOperatorASTNode& node) {
    result = 0;
    node.getLeft().accept(*this);
    int64_t lhs_result = result;
    node.getRight().accept(*this);
    switch (node.get_operator()) {
        case semantic_analysis::BinaryOperatorASTNode::OperatorType::PLUS: {
            result += lhs_result;
            break;
        }
        case semantic_analysis::BinaryOperatorASTNode::OperatorType::MINUS: {
            result -= lhs_result;
            break;
        }
        case semantic_analysis::BinaryOperatorASTNode::OperatorType::MULTIPLY: {
            result *= lhs_result;
            break;
        }
        case semantic_analysis::BinaryOperatorASTNode::OperatorType::DIVIDE: {
            if(result == 0) {
                execution_failed = true;
                throw std::logic_error("Division by zero!");
            }
            result = lhs_result / result;
            break;
        }
    }
}

} // namespace pljit