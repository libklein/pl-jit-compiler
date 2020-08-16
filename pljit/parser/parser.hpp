
#ifndef PLJIT_PARSER_HPP
#define PLJIT_PARSER_HPP

#include "parse_tree_nodes.hpp"
#include <ostream>
#include <pljit/lexer/lexer.hpp>
#include <pljit/lexer/token.hpp>
#include <type_traits>
#include <utility>

namespace pljit::parser {
    class parser {
        using Token = lexer::token;
        using TokenType = lexer::TokenType;
        private:
            lexer::lexer& lexer;
            std::optional<Token> next_token;
            bool error_flag = false;
        public:
        explicit parser(lexer::lexer& lexer) : lexer(lexer), next_token(lexer.next()) {};

        bool has_error() const {
            return error_flag;
        }
        private:
        const std::optional<Token>& peek_token() const {
            return next_token;
        };

        std::optional<Token> consume_token(TokenType expected_type) {
            if(!peek_token() || peek_token()->Type() != expected_type) {
                return std::nullopt;
            }
            auto cur_token = next_token;
            next_token = lexer.next();
            if(!next_token) {
                std::stringstream error_message_builder;
                error_message_builder << "Error: Invalid input at ";
                error_message_builder << lexer.get_current_position();
                report_error( error_message_builder.str());
            }
            return cur_token;
        };

        bool expect_token(TokenType expected_type) const {
            return peek_token().has_value() && peek_token()->Type() == expected_type;
        }

        void report_error(std::string_view message) {
            error_flag = true;
            if(peek_token()) {
                std::cerr << message << ": " << peek_token()->get_code_reference() << std::endl;
            } else {
                // TODO When can this happen?
                std::cerr << message << ": " << lexer.get_current_position() << std::endl;
            }
        }

        std::unique_ptr<literal_node> parse_literal();

        std::unique_ptr<identifier_node> parse_identifier();

        std::unique_ptr<terminal_node> parse_terminal_token(TokenType expected_token_type);

        template<class node_parser>
        // TODO is taking node_parser by value fine here?
        std::optional<std::vector<std::pair<std::unique_ptr<terminal_node>, std::result_of_t<node_parser()>>>>
        parse_list_of(TokenType separator_token_type, node_parser child_parser) {
            std::vector<std::pair<std::unique_ptr<terminal_node>, std::result_of_t<node_parser()>>> list;
            while(expect_token(separator_token_type)) {
                // Cannot fail
                auto separator_token = parse_terminal_token(separator_token_type);
                auto child = child_parser();
                if(!child) {
                    report_error("Error parsing list");
                    return std::nullopt;
                }
                list.emplace_back(std::move(separator_token), std::move(child));
            }
            return {std::move(list)};
        }

        std::unique_ptr<init_declarator_node> parse_init_declarator();

        std::unique_ptr<init_declarator_list_node> parse_init_declarator_list();

        std::unique_ptr<declarator_list_node> parse_declarator_list();

        std::unique_ptr<parameter_declaration_node> parse_parameter_declaration();

        std::unique_ptr<variable_declaration_node> parse_variable_declaration();

        std::unique_ptr<constant_declaration_node> parse_constant_declaration();

        std::unique_ptr<compound_statement_node> parse_compound_statement();

        std::unique_ptr<statement_node> parse_statement();;

        std::unique_ptr<assignment_expression_node> parse_assignment() {
            if(auto identifier = parse_identifier(); identifier) {
                if (auto assignment_operator = parse_terminal_token(lexer::VAR_ASSIGNMENT_OP); assignment_operator) {
                    if (auto additive_expression = parse_additive_expression(); additive_expression) {
                        return std::make_unique<assignment_expression_node>(
                            std::move(identifier), std::move(assignment_operator), std::move(additive_expression));
                    }
                }
            }
            return nullptr;
        };

        std::unique_ptr<additive_expression_node> parse_additive_expression() {
            if(auto multiplicative_expression = parse_multiplicative_expression(); multiplicative_expression) {
                std::unique_ptr<terminal_node> operator_symbol;
                if(expect_token(lexer::PLUS_OP)) {
                    operator_symbol = parse_terminal_token(lexer::PLUS_OP);
                } else if (expect_token(lexer::MINUS_OP)) {
                    operator_symbol = parse_terminal_token(lexer::MINUS_OP);
                } else {
                    return std::make_unique<additive_expression_node>(
                        std::move(multiplicative_expression), nullptr, nullptr);
                }

                if(auto additive_expression = parse_additive_expression(); additive_expression) {
                    return std::make_unique<additive_expression_node>(
                        std::move(multiplicative_expression), std::move(operator_symbol), std::move(additive_expression)
                    );
                }
            }
            return nullptr;
        };

        std::unique_ptr<multiplicative_expression_node> parse_multiplicative_expression() {
            if(auto unary_expression = parse_unary_expression(); unary_expression) {
                std::unique_ptr<terminal_node> operator_symbol;
                if(expect_token(lexer::MULT_OP)) {
                    operator_symbol = parse_terminal_token(lexer::MULT_OP);
                } else if (expect_token(lexer::DIV_OP)) {
                    operator_symbol = parse_terminal_token(lexer::DIV_OP);
                } else {
                    return std::make_unique<multiplicative_expression_node>(std::move(unary_expression), nullptr, nullptr);
                }

                if(operator_symbol) {
                    if (auto multiplicative_expression = parse_multiplicative_expression(); multiplicative_expression) {
                        return std::make_unique<multiplicative_expression_node>(
                            std::move(unary_expression), std::move(operator_symbol), std::move(multiplicative_expression));
                    }
                }
            }
            return nullptr;
        }

        std::unique_ptr<unary_expression_node> parse_unary_expression() {
            std::unique_ptr<terminal_node> operator_symbol(nullptr);
            if(expect_token(lexer::PLUS_OP)) {
                operator_symbol = parse_terminal_token(lexer::PLUS_OP);
            } else if (expect_token(lexer::MINUS_OP)) {
                operator_symbol = parse_terminal_token(lexer::MINUS_OP);
            }
            if(auto primary_expression = parse_primary_expression(); primary_expression) {
                return std::make_unique<unary_expression_node>(std::move(operator_symbol), std::move(primary_expression));
            }
            return nullptr;
        }

        std::unique_ptr<primary_expression_node> parse_primary_expression() {
            if(expect_token(lexer::IDENTIFIER)) {
                if(auto identifier = parse_identifier(); identifier) {
                    return std::make_unique<primary_expression_node>(std::move(identifier));
                }
            } else if (expect_token(lexer::LITERAL)) {
                if(auto literal = parse_literal(); literal) {
                    return std::make_unique<primary_expression_node>(std::move(literal));
                }
            } else if(expect_token(lexer::L_BRACKET)) {
                if(auto l_bracket = parse_terminal_token(lexer::L_BRACKET); l_bracket) {
                    if(auto additive_expression = parse_additive_expression(); additive_expression) {
                        if (auto r_bracket = parse_terminal_token(lexer::R_BRACKET); r_bracket) {
                            return std::make_unique<primary_expression_node>(
                                std::move(l_bracket), std::move(additive_expression), std::move(r_bracket));
                        }
                    }
                }
            }
            return nullptr;
        };

        std::unique_ptr<statement_list_node> parse_statement_list() {
            // Statement {; statement}
            if (auto statement = parse_statement(); statement) {
                if (auto statement_tail = parse_list_of(lexer::STATEMENT_TERMINATOR, [this]() { return this->parse_statement(); }); statement_tail) {
                    return std::make_unique<statement_list_node>(std::move(statement), std::move(*statement_tail));
                }
            }
            return nullptr;
        }

        public:
        std::unique_ptr<function_definition_node> parse_function_definition() {
            auto param_decl = parse_parameter_declaration();
            if(has_error()) { // A non-recoverable error has been detected
                return nullptr;
            }
            auto var_decl = parse_variable_declaration();
            if(has_error()) { // A non-recoverable error has been detected
                return nullptr;
            }
            auto const_decl = parse_constant_declaration();
            if(has_error()) { // A non-recoverable error has been detected
                return nullptr;
            }
            if(auto compound_statement = parse_compound_statement(); compound_statement) {
                if (auto program_terminator = parse_terminal_token(lexer::PROGRAM_TERMINATOR); program_terminator) {
                    if (!peek_token().has_value() || peek_token()->Type() != lexer::EOS) {
                        // Tokens remaining - program must be syntactically invalid.
                        report_error("Error parsing function definition. Input after \".\"");
                        return nullptr;
                    }

                    return std::make_unique<function_definition_node>(
                        std::move(param_decl),
                        std::move(var_decl),
                        std::move(const_decl),
                        std::move(compound_statement),
                        std::move(program_terminator));
                }
            }
            return nullptr;
        }

    };

}

#endif //PLJIT_PARSER_HPP
