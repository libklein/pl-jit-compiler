#include "constant_propagation.hpp"
#include "pljit/semantic_analysis/AST.hpp"

using namespace pljit::optimization::passes;
using namespace pljit::optimization;
using namespace pljit::semantic_analysis;

using ExpressionPtr = std::unique_ptr<ExpressionNode>;

/*
void pljit::optimization::passes::constant_propagation::visit(pljit::semantic_analysis::FunctionNode& node) {
    // First pass
    if(getNumberOfPasses()) {
        for (unsigned i = 0; i < node.get_number_of_statements(); ++i) {
            node.get_statement(i)->accept(*this);
        }
    }
}
void pljit::optimization::passes::constant_propagation::visit(pljit::semantic_analysis::IdentifierNode& node) {
    if(const auto& symbol = symbolTable->get(node.get_symbol_handle()); symbol.type == semantic_analysis::symbol::CONSTANT) {
        expression_mapping.emplace(&node, symbolTable->get(node.get_symbol_handle()).constant_value);
        return;
    } else if (symbol.type == semantic_analysis::symbol::VARIABLE) {
        // TODO Is the variable constant?
    }
}
void pljit::optimization::passes::constant_propagation::visit(pljit::semantic_analysis::LiteralNode& node) {
    expression_mapping.emplace(&node, node.get_value());
}
void pljit::optimization::passes::constant_propagation::visit(pljit::semantic_analysis::ReturnStatementNode& node) {
}
void pljit::optimization::passes::constant_propagation::visit(pljit::semantic_analysis::AssignmentNode& node) {
}
void pljit::optimization::passes::constant_propagation::visit(pljit::semantic_analysis::UnaryOperatorASTNode& node) {
    node.getInput().accept(*this);
    if(get_value(&node.getInput())) {
        //node.setInput(node);
        // Result of unary expression must be constant too
        //expression_mapping.emplace(&node, );
    }
}
void pljit::optimization::passes::constant_propagation::visit(pljit::semantic_analysis::BinaryOperatorASTNode& node) {
}
*/



void pljit::optimization::passes::constant_propagation::optimize(pljit::semantic_analysis::FunctionNode& node) {
    // TODO
    executionContext = std::make_unique<pljit::execution::ExecutionContext>(*symbolTable, 0);
    constant_variables.resize(symbolTable->size());

    for(unsigned i = 0; i < symbolTable->size(); ++i) {
        switch (symbolTable->get(i).type) {
            case symbol::CONSTANT: {
                constant_variables[i] = symbolTable->get(i).constant_value;
                break;
            }
            case symbol::PARAMETER: {
                constant_variables[i] = std::nullopt;
                break;
            }
            case symbol::VARIABLE: {
                constant_variables[i] = std::nullopt;
                break;
            }
        }
    }

    for (unsigned i = 0; i < node.get_number_of_statements(); ++i) {
        node.get_statement(i)->optimize(node.get_statement(i), *this);
    }
}

std::unique_ptr<pljit::semantic_analysis::ExpressionNode> pljit::optimization::passes::constant_propagation::optimize(std::unique_ptr<pljit::semantic_analysis::IdentifierNode> node) {
    if(constant_variables[node->get_symbol_handle()]) {
        auto const_node = std::make_unique<LiteralNode>(*constant_variables[node->get_symbol_handle()]);
        expression_mapping.emplace(node.get(), const_node->get_value());
        return const_node;
    }
    return node;
}

std::unique_ptr<pljit::semantic_analysis::ExpressionNode> pljit::optimization::passes::constant_propagation::optimize(std::unique_ptr<pljit::semantic_analysis::LiteralNode> node) {
    return node;
}

std::unique_ptr<pljit::semantic_analysis::ExpressionNode> pljit::optimization::passes::constant_propagation::optimize(std::unique_ptr<pljit::semantic_analysis::UnaryOperatorASTNode> node) {
    node->getInput().optimize(node->releaseInput(), *this);
    if(get_value(&node->getInput())) {
        return replace_expression(std::move(node));
    }
    return node;
}

std::unique_ptr<pljit::semantic_analysis::ExpressionNode> pljit::optimization::passes::constant_propagation::optimize(std::unique_ptr<pljit::semantic_analysis::BinaryOperatorASTNode> node) {
    node->getLeft().optimize(node->releaseLeft(), *this);
    node->getRight().optimize(node->releaseRight(), *this);
    if(get_value(&node->getLeft()) && get_value(&node->getRight())) {
        // Evaluate this node
        auto result = node->evaluate(*executionContext);
        if(!result) {
            throw std::logic_error("TODO");
        }
        // Replace with proper value
        auto const_node = std::make_unique<pljit::semantic_analysis::LiteralNode>(*result);
        // Push to mapping
        expression_mapping.emplace(const_node.get(), *result);
        return const_node; // Replace with constant
    }
    return node;
}

std::unique_ptr<pljit::semantic_analysis::StatementNode> pljit::optimization::passes::constant_propagation::optimize(std::unique_ptr<pljit::semantic_analysis::ReturnStatementNode> node) {
    node->get_expression().optimize(node->releaseExpression(), *this);
    if(get_value(&node->get_expression())) {
        node->releaseExpression() = replace_expression(std::move(node->releaseExpression()));
    }
    return node;
}

std::unique_ptr<pljit::semantic_analysis::StatementNode> pljit::optimization::passes::constant_propagation::optimize(std::unique_ptr<pljit::semantic_analysis::AssignmentNode> node) {
    node->get_expression().optimize(node->releaseExpression(), *this);
    if(get_value(&node->get_expression())) {
        node->releaseExpression() = replace_expression(std::move(node->releaseExpression()));
        // Update the value
        constant_variables[node->get_identifier().get_symbol_handle()] = static_cast<LiteralNode&>(node->get_expression()).get_value();
    }
    return node;
}
std::optional<int64_t> pljit::optimization::passes::constant_propagation::get_value(pljit::semantic_analysis::ASTNode* expression) const {
    if(expression->getType() == pljit::semantic_analysis::ASTNode::Literal) {
        return static_cast<pljit::semantic_analysis::LiteralNode*>(expression)->get_value();
    }
    if(auto iter = expression_mapping.find(expression); iter != expression_mapping.end()) {
        return iter->second;
    }
    return std::nullopt;

}

ExpressionPtr pljit::optimization::passes::constant_propagation::replace_expression(ExpressionPtr node) {
    // Evaluate this node
    auto result = node->evaluate(*executionContext);
    if(!result) {
        throw std::logic_error("TODO");
    }
    // Replace with proper value
    auto const_node = std::make_unique<pljit::semantic_analysis::LiteralNode>(*result);
    // Push to mapping
    expression_mapping.emplace(node.get(), *result);
    return const_node;
}
