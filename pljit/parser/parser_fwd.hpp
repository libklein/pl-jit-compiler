
#ifndef PLJIT_PARSER_FWD_HPP
#define PLJIT_PARSER_FWD_HPP

namespace pljit::parser {
enum grammar_type {
    FUNCTION_DEFINITION,
    PARAMETER_DECLARATIONS,
    VARIABLE_DECLARATIONS,
    CONSTANT_DECLARATIONS,
    DECLARATOR_LIST,
    INIT_DECLARATOR_LIST,
    INIT_DECLARATOR,
    COMPOUND_STATEMENT,
    STATEMENT_LIST,
    STATEMENT,
    ASSIGNMENT_EXPRESSION,
    MULTIPLICATIVE_EXPRESSION,
    ADDITIVE_EXPRESSION,
    UNARY_EXPRESSION,
    PRIMARY_EXPRESSION,

    LITERAL,
    IDENTIFIER,
    TEXT_NODE
};

class parser;
class parse_tree_visitor;
struct terminal_node;
struct literal_node;
struct identifier_node;
struct init_declarator_node;
struct declarator_list_node;
struct init_declarator_list_node;
struct parameter_declaration_node;
struct variable_declaration_node;
struct constant_declaration_node;
struct multiplicative_expression_node;
struct additive_expression_node;
struct primary_expression_node;
struct unary_expression_node;
struct assignment_expression_node;
struct statement_node;
struct statement_list_node;
struct compound_statement_node;
struct function_definition_node;
} // namespace pljit::parser

#endif //PLJIT_PARSER_FWD_HPP
