#include "ASTCreator.hpp"
#include "pljit/parser/parse_tree_nodes.hpp"
#include "pljit/semantic_analysis/AST.hpp"

using namespace pljit;
using namespace pljit::semantic_analysis;

auto ASTCreator::register_symbol(const parser::identifier_node& node, symbol::symbol_type type, std::optional<int64_t> value) -> std::pair<symbol_handle, bool> {
    std::string_view name = node.get_token().get_code_reference().str();

    if (auto handle_iter = identifier_mapping.find(name); handle_iter != identifier_mapping.end()) {
        return {handle_iter->second, false};
    }
    symbol_handle id = symbols.insert(node.get_token().get_code_reference(), type, value);
    identifier_mapping.emplace(name, id);
    return {id, true};
}

bool ASTCreator::analyze_declarations(const parser::declarator_list_node& node, symbol::symbol_type symbolType) {
    for (unsigned i = 0; i < node.get_number_of_declarations(); ++i) {
        if (auto [handle, success] = register_symbol(*node.get_declaration(i), symbolType, std::nullopt); !success) {
            std::cerr << "Error: Redeclaration of identifier \""
                      << node.get_declaration(i)->get_token().get_code_reference().str()
                      << "\" originally defined here: ";
            std::cerr << symbols.get(handle).declaration << std::endl;
            return false;
        }
    }
    return true;
}

bool ASTCreator::analyze_declarations(const parser::init_declarator_list_node& node, symbol::symbol_type symbolType) {
    for (unsigned i = 0; i < node.get_number_of_declarations(); ++i) {
        const auto* declaration = node.get_declaration(i);
        if (auto [handle, success] = register_symbol(*declaration->get_identifier(), symbolType, declaration->get_value()->get_value()); !success) {
            std::cerr << "Error: Redeclaration of identifier \""
                      << node.get_declaration(i)->get_identifier()->get_token().get_code_reference().str()
                      << "\" originally defined here: ";
            std::cerr << symbols.get(handle).declaration << std::endl;
            return false;
        }
    }
    return true;
}

std::unique_ptr<FunctionNode> ASTCreator::analyze_function(const parser::function_definition_node& parseTree) {
    using pljit::parser::statement_node;
    // Parse declarations
    if (parseTree.has_parameter_declaration) {
        const auto& parameter_list = *parseTree.get_parameter_declarations()->get_declarator_list();
        if (!analyze_declarations(parameter_list, symbol::symbol_type::PARAMETER)) {
            return nullptr;
        }
    }

    if (parseTree.has_variable_declaration) {
        const auto& variable_list = *parseTree.get_variable_declarations()->get_declarator_list();
        if (!analyze_declarations(variable_list, symbol::symbol_type::VARIABLE)) {
            return nullptr;
        }
    };

    if (parseTree.has_constant_declaration) {
        const auto& constant_list = *parseTree.get_constant_declarations()->get_init_declarator_list();
        if (!analyze_declarations(constant_list, symbol::symbol_type::CONSTANT)) {
            return nullptr;
        }
    };

    bool has_return_statement = false;
    const parser::statement_list_node* statement_list_node = parseTree.get_compund_statement()->get_statement_list();
    std::vector<std::unique_ptr<StatementNode>> statements(statement_list_node->get_number_of_statements());
    for (unsigned int i = 0; i < statement_list_node->get_number_of_statements(); ++i) {
        const statement_node* statement = statement_list_node->get_statement(i);
        if (statement->statement_type == statement_node::statement_type::RETURN_STATEMENT) {
            has_return_statement = true;
            statements[i] = analyze_return_statement(*statement);
        } else {
            statements[i] = analyze_assignment_node(*statement->get_assignment());
        }
        if (!statements[i]) return nullptr;
    }

    if (!has_return_statement) {
        std::cerr << "Error: Missing return statement!" << std::endl;
        return nullptr;
    }

    return std::make_unique<FunctionNode>(std::move(statements), std::move(symbols));
}

std::unique_ptr<IdentifierNode> ASTCreator::analyze_identifier(const parser::identifier_node& node) {
    // Called only for identifiers found in the main program - i.e. that have the statement-list
    // node as an ancestor.
    if (auto identifier_id = identifier_mapping.find(node.get_name()); identifier_id != identifier_mapping.end()) {
        return std::make_unique<IdentifierNode>(identifier_id->second);
    } else {
        std::cerr << "Error: Undeclared identifier \"" << node.get_name() << "\"\n";
        std::cerr << node.get_token().get_code_reference() << std::endl;
        return nullptr;
    }
}

std::unique_ptr<LiteralNode> ASTCreator::analyze_literal(const parser::literal_node& node) {
    return std::make_unique<LiteralNode>(node.get_value());
}

std::unique_ptr<ReturnStatementNode> ASTCreator::analyze_return_statement(const parser::statement_node& node) {
    assert(node.statement_type == parser::statement_node::statement_type::RETURN_STATEMENT);
    std::unique_ptr<ExpressionNode> expression = analyze_expression(*node.get_return_expression());
    if (!expression) return nullptr;
    return std::make_unique<ReturnStatementNode>(std::move(expression));
}

std::unique_ptr<AssignmentNode> ASTCreator::analyze_assignment_node(const parser::assignment_expression_node& node) {
    auto identifier = analyze_identifier(*node.get_identifier());
    if (!identifier) return nullptr;

    // Update symbol table
    auto& symbol = symbols.get(identifier->get_symbol_handle());
    symbol.set_initialized();

    if (symbol.type == symbol::CONSTANT) {
        std::cerr << "Error: Assigning to constant \"" << node.get_identifier()->get_name() << "\" in \n";
        std::cerr << node.get_identifier()->get_token().get_code_reference() << std::endl;
        return nullptr;
    }

    auto expression = analyze_expression(*node.get_expression());

    return std::make_unique<AssignmentNode>(std::move(identifier), std::move(expression));
}

std::unique_ptr<UnaryOperatorASTNode> ASTCreator::analyze_expression(const parser::unary_expression_node& node) {
    using UnaryOperatorType = parser::unary_expression_node::OperationType;
    std::unique_ptr<ExpressionNode> child = analyze_expression(*node.get_expression());
    if (!child) return nullptr;

    return std::make_unique<UnaryOperatorASTNode>(std::move(child),
                                                  node.get_operation() == UnaryOperatorType ::MINUS ? UnaryOperatorASTNode::OperatorType::MINUS : UnaryOperatorASTNode::OperatorType::PLUS);
}

std::unique_ptr<ExpressionNode> ASTCreator::analyze_expression(const parser::multiplicative_expression_node& node) {
    using MultiplicativeExpressionType = parser::multiplicative_expression_node::OperationType;
    auto lhs = analyze_expression(*node.get_lhs_expression());
    if (!node.has_subexpression || !lhs) {
        return lhs;
    }
    auto rhs = analyze_expression(*node.get_rhs_expression());
    if (!rhs) return nullptr;
    BinaryOperatorASTNode::OperatorType operation;
    switch (node.get_operation()) {
        case MultiplicativeExpressionType::MULTIPLY: {
            operation = BinaryOperatorASTNode::OperatorType::MULTIPLY;
            break;
        }
        case MultiplicativeExpressionType::DIVIDE: {
            operation = BinaryOperatorASTNode::OperatorType::DIVIDE;
            break;
        }
    }

    return std::make_unique<BinaryOperatorASTNode>(std::move(lhs), operation, std::move(rhs));
}

std::unique_ptr<ExpressionNode> ASTCreator::analyze_expression(const parser::additive_expression_node& node) {
    using AdditiveExpressionType = parser::additive_expression_node::OperationType;
    auto lhs = analyze_expression(*node.get_lhs_expression());
    if (!node.has_subexpression || !lhs) {
        return lhs;
    }
    auto rhs = analyze_expression(*node.get_rhs_expression());
    if (!rhs) return nullptr;
    BinaryOperatorASTNode::OperatorType operation;
    switch (node.get_operation()) {
        case AdditiveExpressionType::MINUS: {
            operation = BinaryOperatorASTNode::OperatorType::MINUS;
            break;
        }
        case AdditiveExpressionType::PLUS: {
            operation = BinaryOperatorASTNode::OperatorType::PLUS;
            break;
        }
    }

    return std::make_unique<BinaryOperatorASTNode>(std::move(lhs), operation, std::move(rhs));
}

std::unique_ptr<ExpressionNode> ASTCreator::analyze_expression(const parser::primary_expression_node& node) {
    using ExpressionType = parser::primary_expression_node::primary_expression_type;
    switch (node.statement_type) {
        case ExpressionType::IDENTIFIER: {
            auto initializer = analyze_identifier(*node.get_identifier());
            if (!initializer) return nullptr;

            if (!symbols.get(initializer->get_symbol_handle()).initialized) {
                std::cerr << "Error: Variable \"" << node.get_identifier()->get_name() << "\" has not been initialized but is referenced in \n";
                std::cerr << node.get_identifier()->get_token().get_code_reference() << std::endl;
                return nullptr;
            }

            return initializer;
        };
        case ExpressionType::LITERAL: {
            return analyze_literal(*node.get_literal());
        };
        case ExpressionType::ADDITIVE_EXPRESSION: {
            return analyze_expression(*node.get_expression());
        };
    }
    return nullptr;
}
std::unique_ptr<pljit::semantic_analysis::FunctionNode> ASTCreator::CreateAST(const parser::function_definition_node& parseTree) {
    ASTCreator ast_creator;
    return ast_creator.analyze_function(parseTree);
}
