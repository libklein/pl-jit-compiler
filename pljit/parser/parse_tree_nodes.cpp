
#include "parse_tree_nodes.hpp"
#include "parse_tree_visitor.hpp"

void pljit::parser::terminal_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::function_defition_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::constant_declaration_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}

void pljit::parser::statement_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::init_declarator_list_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::init_declarator_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::unary_expression_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::variable_declaration_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::statement_list_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::literal_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::primary_expression_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::compound_statement_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::assignment_expression_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::identifier_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::parameter_declaration_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::additive_expression_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::multiplicative_expression_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
void pljit::parser::declarator_list_node::accept(pljit::parser::parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
