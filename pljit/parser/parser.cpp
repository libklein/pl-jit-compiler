#include "parser.hpp"

using namespace pljit::parser;

namespace pljit::parser {

std::unique_ptr<literal_node> parser::parse_literal() {
    auto literal_token = consume_token(TokenType::LITERAL);
    if(!literal_token) {
        report_error("Error parsing literal");
        return nullptr;
    }
    return std::make_unique<literal_node>(std::make_unique<terminal_node>(*literal_token));
}
std::unique_ptr<identifier_node> parser::parse_identifier() {
    auto identifier_token = consume_token(TokenType::IDENTIFIER);
    if(!identifier_token) {
        report_error("Error parsing identifier");
        return nullptr;
    }
    return std::make_unique<identifier_node>(std::make_unique<terminal_node>(*identifier_token));
}
std::unique_ptr<terminal_node> parser::parse_terminal_token(parser::TokenType expected_token_type) {
    auto token = consume_token(expected_token_type);
    if(!token) {
        report_error("Error parsing terminal symbol");
        return nullptr;
    }
    return std::make_unique<terminal_node>(*token);
}
std::unique_ptr<init_declarator_node> parser::parse_init_declarator() {
    if (auto identifier = parse_identifier(); identifier) {
        if(auto assignment_op = parse_terminal_token(lexer::INIT_ASSIGNMENT_OP); assignment_op) {
            if(auto literal = parse_literal(); literal) {
                return std::make_unique<init_declarator_node>(
                    std::move(identifier), std::move(assignment_op), std::move(literal));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<init_declarator_list_node> parser::parse_init_declarator_list() {
    if (auto init_declarator = parse_init_declarator(); init_declarator) {
        if (auto tail = parse_list_of(lexer::SEPARATOR, [this]() { return this->parse_init_declarator(); }); tail) {
            return std::make_unique<init_declarator_list_node>(std::move(init_declarator), std::move(*tail));
        }
    }
    return nullptr;
}
std::unique_ptr<declarator_list_node> parser::parse_declarator_list() {
    if (auto identifier = parse_identifier(); identifier) {
        if (auto identifier_tail = parse_list_of(lexer::SEPARATOR, [this]() { return this->parse_identifier(); }); identifier_tail) {
            return std::make_unique<declarator_list_node>(std::move(identifier), std::move(*identifier_tail));
        }
    }
    return nullptr;
}
std::unique_ptr<parameter_declaration_node> parser::parse_parameter_declaration() {
    if(!expect_token(lexer::PARAM)) return nullptr;
    if(auto param_kw = parse_terminal_token(lexer::PARAM); param_kw) {
        if(auto declarator_list = parse_declarator_list(); declarator_list) {
            if(auto statement_terminator = parse_terminal_token(lexer::STATEMENT_TERMINATOR); statement_terminator) {
                return std::make_unique<parameter_declaration_node>(
                    std::move(param_kw),
                    std::move(declarator_list),
                    std::move(statement_terminator));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<variable_declaration_node> parser::parse_variable_declaration() {
    if(!expect_token(lexer::VAR)) return nullptr;
    if(auto var_kw = parse_terminal_token(lexer::VAR); var_kw) {
        if(auto declarator_list = parse_declarator_list(); declarator_list) {
            if(auto statement_terminator = parse_terminal_token(lexer::STATEMENT_TERMINATOR); statement_terminator) {
                return std::make_unique<variable_declaration_node>(
                    std::move(var_kw),
                    std::move(declarator_list),
                    std::move(statement_terminator));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<constant_declaration_node> parser::parse_constant_declaration() {
    if(!expect_token(lexer::CONST)) return nullptr;
    if(auto const_kw = parse_terminal_token(lexer::CONST); const_kw) {
        if(auto init_declarator_list = parse_init_declarator_list(); init_declarator_list) {
            if(auto statement_terminator = parse_terminal_token(lexer::STATEMENT_TERMINATOR); statement_terminator) {
                return std::make_unique<constant_declaration_node>(
                    std::move(const_kw),
                    std::move(init_declarator_list),
                    std::move(statement_terminator));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<compound_statement_node> parser::parse_compound_statement() {
    if(auto begin_kw = parse_terminal_token(lexer::BEGIN); begin_kw) {
        if(auto statement_list = parse_statement_list(); statement_list) {
            if(auto end_kw = parse_terminal_token(lexer::END); end_kw) {
                return std::make_unique<compound_statement_node>(
                    std::move(begin_kw),
                    std::move(statement_list),
                    std::move(end_kw));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<statement_node> parser::parse_statement() {
    if(expect_token(lexer::RETURN)) {
        if(auto return_kw = parse_terminal_token(lexer::RETURN); return_kw) {
            if(auto additive_expression = parse_additive_expression(); additive_expression) {
                return std::make_unique<statement_node>(std::move(return_kw), std::move(additive_expression));
            }
        }
    } else {
        if(auto assignment = parse_assignment(); assignment) {
            return std::make_unique<statement_node>(std::move(assignment));
        }
    }
    return nullptr;
}
} // namespace pljit::parser
