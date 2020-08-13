#include "parser.hpp"

using namespace pljit::parser;

std::unique_ptr<compound_statement_node> pljit::parser::parser::parse_compound_statement() {
    return std::unique_ptr<compound_statement_node>();
}
std::unique_ptr<statement_list_node> parser::parse_statement_list() {
    return std::unique_ptr<statement_list_node>();
}
std::unique_ptr<statement_node> parser::parse_statement() {
    return std::unique_ptr<statement_node>();
}
std::unique_ptr<assignment_expression_node> parser::parse_assignment() {
    return std::unique_ptr<assignment_expression_node>();
}
std::unique_ptr<additive_expression_node> parser::parse_additive_expression() {
    return std::unique_ptr<additive_expression_node>();
}
std::unique_ptr<multiplicative_expression_node> parser::parse_multiplicative_expression() {
    return std::unique_ptr<multiplicative_expression_node>();
}
std::unique_ptr<unary_expression_node> parser::parse_unary_expression() {
    return std::unique_ptr<unary_expression_node>();
}
