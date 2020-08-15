#include "Executor.hpp"
#include "semantic_analysis/AST.hpp"

namespace pljit {

void Executor::visit(semantic_analysis::FunctionNode& node) {
    for(unsigned i = 0; i < node.get_number_of_statements(); ++i) {
        node.get_statement(i).accept(*this);
        if(execution_failed) return;
    }
}

void Executor::visit(semantic_analysis::IdentifierNode& node) {
    result = variables[node.get_symbol_handle()];
}

void Executor::visit(semantic_analysis::LiteralNode& node) {
    result = node.get_value();
}

void Executor::visit(semantic_analysis::ReturnStatementNode& node) {
    result = 0;
    node.get_expression().accept(*this);
    if(execution_failed) return;
}

void Executor::visit(semantic_analysis::AssignmentNode& node) {
    result = 0;
    node.get_expression().accept(*this);
    if(execution_failed) return;
    variables[node.get_identifier().get_symbol_handle()] = result;
}

void Executor::visit(semantic_analysis::UnaryOperatorASTNode& node) {
    result = 0;
    node.getInput().accept(*this);
    if(execution_failed) return;
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
    if(execution_failed) return;
    int64_t lhs_result = result;
    node.getRight().accept(*this);
    if(execution_failed) return;
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
                // TODO Error... We need to know where each AST node begins/ends
                std::cerr << "Error: Division by zero at " << "TODO" << std::endl;
                return;
            }
            result = lhs_result / result;
            break;
        }
    }
}

std::optional<int64_t> Executor::Execute(semantic_analysis::ASTRoot& ast, const semantic_analysis::symbol_table& symbols, const std::vector<int64_t>& parameters) {
    auto execution_visitor = Executor(symbols, parameters);
    ast.accept(execution_visitor);
    if(!execution_visitor.execution_failed) return execution_visitor.get_result();
    return std::nullopt;
}

Executor::Executor(const semantic_analysis::symbol_table& symbols, const std::vector<int64_t>& parameters)
    : variables(symbols.get_number_of_variables()) {
    assert(symbols.get_number_of_parameters() == parameters.size());
    std::copy(parameters.begin(), parameters.end(), variables.begin());
    initialize_constants(symbols);
}

void Executor::initialize_constants(const semantic_analysis::symbol_table& symbols) {
    for(auto next_constant = symbols.constants_begin(); next_constant != symbols.constants_end(); ++next_constant) {
        assert(next_constant->initialized);
        variables[next_constant->id] = next_constant->get_value();
    }
}

} // namespace pljit