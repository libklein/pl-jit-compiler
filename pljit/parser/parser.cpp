#include "parser.hpp"

using namespace pljit::parser;

namespace pljit::parser {

namespace {

template <class node_type>
std::unique_ptr<node_type> create_node() {
    return std::make_unique<node_type>();
}

} // namespace

std::unique_ptr<literal_node> parser::parse_literal() {
    auto literal_token = consume_token(TokenType::LITERAL);
    if (!literal_token) {
        report_error("Error parsing literal", {});
        return nullptr;
    }
    return std::make_unique<literal_node>(std::make_unique<terminal_node>(*literal_token), literal_token->get_code_reference());
}
std::unique_ptr<identifier_node> parser::parse_identifier() {
    auto identifier_token = consume_token(TokenType::IDENTIFIER);
    if (!identifier_token) {
        report_error("Error parsing identifier", {});
        return nullptr;
    }
    return std::make_unique<identifier_node>(std::make_unique<terminal_node>(*identifier_token), identifier_token->get_code_reference());
}
std::unique_ptr<terminal_node> parser::parse_terminal_token(parser::TokenType expected_token_type) {
    auto token = consume_token(expected_token_type);
    if (!token) {
        report_error("Error parsing terminal symbol", {});
        return nullptr;
    }
    return std::make_unique<terminal_node>(*token);
}
std::unique_ptr<init_declarator_node> parser::parse_init_declarator() {
    auto source_pos = peek_token()->get_code_reference();
    if (auto identifier = parse_identifier(); identifier) {
        if (auto assignment_op = parse_terminal_token(lexer::INIT_ASSIGNMENT_OP); assignment_op) {
            if (auto literal = parse_literal(); literal) {
                source_pos.extend(literal->getCodeReference());
                return std::make_unique<init_declarator_node>(source_pos,
                                                              std::move(identifier), std::move(assignment_op), std::move(literal));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<init_declarator_list_node> parser::parse_init_declarator_list() {
    auto source_pos = peek_token()->get_code_reference();
    if (auto init_declarator = parse_init_declarator(); init_declarator) {
        if (auto tail = parse_list_of(lexer::SEPARATOR, [this]() { return this->parse_init_declarator(); }); tail) {
            if (tail->empty()) {
                source_pos.extend(init_declarator->getCodeReference());
            } else {
                source_pos.extend(tail->back().second->getCodeReference());
            }
            return std::make_unique<init_declarator_list_node>(source_pos, std::move(init_declarator), std::move(*tail));
        }
    }
    return nullptr;
}
std::unique_ptr<declarator_list_node> parser::parse_declarator_list() {
    auto source_pos = peek_token()->get_code_reference();
    if (auto identifier = parse_identifier(); identifier) {
        if (auto identifier_tail = parse_list_of(lexer::SEPARATOR, [this]() { return this->parse_identifier(); }); identifier_tail) {
            if (identifier_tail->empty()) {
                source_pos.extend(identifier->getCodeReference());
            } else {
                source_pos.extend(identifier_tail->back().second->getCodeReference());
            }
            return std::make_unique<declarator_list_node>(source_pos, std::move(identifier), std::move(*identifier_tail));
        }
    }
    return nullptr;
}
std::unique_ptr<parameter_declaration_node> parser::parse_parameter_declaration() {
    auto source_pos = peek_token()->get_code_reference();
    if (!expect_token(lexer::PARAM)) return nullptr;
    if (auto param_kw = parse_terminal_token(lexer::PARAM); param_kw) {
        if (auto declarator_list = parse_declarator_list(); declarator_list) {
            if (auto statement_terminator = parse_terminal_token(lexer::STATEMENT_TERMINATOR); statement_terminator) {
                source_pos.extend(statement_terminator->getCodeReference());
                return std::make_unique<parameter_declaration_node>(
                    source_pos,
                    std::move(param_kw),
                    std::move(declarator_list),
                    std::move(statement_terminator));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<variable_declaration_node> parser::parse_variable_declaration() {
    auto source_pos = peek_token()->get_code_reference();
    if (!expect_token(lexer::VAR)) return nullptr;
    if (auto var_kw = parse_terminal_token(lexer::VAR); var_kw) {
        if (auto declarator_list = parse_declarator_list(); declarator_list) {
            if (auto statement_terminator = parse_terminal_token(lexer::STATEMENT_TERMINATOR); statement_terminator) {
                source_pos.extend(statement_terminator->getCodeReference());
                return std::make_unique<variable_declaration_node>(
                    source_pos,
                    std::move(var_kw),
                    std::move(declarator_list),
                    std::move(statement_terminator));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<constant_declaration_node> parser::parse_constant_declaration() {
    auto source_pos = peek_token()->get_code_reference();
    if (!expect_token(lexer::CONST)) return nullptr;
    if (auto const_kw = parse_terminal_token(lexer::CONST); const_kw) {
        if (auto init_declarator_list = parse_init_declarator_list(); init_declarator_list) {
            if (auto statement_terminator = parse_terminal_token(lexer::STATEMENT_TERMINATOR); statement_terminator) {
                source_pos.extend(statement_terminator->getCodeReference());
                return std::make_unique<constant_declaration_node>(
                    source_pos,
                    std::move(const_kw),
                    std::move(init_declarator_list),
                    std::move(statement_terminator));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<compound_statement_node> parser::parse_compound_statement() {
    auto source_pos = peek_token()->get_code_reference();
    if (auto begin_kw = parse_terminal_token(lexer::BEGIN); begin_kw) {
        if (auto statement_list = parse_statement_list(); statement_list) {
            if (auto end_kw = parse_terminal_token(lexer::END); end_kw) {
                source_pos.extend(end_kw->getCodeReference());
                return std::make_unique<compound_statement_node>(
                    source_pos,
                    std::move(begin_kw),
                    std::move(statement_list),
                    std::move(end_kw));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<statement_node> parser::parse_statement() {
    auto source_pos = peek_token()->get_code_reference();
    if (expect_token(lexer::RETURN)) {
        if (auto return_kw = parse_terminal_token(lexer::RETURN); return_kw) {
            if (auto additive_expression = parse_additive_expression(); additive_expression) {
                source_pos.extend(additive_expression->getCodeReference());
                return std::make_unique<statement_node>(source_pos, std::move(return_kw), std::move(additive_expression));
            }
        }
    } else {
        if (auto assignment = parse_assignment(); assignment) {
            source_pos.extend(assignment->getCodeReference());
            return std::make_unique<statement_node>(source_pos, std::move(assignment));
        }
    }
    return nullptr;
}
std::unique_ptr<assignment_expression_node> parser::parse_assignment() {
    auto source_pos = peek_token()->get_code_reference();
    if (auto identifier = parse_identifier(); identifier) {
        if (auto assignment_operator = parse_terminal_token(lexer::VAR_ASSIGNMENT_OP); assignment_operator) {
            if (auto additive_expression = parse_additive_expression(); additive_expression) {
                source_pos.extend(additive_expression->getCodeReference());
                return std::make_unique<assignment_expression_node>(
                    source_pos, std::move(identifier), std::move(assignment_operator), std::move(additive_expression));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<additive_expression_node> parser::parse_additive_expression() {
    auto source_pos = peek_token()->get_code_reference();
    if (auto multiplicative_expression = parse_multiplicative_expression(); multiplicative_expression) {
        std::unique_ptr<terminal_node> operator_symbol;
        if (expect_token(lexer::PLUS_OP)) {
            operator_symbol = parse_terminal_token(lexer::PLUS_OP);
        } else if (expect_token(lexer::MINUS_OP)) {
            operator_symbol = parse_terminal_token(lexer::MINUS_OP);
        } else {
            source_pos.extend(multiplicative_expression->getCodeReference());
            return std::make_unique<additive_expression_node>(
                source_pos, std::move(multiplicative_expression), nullptr, nullptr);
        }

        if (auto additive_expression = parse_additive_expression(); additive_expression) {
            source_pos.extend(additive_expression->getCodeReference());
            return std::make_unique<additive_expression_node>(
                source_pos, std::move(multiplicative_expression), std::move(operator_symbol), std::move(additive_expression));
        }
    }
    return nullptr;
}
std::unique_ptr<multiplicative_expression_node> parser::parse_multiplicative_expression() {
    auto source_pos = peek_token()->get_code_reference();
    if (auto unary_expression = parse_unary_expression(); unary_expression) {
        std::unique_ptr<terminal_node> operator_symbol;
        if (expect_token(lexer::MULT_OP)) {
            operator_symbol = parse_terminal_token(lexer::MULT_OP);
        } else if (expect_token(lexer::DIV_OP)) {
            operator_symbol = parse_terminal_token(lexer::DIV_OP);
        } else {
            source_pos.extend(unary_expression->getCodeReference());
            return std::make_unique<multiplicative_expression_node>(source_pos, std::move(unary_expression), nullptr, nullptr);
        }

        if (operator_symbol) {
            if (auto multiplicative_expression = parse_multiplicative_expression(); multiplicative_expression) {
                source_pos.extend(multiplicative_expression->getCodeReference());
                return std::make_unique<multiplicative_expression_node>(
                    source_pos, std::move(unary_expression), std::move(operator_symbol), std::move(multiplicative_expression));
            }
        }
    }
    return nullptr;
}
std::unique_ptr<unary_expression_node> parser::parse_unary_expression() {
    auto source_pos = peek_token()->get_code_reference();
    std::unique_ptr<terminal_node> operator_symbol(nullptr);
    if (expect_token(lexer::PLUS_OP)) {
        operator_symbol = parse_terminal_token(lexer::PLUS_OP);
    } else if (expect_token(lexer::MINUS_OP)) {
        operator_symbol = parse_terminal_token(lexer::MINUS_OP);
    }
    if (auto primary_expression = parse_primary_expression(); primary_expression) {
        source_pos.extend(primary_expression->getCodeReference());
        return std::make_unique<unary_expression_node>(source_pos, std::move(operator_symbol), std::move(primary_expression));
    }
    return nullptr;
}
std::unique_ptr<primary_expression_node> parser::parse_primary_expression() {
    auto source_pos = peek_token()->get_code_reference();
    if (expect_token(lexer::IDENTIFIER)) {
        if (auto identifier = parse_identifier(); identifier) {
            source_pos.extend(identifier->getCodeReference());
            return std::make_unique<primary_expression_node>(source_pos, std::move(identifier));
        }
    } else if (expect_token(lexer::LITERAL)) {
        if (auto literal = parse_literal(); literal) {
            source_pos.extend(literal->getCodeReference());
            return std::make_unique<primary_expression_node>(source_pos, std::move(literal));
        }
    } else if (expect_token(lexer::L_BRACKET)) {
        if (auto l_bracket = parse_terminal_token(lexer::L_BRACKET); l_bracket) {
            if (auto additive_expression = parse_additive_expression(); additive_expression) {
                if (auto r_bracket = parse_terminal_token(lexer::R_BRACKET); r_bracket) {
                    source_pos.extend(r_bracket->getCodeReference());
                    return std::make_unique<primary_expression_node>(source_pos,
                                                                     std::move(l_bracket), std::move(additive_expression), std::move(r_bracket));
                }
            }
        }
    }
    return nullptr;
}
std::unique_ptr<statement_list_node> parser::parse_statement_list() {
    auto source_pos = peek_token()->get_code_reference();
    // Statement {; statement}
    if (auto statement = parse_statement(); statement) {
        if (auto statement_tail = parse_list_of(lexer::STATEMENT_TERMINATOR, [this]() { return this->parse_statement(); }); statement_tail) {
            if (statement_tail->empty()) {
                source_pos.extend(statement->getCodeReference());
            } else {
                source_pos.extend(statement_tail->back().second->getCodeReference());
            }
            return std::make_unique<statement_list_node>(source_pos, std::move(statement), std::move(*statement_tail));
        }
    }
    return nullptr;
}
std::unique_ptr<function_definition_node> parser::parse_function_definition() {
    error_flag = false; // Reset error
    auto source_pos = peek_token()->get_code_reference();
    auto param_decl = parse_parameter_declaration();
    if (has_error()) { // A non-recoverable error has been detected
        return nullptr;
    }
    auto var_decl = parse_variable_declaration();
    if (has_error()) { // A non-recoverable error has been detected
        return nullptr;
    }
    auto const_decl = parse_constant_declaration();
    if (has_error()) { // A non-recoverable error has been detected
        return nullptr;
    }
    if (auto compound_statement = parse_compound_statement(); compound_statement) {
        if (auto program_terminator = parse_terminal_token(lexer::PROGRAM_TERMINATOR); program_terminator) {
            source_pos.extend(program_terminator->getCodeReference());
            if (!peek_token().has_value() || peek_token()->Type() != lexer::EOS) {
                // Tokens remaining - program must be syntactically invalid.
                report_error("Error parsing function definition. Input after \".\"", {});
                return nullptr;
            }

            return std::make_unique<function_definition_node>(
                source_pos,
                std::move(param_decl),
                std::move(var_decl),
                std::move(const_decl),
                std::move(compound_statement),
                std::move(program_terminator));
        }
    }
    return nullptr;
}
void parser::report_error(std::string_view message, std::optional<source_management::SourceFragment> position) {
    if (has_error()) return;
    error_flag = true;
    if (position) {
        std::cerr << message << ": " << *position << std::endl;
        return;
    } else if (peek_token()) {
        std::cerr << message << ": " << peek_token()->get_code_reference() << std::endl;
    } else {
        std::cerr << message << std::endl;
    }
}
bool parser::expect_token(parser::TokenType expected_type) const {
    return peek_token().has_value() && peek_token()->Type() == expected_type;
}
auto parser::consume_token(parser::TokenType expected_type) -> std::optional<Token> {
    if (!peek_token() || peek_token()->Type() != expected_type) {
        return std::nullopt;
    }
    auto cur_token = next_token;
    next_token = lexer.next();
    if (!next_token) {
        std::stringstream error_message_builder;
        error_message_builder << "Error: Invalid input at ";
        error_message_builder << lexer.get_current_position();
        report_error(error_message_builder.str(), {});
    }
    return cur_token;
}
bool parser::has_error() const {
    return error_flag;
}
auto parser::peek_token() const -> const std::optional<Token>& {
    return next_token;
}
parser::parser(lexer::lexer& lexer) : lexer(lexer), next_token(lexer.next()) {}
} // namespace pljit::parser
