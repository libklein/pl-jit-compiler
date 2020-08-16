#ifndef PLJIT_DOT_PRINT_VISITOR_HPP
#define PLJIT_DOT_PRINT_VISITOR_HPP

#include "ast_visitor.hpp"
#include <iostream>

namespace pljit::semantic_analysis {

class dot_print_visitor : public ast_visitor {
    std::ostream& out = std::cout;
    unsigned int next_id = 0;

    unsigned write_labeled_node(std::string_view label);
    void print_child(ASTNode& node, unsigned parent_id);

    public:
    dot_print_visitor() = default;
    explicit dot_print_visitor(std::ostream& out);

    void visit(FunctionNode& node) override;
    void visit(IdentifierNode& node) override;
    void visit(LiteralNode& node) override;
    void visit(ReturnStatementNode& node) override;
    void visit(AssignmentNode& node) override;
    void visit(UnaryOperatorASTNode& node) override;
    void visit(BinaryOperatorASTNode& node) override;
};

} // namespace pljit::semantic_analysis

#endif //PLJIT_DOT_PRINT_VISITOR_HPP
