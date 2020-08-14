#include "dot_print_visitor.hpp"
#include "parse_tree_nodes.hpp"

using namespace pljit::parser;

unsigned dot_print_visitor::write_labeled_node(std::string_view label) {
    unsigned id = next_id++;
    if(id > 0) {
        out << "n_" << id << ";";
    }
    out << "n_" << id << " [label=\"" << label << "\"];\n";
    return id;
}

void dot_print_visitor::visit(const declarator_list_node& node) {
    write_non_terminal_node(node);
}

void dot_print_visitor::visit(const identifier_node& node) {
    write_non_terminal_node(node);
}

void dot_print_visitor::visit(const literal_node& node) {
    write_non_terminal_node(node);
}

void dot_print_visitor::visit(const terminal_node& node) {
    // TODO: Add additional_attributes string_view to write_labeled_node and print as rectangle.
    write_labeled_node(node.get_token().get_code_reference().str());
}

void dot_print_visitor::visit(const additive_expression_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const assignment_expression_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const compound_statement_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const constant_declaration_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const init_declarator_list_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const init_declarator_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const multiplicative_expression_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const parameter_declaration_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const primary_expression_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const statement_list_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const statement_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const function_defition_node& node) {
    out << "graph {\n";
    write_non_terminal_node(node);
    out << "}";
}
void dot_print_visitor::visit(const unary_expression_node& node) {
    write_non_terminal_node(node);
}
void dot_print_visitor::visit(const variable_declaration_node& node) {
    write_non_terminal_node(node);
}
dot_print_visitor::dot_print_visitor(std::ostream& out) : out(out) {}
