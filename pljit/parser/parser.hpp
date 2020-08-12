
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

namespace pljit::parser {
    struct parsing_error {
        std::string message;
        lexer::token token;

        friend std::ostream& operator<<(std::ostream& os, const parsing_error& error) {
            os << error.message << ": " << error.token.get_code_reference();
            return os;
        }
    };

    class parser {
        using Token = lexer::token;
        using TokenType = lexer::TokenType;
        private:
            lexer::lexer& lexer;
            Token next_token;
        public:
        explicit parser(lexer::lexer& lexer) : lexer(lexer), next_token(lexer.next()) {};
        private:
        Token& peek_token() {
            return next_token;
        };
        Token consume_token() {
            auto cur_token = next_token;
            next_token = lexer.next();
            return cur_token;
        };

        generic_terminal_node parse_terminal_token(TokenType expected_token_type) {
            auto token = consume_token();
            if(token.Type() != expected_token_type) {
                throw parsing_error{"Error parsing terminal symbol", token};
            }
            return generic_terminal_node(token);
        }

        template<class node_parser>
        // TODO is taking node_parser by value fine here?
        std::vector<std::pair<generic_terminal_node, std::result_of_t<node_parser()>>> parse_list_of(TokenType separator_token_type, node_parser child_parser) {
            std::vector<std::pair<generic_terminal_node, std::result_of_t<node_parser()>>> list;
            while(peek_token().Type() == separator_token_type) {
                auto separator_token = parse_terminal_token(separator_token_type);
                list.emplace_back(separator_token, child_parser());
            }
            return list;
        }

        identifier_node parse_identifier() {
            auto identifier_token = consume_token();
            if(identifier_token.Type() != TokenType::IDENTIFIER) {
                // TODO Throw error
                throw parsing_error{"Error parsing identifier", identifier_token};
            }
            return identifier_node(identifier_token);
        }

        declarator_list_node parse_declarator_list() {
            return declarator_list_node{
                parse_identifier(),
                parse_list_of(lexer::SEPARATOR, [this]() { return this->parse_identifier(); })
            };
        }

        std::optional<parameter_declaration_node> parse_parameter_declaration() {
            if(peek_token().Type() != TokenType::PARAM) return std::nullopt;
            return std::optional<parameter_declaration_node>{
                std::in_place,
                parse_terminal_token(TokenType::PARAM),
                parse_declarator_list(),
                parse_terminal_token(TokenType::STATEMENT_TERMINATOR)
            };
        }

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
        std::optional<test_function_defition_node> parse_test_function_definition() {
            /*return {
                parse_parameter_declaration(),
                parse_variable_declaration(),
                parse_constant_declaration(),
                parse_compund_statement(),
                parse_terminal_token(TokenType::PROGRAM_TERMINATOR)
            };*/

            return std::optional<test_function_defition_node>{
                std::in_place,
                parse_parameter_declaration(),
                parse_terminal_token(TokenType::PROGRAM_TERMINATOR)
            };
        }

    };

}

#endif //PLJIT_PARSER_HPP
