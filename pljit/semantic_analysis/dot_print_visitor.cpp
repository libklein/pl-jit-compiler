#include "dot_print_visitor.hpp"
#include "AST.hpp"

using namespace pljit::semantic_analysis;

void dot_print_visitor::visit(FunctionNode& node) {
    out << "graph {\n";
    auto id = write_labeled_node("Function");
    for(unsigned i = 0; i < node.get_number_of_statements(); ++i) {
        out << "n_" << id << " -- ";
        node.get_statement(i)->accept(*this);
    }
    out << "}";
}

void dot_print_visitor::visit(IdentifierNode&) {
    write_labeled_node("Identifier");
}

void dot_print_visitor::visit(LiteralNode& node) {
    write_labeled_node(std::to_string(node.get_value()));
}

void dot_print_visitor::visit(ReturnStatementNode& node) {
    auto id = write_labeled_node("Return Statement");
    print_child(node.get_expression(), id);
}

void dot_print_visitor::visit(AssignmentNode& node) {
    auto id = write_labeled_node("Assignment");
    print_child(node.get_identifier(), id);
    print_child(node.get_expression(), id);
}

void dot_print_visitor::visit(UnaryOperatorASTNode& node) {
    unsigned id;
    if(node.get_operator() == UnaryOperatorASTNode::OperatorType::PLUS) {
        id = write_labeled_node("+(child)");
    } else {
        id = write_labeled_node("-(child)");
    }
    print_child(node.getInput(), id);
}

void dot_print_visitor::visit(BinaryOperatorASTNode& node) {
    unsigned id;
    switch (node.get_operator()) {
        case BinaryOperatorASTNode::OperatorType::PLUS: {
            id = write_labeled_node("lhs + rhs");
            break;
        }
        case BinaryOperatorASTNode::OperatorType::MINUS: {
            id = write_labeled_node("lhs - rhs");
            break;
        }
        case BinaryOperatorASTNode::OperatorType::MULTIPLY: {
            id = write_labeled_node("lhs * rhs");
            break;
        }
        case BinaryOperatorASTNode::OperatorType::DIVIDE: {
            id = write_labeled_node("lhs / rhs");
            break;
        }
        default:
            break;
    }
    print_child(node.getLeft(), id);
    print_child(node.getRight(), id);
}

unsigned dot_print_visitor::write_labeled_node(std::string_view label) {
    unsigned id = next_id++;
    if(id > 0) {
        out << "n_" << id << ";";
    }
    out << "n_" << id << " [label=\"" << label << "\"];\n";
    return id;
}

void dot_print_visitor::print_child(ASTNode& node, unsigned int parent_id) {
    out << "n_" << parent_id << " -- ";
    node.accept(*this);
}

dot_print_visitor::dot_print_visitor(std::ostream& out) : out(out) {}
