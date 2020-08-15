
#include "ast_creation_visitor.hpp"
#include "pljit/lexer/token.hpp"
#include <memory>
#include <cassert>

using namespace pljit;

/**
 * Visitor pattern would save me calls like
 * parseStatement(node.getCompoundStatement().getStatementList().getChildren())
 * but i have to type-cast all the time and overwriting next_node is not particulary nice either.
 */

namespace {
    /*pljit::lexer::token get_token(const parser::node_ptr& terminal_node) {
        assert(terminal_node->get_type() == parser::TEXT_NODE);
        return static_cast<const parser::terminal_node&>(*terminal_node).get_token();
    }*/

    bool is_expression(const std::unique_ptr<semantic_analysis::ASTNode>& node) {
        return node && (node->getType() == pljit::semantic_analysis::ASTNode::BinaryOperation
                        || node->getType() == pljit::semantic_analysis::ASTNode::UnaryOperation
                        || node->getType() == pljit::semantic_analysis::ASTNode::Identifier
                        || node->getType() == pljit::semantic_analysis::ASTNode::Literal
                        );
    }

    template <class node_type>
    std::unique_ptr<node_type> node_cast(std::unique_ptr<semantic_analysis::ASTNode>& node) {
        return std::unique_ptr<node_type>(static_cast<node_type*>(node.release()));
    }
} // namespace

auto semantic_analysis::ast_creation_visitor::AnalyzeParseTree(const parser::function_defition_node& parse_tree) -> std::pair<std::unique_ptr<FunctionNode>, symbol_table> {
    ast_creation_visitor ast_creator;
    parse_tree.accept(ast_creator);
    return ast_creator.release_result();
}

auto semantic_analysis::ast_creation_visitor::lookup_identifier(std::string_view name) const -> std::optional<symbol_handle> {
    if(auto symbol_iter = identifier_mapping.find(name); symbol_iter != identifier_mapping.end()) {
        return {symbol_iter->second};
    } else {
        return std::nullopt;
    }
}

auto semantic_analysis::ast_creation_visitor::register_symbol(const parser::identifier_node& node, semantic_analysis::symbol::symbol_type type, std::optional<int64_t> value) -> std::pair<symbol_handle, bool> {
    std::string_view identifier = node.get_token().get_code_reference().str();
    if(auto handle = lookup_identifier(identifier); handle) return {*handle, false};
    symbol_handle id = symbols.insert(node.get_token().get_code_reference(), type, value);
    identifier_mapping.emplace(identifier, id);
    return {id, true};
}

void semantic_analysis::ast_creation_visitor::visit(const parser::init_declarator_node& node) {
    if(auto [handle, success] = register_symbol(*node.get_identifier(), symbol::CONSTANT, node.get_value()->get_value()); !success){
        std::cerr << "Error: Redeclaration of identifier \"" << node.get_identifier()->get_token().get_code_reference().str()
                  << "\" originally defined here: ";
        std::cerr << symbols.get(handle).declaration << std::endl;
        construction_failed = true;
    }
}

void semantic_analysis::ast_creation_visitor::visit(const parser::declarator_list_node& node) {
    for(unsigned int i = 0; i < node.get_number_of_declarations(); ++i) {
        // Distinguish between variables and parameters via next_symbol_type, which is set when
        // variable/parameter declaration nodes are encountered.
        if(auto [handle, success] = register_symbol(*node.get_declaration(i), next_symbol_type, std::nullopt); !success) {
            std::cerr << "Error: Redeclaration of identifier \""
                      << node.get_declaration(i)->get_token().get_code_reference().str()
                      << "\" originally defined here: ";
            std::cerr << symbols.get(handle).declaration << std::endl;
            construction_failed = true;
        }
    }
}

void semantic_analysis::ast_creation_visitor::visit(const parser::identifier_node& node) {
    assert(!next_node);
    auto symbol = lookup_identifier(node.get_name());
    if(symbol) {
        next_node = std::make_unique<semantic_analysis::IdentifierNode>(*symbol);
    } else {
        std::cerr << "Error: Undeclared identifier \"" << node.get_name() << "\"\n";
        std::cerr << node.get_token().get_code_reference() << std::endl;
        construction_failed = true;
    }
}

void semantic_analysis::ast_creation_visitor::visit(const parser::literal_node& node) {
    assert(!next_node);
    assert(node.get_children().size() == 1 && node.get_children().front()->get_type() == parser::TEXT_NODE);
    next_node = std::make_unique<semantic_analysis::LiteralNode>(node.get_value());
}

void semantic_analysis::ast_creation_visitor::visit(const parser::terminal_node&) {}

void semantic_analysis::ast_creation_visitor::visit(const parser::additive_expression_node& node) {
    // Is either a multiplicative expression - in this case we dont do anything
    // or actually an addition
    if(node.has_subexpression) {
        // Parse lhs
        node.get_lhs_expression()->accept(*this);
        if(construction_failed) return;
        assert(is_expression(next_node));
        std::unique_ptr<ExpressionNode> lhs(static_cast<ExpressionNode*>(next_node.release()));
        // Parse op type
        // TODO Move to parser tree - otherwise coupling between ast and lexer (?)
        auto op_type = node.get_operator()->get_token().Type() == lexer::PLUS_OP ?
            BinaryOperatorASTNode::OperatorType::PLUS
            : BinaryOperatorASTNode::OperatorType::MINUS;
        // Parse rhs
        node.get_rhs_expression()->accept(*this);
        if(construction_failed) return;
        assert(is_expression(next_node));
        std::unique_ptr<ExpressionNode> rhs(static_cast<ExpressionNode*>(next_node.release()));
        next_node = std::make_unique<BinaryOperatorASTNode>(std::move(lhs), op_type, std::move(rhs));
    } else {
        node.get_lhs_expression()->accept(*this);
    }
}

void semantic_analysis::ast_creation_visitor::visit(const parser::assignment_expression_node& node) {
    // Parse identifier
    node.get_identifier()->accept(*this);
    if(construction_failed) return;
    assert(next_node->getType() == ASTNode::Identifier);
    std::unique_ptr<IdentifierNode> identifer(static_cast<IdentifierNode*>(next_node.release()));

    // Update symbol table
    symbols.get(identifer->get_symbol_handle()).initialized = true;

    if(symbols.get(identifer->get_symbol_handle()).type == symbol::CONSTANT) {
        std::cerr << "Error: Assigning to constant \"" << node.get_identifier()->get_name() << "\" in \n";
        std::cerr << node.get_identifier()->get_token().get_code_reference() << std::endl;
        construction_failed = true;
    }

    // Parse expression
    node.get_expression()->accept(*this);
    if(construction_failed) return;
    assert(is_expression(next_node));
    std::unique_ptr<ExpressionNode> expression(node_cast<ExpressionNode>(next_node));
    next_node = std::make_unique<AssignmentNode>(std::move(identifer), std::move(expression));
}

void semantic_analysis::ast_creation_visitor::visit(const parser::compound_statement_node& node) {
    node.get_statement_list()->accept(*this);
}

void semantic_analysis::ast_creation_visitor::visit(const parser::constant_declaration_node& node) {
    // Not nessesary in the current implementation (init-declarations can only be constant). But
    // perhaps nessesary later.
    next_symbol_type = symbol::CONSTANT;
    node.get_init_declarator_list()->accept(*this);
}

void semantic_analysis::ast_creation_visitor::visit(const parser::init_declarator_list_node& node) {
    for(unsigned int i = 0; i < node.get_number_of_declarations(); ++i) {
        node.get_declaration(i)->accept(*this);
        if(construction_failed) return;
    }
}

void semantic_analysis::ast_creation_visitor::visit(const parser::parameter_declaration_node& node) {
    next_symbol_type = symbol::PARAMETER;
    node.get_declarator_list()->accept(*this);
}

void semantic_analysis::ast_creation_visitor::visit(const parser::variable_declaration_node& node) {
    next_symbol_type = symbol::VARIABLE;
    node.get_declarator_list()->accept(*this);
}

void semantic_analysis::ast_creation_visitor::visit(const parser::multiplicative_expression_node& node) {
    if(node.has_subexpression) {
        // Parse lhs
        node.get_lhs_expression()->accept(*this);
        if(construction_failed) return;

        assert(is_expression(next_node));
        auto lhs = node_cast<ExpressionNode>(next_node);
        // Parse op type
        auto op_type = node.get_operator()->get_token().Type() == lexer::MULT_OP ?
                       BinaryOperatorASTNode::OperatorType::MULTIPLY : BinaryOperatorASTNode::OperatorType::DIVIDE;
        // Parse rhs
        node.get_rhs_expression()->accept(*this);
        if(construction_failed) return;

        assert(is_expression(next_node));
        auto rhs = node_cast<ExpressionNode>(next_node);
        next_node = std::make_unique<BinaryOperatorASTNode>(std::move(lhs), op_type, std::move(rhs));
    } else {
        node.get_lhs_expression()->accept(*this);
    }
}

void semantic_analysis::ast_creation_visitor::visit(const parser::primary_expression_node& node) {
    using expression_type = parser::primary_expression_node::primary_expression_type;
    if(node.statement_type == expression_type ::ADDITIVE_EXPRESSION) {
        node.get_expression()->accept(*this);
        assert(is_expression(next_node));
    } else if (node.statement_type == expression_type::LITERAL) {
        node.get_literal()->accept(*this);
        assert(next_node->getType() == ASTNode::Literal);
    } else {
        node.get_identifier()->accept(*this);
        if(construction_failed) return;
        if(!symbols.get(static_cast<IdentifierNode&>(*next_node).get_symbol_handle()).initialized) {
            std::cerr << "Error: Variable \"" << node.get_identifier()->get_name() << "\" has not been initialized but is referenced in \n";
            std::cerr << node.get_identifier()->get_token().get_code_reference() << std::endl;
            construction_failed = true;
        }
        assert(next_node->getType() == ASTNode::Identifier);
    }
}

void semantic_analysis::ast_creation_visitor::visit(const parser::statement_list_node&) {}

void semantic_analysis::ast_creation_visitor::visit(const parser::statement_node& node) {
    if(node.statement_type == parser::statement_node::statement_type::RETURN_STATEMENT) {
        // TODO Mark found return
        node.get_return_expression()->accept(*this);
        if(construction_failed) return;
        assert(is_expression(next_node));
        auto expression = node_cast<ExpressionNode>(next_node);
        next_node = std::make_unique<ReturnStatementNode>(std::move(expression));
    } else {
        node.get_assignment()->accept(*this);
    }
}

void semantic_analysis::ast_creation_visitor::visit(const parser::function_defition_node& node) {
    construction_failed = false;
    // Parse declarations
    if(node.has_parameter_declaration) node.get_parameter_declarations()->accept(*this);
    if(construction_failed) return;

    if(node.has_variable_declaration) node.get_variable_declarations()->accept(*this);
    if(construction_failed) return;

    if(node.has_constant_declaration) node.get_constant_declarations()->accept(*this);
    if(construction_failed) return;

    bool returns = false;

    const parser::statement_list_node* statement_list_node = node.get_compund_statement()->get_statement_list();
    std::vector<std::unique_ptr<StatementNode>> statements(statement_list_node->get_number_of_statements());
    for(unsigned int i = 0; i < statement_list_node->get_number_of_statements(); ++i) {
        statement_list_node->get_statement(i)->accept(*this);
        if(construction_failed) return;

        assert(next_node->getType() == ASTNode::ReturnStatement || next_node->getType() == ASTNode::Assignment);
        returns |= (next_node->getType() == ASTNode::ReturnStatement);
        statements[i] = node_cast<StatementNode>(next_node);
    }

    if(!returns) {
        std::cerr << "Error: Missing return statement!" << std::endl;
        construction_failed = true;
    }

    if(!construction_failed) root = std::make_unique<FunctionNode>(std::move(statements));
}

void semantic_analysis::ast_creation_visitor::visit(const parser::unary_expression_node& node) {
    node.get_expression()->accept(*this);
    if(construction_failed) return;

    assert(is_expression(next_node));
    auto expression_node = node_cast<ExpressionNode>(next_node);
    if(node.has_unary_operator && node.get_unary_operator()->get_token().Type() == lexer::MINUS_OP) {
        next_node = std::make_unique<UnaryOperatorASTNode>(std::move(expression_node), UnaryOperatorASTNode::OperatorType::MINUS);
    } else {
        next_node = std::make_unique<UnaryOperatorASTNode>(std::move(expression_node), UnaryOperatorASTNode::OperatorType::PLUS);
    }
}
