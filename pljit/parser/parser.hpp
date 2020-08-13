
#ifndef PLJIT_PARSER_HPP
#define PLJIT_PARSER_HPP

#include "parse_tree_nodes.hpp"
#include <ostream>
#include <pljit/lexer/lexer.hpp>
#include <pljit/lexer/token.hpp>
#include <type_traits>
#include <utility>

/**
* Drawbacks of the current implementation:
* constructors can be unsafe, i.e. when parsing via
*      node(parse_1(), parse_2())
* c++ does not guarantee ordering in this case
 *
 * TODO Switch to TokenIterator
*/

/**
 *
 * Value-based Tree
 * Advantages:
 *  Little overhead (memory allocation)
 *  Sequencing with {} operators
 *  Static type safety (? Is guaranteed by solver but if we ever wish to make the parser into it's own library)
 * Disadvantages:
 *  Fairly verbose
 *  Inflexible (?)
 *  We need a visitor accept impl for each node anyway
 *  Cannot work with subtrees unless CRTP is used
 *
 * Virtual Tree
 * Advantages:
 *  Simple implementation - we know that each node is correct
 * Drawbacks:
 *  No type safety unless we constrain our constructors, but the the
 *      main advantage (simple implementation) is somewhat mitigate. Also, not constraining our constructors is bad behaviour.
 *  We still need to implement a class for each non-terminal anyway...
 */

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

        std::unique_ptr<literal_node> parse_literal() {
            auto literal_token = consume_token(TokenType::LITERAL);
            if(!literal_token) {
                report_error("Error parsing literal");
                return nullptr;
            }
            return std::make_unique<literal_node>(std::make_unique<terminal_node>(*literal_token));
        }

        std::unique_ptr<identifier_node> parse_identifier() {
            auto identifier_token = consume_token(TokenType::IDENTIFIER);
            if(!identifier_token) {
                report_error("Error parsing identifier");
                return nullptr;
            }
            return std::make_unique<identifier_node>(std::make_unique<terminal_node>(*identifier_token));
        }

        std::unique_ptr<terminal_node> parse_terminal_token(TokenType expected_token_type) {
            auto token = consume_token(expected_token_type);
            if(!token) {
                report_error("Error parsing terminal symbol");
                return nullptr;
            }
            return std::make_unique<terminal_node>(*token);
        }

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

        std::unique_ptr<init_declarator_node> parse_init_declarator() {
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

        std::unique_ptr<init_declarator_list_node> parse_init_declarator_list() {
            if (auto init_declarator = parse_init_declarator(); init_declarator) {
                if (auto tail = parse_list_of(lexer::SEPARATOR, [this]() { return this->parse_init_declarator(); }); tail) {
                    return std::make_unique<init_declarator_list_node>(std::move(init_declarator), std::move(*tail));
                }
            }
            return nullptr;
        }

        std::unique_ptr<declarator_list_node> parse_declarator_list() {
            if (auto identifier = parse_identifier(); identifier) {
                if (auto identifier_tail = parse_list_of(lexer::SEPARATOR, [this]() { return this->parse_identifier(); }); identifier_tail) {
                    return std::make_unique<declarator_list_node>(std::move(identifier), std::move(*identifier_tail));
                }
            }
            return nullptr;
        }

        std::unique_ptr<parameter_declaration_node> parse_parameter_declaration() {
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

        std::unique_ptr<variable_declaration_node> parse_variable_declaration() {
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

        std::unique_ptr<constant_declaration_node> parse_constant_declaration() {
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

        std::unique_ptr<compound_statement_node> parse_compound_statement();

        std::unique_ptr<statement_list_node> parse_statement_list();

        std::unique_ptr<statement_node> parse_statement();

        std::unique_ptr<assignment_expression_node> parse_assignment();

        std::unique_ptr<additive_expression_node> parse_additive_expression();

        std::unique_ptr<multiplicative_expression_node> parse_multiplicative_expression();

        std::unique_ptr<unary_expression_node> parse_unary_expression();

        std::unique_ptr<primary_expression_node> parse_primary_expression;

        std::optional<test_function_defition_node> parse_function_definition() {
            /*return {
                parse_parameter_declaration(),
                parse_variable_declaration(),
                parse_constant_declaration(),
                parse_compund_statement(),
                parse_terminal_token(TokenType::PROGRAM_TERMINATOR)
            };*/
            // TODO Check lexer, should be at EOS
            return std::nullopt;
        }

        public:
        std::unique_ptr<test_function_defition_node> parse_test_function_definition() {
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
            if(auto program_terminator = parse_terminal_token(lexer::PROGRAM_TERMINATOR); program_terminator) {
                if(!peek_token().has_value() || peek_token()->Type() != lexer::EOS) {
                    // Tokens remaining - program must be syntactically invalid.
                    report_error("Error parsing function definition. Input after \".\"");
                    return nullptr;
                }
                return std::make_unique<test_function_defition_node>(
                    std::move(param_decl),
                    std::move(var_decl),
                    std::move(const_decl),
                    std::move(program_terminator));
            }
            return nullptr;
        }

    };

}

#endif //PLJIT_PARSER_HPP
