
#ifndef PLJIT_DOT_PRINT_VISITOR_HPP
#define PLJIT_DOT_PRINT_VISITOR_HPP

#include "parse_tree_visitor.hpp"
#include <iostream>

namespace pljit::parser {

    class dot_print_visitor : public parse_tree_visitor {
        std::ostream& out = std::cout;
        unsigned next_id = 0;

        unsigned write_labeled_node(std::string_view label);

        template <class node_type>
        void write_non_terminal_node(const node_type& node) {
            static_assert(std::is_base_of_v<non_terminal_node, node_type>, "Can only write non-terminal nodes");
            unsigned id = write_labeled_node(node_type::NAME);
            for(const auto& child : node.get_children()) {
                out << "n_" << id << " -- ";
                child->accept(*this);
            }
        }
        public:
        void visit(const declarator_list_node& node) override;
        void visit(const identifier_node& node) override;
        void visit(const literal_node& node) override;
        void visit(const terminal_node& node) override;
        void visit(const additive_expression_node& node) override;
        void visit(const assignment_expression_node& node) override;
        void visit(const compound_statement_node& node) override;
        void visit(const constant_declaration_node& node) override;
        void visit(const function_defition_node& node) override;
        void visit(const init_declarator_list_node& node) override;
        void visit(const init_declarator_node& node) override;
        void visit(const multiplicative_expression_node& node) override;
        void visit(const parameter_declaration_node& node) override;
        void visit(const primary_expression_node& node) override;
        void visit(const statement_list_node& node) override;
        void visit(const statement_node& node) override;
        void visit(const test_function_defition_node& node) override;
        void visit(const unary_expression_node& node) override;
        void visit(const variable_declaration_node& node) override;
    };

} // namespace pljit::parser

#endif //PLJIT_DOT_PRINT_VISITOR_HPP
