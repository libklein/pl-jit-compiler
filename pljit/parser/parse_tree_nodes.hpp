
#ifndef PLJIT_PARSE_TREE_NODES_HPP
#define PLJIT_PARSE_TREE_NODES_HPP

#include <pljit/lexer/token.hpp>
#include <utility>

namespace pljit::parser {

    /**
     * Requirements:
     * * Type deduction should be fast
     * * Contains reference
     */

    /**
     * Possible implementation:
     * parse_node_traits<>, include name etc
     * CRTP parse_tree_node, Name() function which uses trait class
     * macro which takes care of class definition
     *
     * TODO Delete copy constructor to make sure we are not making superfluous copies
     */
    struct parse_tree_node {};

    struct terminal_parse_tree_node : public parse_tree_node {
        lexer::token token;

        public:
        explicit terminal_parse_tree_node(lexer::token token) : token(token) {};
    };

    struct non_terminal_parse_tree_node : public parse_tree_node {};

    struct generic_terminal_node : public terminal_parse_tree_node {
        public:
        using terminal_parse_tree_node::terminal_parse_tree_node;
    };

    struct literal_node : public terminal_parse_tree_node {

    };

    struct identifier_node : public terminal_parse_tree_node {
        public:
        using terminal_parse_tree_node::terminal_parse_tree_node;
    };

    struct init_declarator_node : public non_terminal_parse_tree_node {
        identifier_node identifier;
        generic_terminal_node assignment_operator;
        literal_node literal;
        init_declarator_node(const identifier_node& identifier, const generic_terminal_node& assignmentOperator, const literal_node& literal) : identifier(identifier), assignment_operator(assignmentOperator), literal(literal) {}
    };

    struct declarator_list_node : public non_terminal_parse_tree_node {
        identifier_node identifier;
        std::vector<std::pair<generic_terminal_node, identifier_node>> other_identifiers;
        declarator_list_node(const identifier_node& identifier, std::vector<std::pair<generic_terminal_node, identifier_node>>  otherIdentifiers) : identifier(identifier), other_identifiers(std::move(otherIdentifiers)) {}
    };

    struct init_declarator_list_node : public non_terminal_parse_tree_node {
        identifier_node identifier;
        std::vector<std::pair<generic_terminal_node, init_declarator_node>> other_identifiers;
        init_declarator_list_node(const identifier_node& identifier, std::vector<std::pair<generic_terminal_node, init_declarator_node>>  otherIdentifiers) : identifier(identifier), other_identifiers(std::move(otherIdentifiers)) {}
    };

    struct parameter_declaration_node : public non_terminal_parse_tree_node {
        generic_terminal_node param_keyword;
        declarator_list_node declarators;
        generic_terminal_node statement_terminator;
        parameter_declaration_node(const generic_terminal_node& paramKeyword, declarator_list_node  declarators, const generic_terminal_node& statementTerminator) : param_keyword(paramKeyword), declarators(std::move(declarators)), statement_terminator(statementTerminator) {}
    };

    struct variable_declaration_node : public non_terminal_parse_tree_node {
        generic_terminal_node var_keyword;
        declarator_list_node declarators;
        generic_terminal_node statement_terminator;
        variable_declaration_node(const generic_terminal_node& varKeyword, declarator_list_node  declarators, const generic_terminal_node& statementTerminator) : var_keyword(varKeyword), declarators(std::move(declarators)), statement_terminator(statementTerminator) {}
    };

    struct constant_declaration_node : public non_terminal_parse_tree_node {
        generic_terminal_node const_keyword;
        init_declarator_list_node declarators;
        generic_terminal_node statement_terminator;
        constant_declaration_node(const generic_terminal_node& constKeyword, init_declarator_list_node  declarators, const generic_terminal_node& statementTerminator) : const_keyword(constKeyword), declarators(std::move(declarators)), statement_terminator(statementTerminator) {}
    };

    struct compund_statement_node : public non_terminal_parse_tree_node {

    };

    struct function_defition_node : public non_terminal_parse_tree_node {
        std::optional<parameter_declaration_node> parameter_declarations;
        std::optional<variable_declaration_node> variable_declaration;
        std::optional<constant_declaration_node> constant_declarations;
        compund_statement_node compund_statement;
        generic_terminal_node program_terminator;
        function_defition_node(std::optional<parameter_declaration_node>  parameterDeclarations, std::optional<variable_declaration_node>  variableDeclaration, std::optional<constant_declaration_node>  constantDeclarations, const compund_statement_node& compundStatement, const generic_terminal_node& programTerminator) : parameter_declarations(std::move(parameterDeclarations)), variable_declaration(std::move(variableDeclaration)), constant_declarations(std::move(constantDeclarations)), compund_statement(compundStatement), program_terminator(programTerminator) {}
    };

    struct test_function_defition_node : public non_terminal_parse_tree_node {
        std::optional<parameter_declaration_node> parameter_declarations;
        generic_terminal_node program_terminator;

        test_function_defition_node(std::optional<parameter_declaration_node>  parameterDeclarations, const generic_terminal_node& programTerminator) : parameter_declarations(std::move(parameterDeclarations)), program_terminator(programTerminator) {}
    };
} // namespace pljit::parser

#endif //PLJIT_PARSE_TREE_NODES_HPP
