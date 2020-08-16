
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
        explicit parser(lexer::lexer& lexer);

        bool has_error() const;
        private:
        const std::optional<Token>& peek_token() const;

        std::optional<Token> consume_token(TokenType expected_type);

        bool expect_token(TokenType expected_type) const;

        void report_error(std::string_view message, std::optional<source_management::SourceFragment>);

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

        std::unique_ptr<statement_node> parse_statement();

        std::unique_ptr<assignment_expression_node> parse_assignment();

        std::unique_ptr<additive_expression_node> parse_additive_expression();

        std::unique_ptr<multiplicative_expression_node> parse_multiplicative_expression();

        std::unique_ptr<unary_expression_node> parse_unary_expression();

        std::unique_ptr<primary_expression_node> parse_primary_expression();

        std::unique_ptr<statement_list_node> parse_statement_list();

        public:
        std::unique_ptr<function_definition_node> parse_function_definition();

    };

}

#endif //PLJIT_PARSER_HPP
