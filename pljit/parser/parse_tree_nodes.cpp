
#include "parse_tree_nodes.hpp"
#include "parse_tree_visitor.hpp"

namespace pljit::parser {

void terminal_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void function_definition_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void constant_declaration_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}

void statement_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void init_declarator_list_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void init_declarator_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void unary_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void variable_declaration_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void statement_list_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void literal_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void primary_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void compound_statement_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void assignment_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void identifier_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void parameter_declaration_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void additive_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void multiplicative_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void declarator_list_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}

} // namespace pljit::parser