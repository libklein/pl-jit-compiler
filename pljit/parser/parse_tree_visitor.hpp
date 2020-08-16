
#ifndef PLJIT_PARSE_TREE_VISITOR_HPP
#define PLJIT_PARSE_TREE_VISITOR_HPP

namespace pljit::parser {
class non_terminal_node;

class terminal_node;
class additive_expression_node;
class assignment_expression_node;
class compound_statement_node;
class constant_declaration_node;
class function_definition_node;
class init_declarator_node;
class init_declarator_list_node;
class declarator_list_node;
class identifier_node;
class literal_node;
class multiplicative_expression_node;
class parameter_declaration_node;
class primary_expression_node;
class statement_list_node;
class statement_node;
class function_definition_node;
class unary_expression_node;
class variable_declaration_node;

class parse_tree_visitor {
    public:
    virtual void visit(const declarator_list_node&) = 0;
    virtual void visit(const identifier_node&) = 0;
    virtual void visit(const literal_node&) = 0;
    virtual void visit(const terminal_node&) = 0;
    virtual void visit(const additive_expression_node&) = 0;
    virtual void visit(const assignment_expression_node&) = 0;
    virtual void visit(const compound_statement_node&) = 0;
    virtual void visit(const constant_declaration_node&) = 0;
    virtual void visit(const init_declarator_list_node&) = 0;
    virtual void visit(const init_declarator_node&) = 0;
    virtual void visit(const multiplicative_expression_node&) = 0;
    virtual void visit(const parameter_declaration_node&) = 0;
    virtual void visit(const primary_expression_node&) = 0;
    virtual void visit(const statement_list_node&) = 0;
    virtual void visit(const statement_node&) = 0;
    virtual void visit(const function_definition_node&) = 0;
    virtual void visit(const unary_expression_node&) = 0;
    virtual void visit(const variable_declaration_node&) = 0;

    virtual ~parse_tree_visitor() = default;
};

} // namespace pljit::parser

#endif //PLJIT_PARSE_TREE_VISITOR_HPP
