
#include "parse_tree_nodes.hpp"
#include "parse_tree_visitor.hpp"

namespace pljit::parser {

void terminal_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
const lexer::token& terminal_node::get_token() const {
    return token;
}
std::string_view terminal_node::get_text() const {
    return token.get_code_reference().str();
}
terminal_node::terminal_node(lexer::token token) : node_base(TEXT_NODE, token.get_code_reference()), token(token) {}
void function_definition_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
function_definition_node::function_definition_node(source_management::SourceFragment fragment, std::unique_ptr<parameter_declaration_node> parameterDeclarations, std::unique_ptr<variable_declaration_node> variableDeclaration, std::unique_ptr<constant_declaration_node> constantDeclarations, std::unique_ptr<compound_statement_node> compundStatement, std::unique_ptr<terminal_node> programTerminator)
    : non_terminal_node(FUNCTION_DEFINITION, fragment, std::move(compundStatement), std::move(programTerminator)) {
    if (constantDeclarations) {
        has_constant_declaration = true;
        this->insert_child(children.begin(), std::move(constantDeclarations));
    }
    if (variableDeclaration) {
        has_variable_declaration = true;
        this->insert_child(children.begin(), std::move(variableDeclaration));
    }
    if (parameterDeclarations) {
        has_parameter_declaration = true;
        this->insert_child(children.begin(), std::move(parameterDeclarations));
    }
}
const compound_statement_node* function_definition_node::get_compund_statement() const {
    unsigned compound_statement_index = has_constant_declaration + has_variable_declaration + has_parameter_declaration;
    return get_child<compound_statement_node>(compound_statement_index);
}
const constant_declaration_node* function_definition_node::get_constant_declarations() const {
    return has_constant_declaration ?
        get_child<constant_declaration_node>(has_variable_declaration + has_parameter_declaration) :
        nullptr;
}
const variable_declaration_node* function_definition_node::get_variable_declarations() const {
    return has_variable_declaration ? get_child<variable_declaration_node>(has_parameter_declaration) : nullptr;
}
const parameter_declaration_node* function_definition_node::get_parameter_declarations() const {
    return has_parameter_declaration ? get_child<parameter_declaration_node>(0) : nullptr;
}
void constant_declaration_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
constant_declaration_node::constant_declaration_node(source_management::SourceFragment fragment, std::unique_ptr<terminal_node> const_keyword, std::unique_ptr<init_declarator_list_node> declarators, std::unique_ptr<terminal_node> statement_terminator)
    : non_terminal_node(CONSTANT_DECLARATIONS, fragment, std::move(const_keyword), std::move(declarators), std::move(statement_terminator)) {}
const init_declarator_list_node* constant_declaration_node::get_init_declarator_list() const {
    return get_child<init_declarator_list_node>(1);
}

void statement_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
statement_node::statement_node(source_management::SourceFragment fragment, std::unique_ptr<assignment_expression_node> assignment) : non_terminal_node(STATEMENT, fragment, std::move(assignment)), statement_type(statement_type::ASSIGNMENT_STATEMENT) {}
const assignment_expression_node* statement_node::get_assignment() const {
    assert(statement_type == statement_type::ASSIGNMENT_STATEMENT);
    return get_child<assignment_expression_node>(0);
}
const additive_expression_node* statement_node::get_return_expression() const {
    assert(statement_type == statement_type::RETURN_STATEMENT);
    return get_child<additive_expression_node>(1);
}
void init_declarator_list_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
init_declarator_list_node::init_declarator_list_node(source_management::SourceFragment fragment, std::unique_ptr<init_declarator_node> identifier, std::vector<std::pair<std::unique_ptr<terminal_node>, std::unique_ptr<init_declarator_node>>> otherIdentifiers) : non_terminal_node(INIT_DECLARATOR_LIST, fragment, std::move(identifier)) {
    for (auto& entry : otherIdentifiers) {
        add_child(std::move(entry.first), std::move(entry.second));
    }
}
auto init_declarator_list_node::get_number_of_declarations() const -> size_type {
    assert(get_number_of_dynamic_children() % 2 == 0);
    return 1 + (get_number_of_dynamic_children() / 2);
}
const init_declarator_node* init_declarator_list_node::get_declaration(unsigned long index) const {
    return get_child<init_declarator_node>(index * 2);
}
void init_declarator_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
init_declarator_node::init_declarator_node(source_management::SourceFragment fragment, std::unique_ptr<identifier_node> identifier, std::unique_ptr<terminal_node> assignmentOperator, std::unique_ptr<literal_node> literal)
    : non_terminal_node(INIT_DECLARATOR, fragment,
                        std::move(identifier), std::move(assignmentOperator), std::move(literal)) {}
const identifier_node* init_declarator_node::get_identifier() const {
    return get_child<identifier_node>(0);
}
const literal_node* init_declarator_node::get_value() const {
    return get_child<literal_node>(2);
}
void unary_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
unary_expression_node::unary_expression_node(source_management::SourceFragment fragment, std::unique_ptr<terminal_node> unary_operator, std::unique_ptr<primary_expression_node> primary_expression)
    : non_terminal_node(UNARY_EXPRESSION, fragment) {
    has_unary_operator = this->add_child(std::move(unary_operator));
    this->add_child(std::move(primary_expression));
    if (has_unary_operator && get_unary_operator()->get_token().Type() == lexer::MINUS_OP) {
        op = OperationType::MINUS;
    }
}
unary_expression_node::OperationType unary_expression_node::get_operation() const {
    return op;
}
const terminal_node* unary_expression_node::get_unary_operator() const {
    return get_child<terminal_node>(0);
}
const primary_expression_node* unary_expression_node::get_expression() const {
    return get_child<primary_expression_node>(has_unary_operator);
}
void variable_declaration_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
variable_declaration_node::variable_declaration_node(source_management::SourceFragment fragment, std::unique_ptr<terminal_node> variable_keyword, std::unique_ptr<declarator_list_node> declarators, std::unique_ptr<terminal_node> statement_terminator)
    : non_terminal_node(VARIABLE_DECLARATIONS, fragment, std::move(variable_keyword), std::move(declarators), std::move(statement_terminator)) {}
const declarator_list_node* variable_declaration_node::get_declarator_list() const {
    return get_child<declarator_list_node>(1);
}
void statement_list_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
statement_list_node::statement_list_node(source_management::SourceFragment fragment, std::unique_ptr<statement_node> statement, std::vector<std::pair<std::unique_ptr<terminal_node>, std::unique_ptr<statement_node>>> additional_statements)
    : non_terminal_node(STATEMENT_LIST, fragment, std::move(statement)) {
    for (auto& [separator, next_statement] : additional_statements) {
        add_child(std::move(separator), std::move(next_statement));
    }
}
auto statement_list_node::get_number_of_statements() const -> size_type {
    assert(get_number_of_dynamic_children() % 2 == 0);
    return 1 + (get_number_of_dynamic_children() / 2);
}
const statement_node* statement_list_node::get_statement(unsigned long index) const {
    return get_child<statement_node>(index * 2);
}
void literal_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
literal_node::literal_node(node_ptr token, source_management::SourceFragment source) : non_terminal_node(LITERAL, source, std::move(token)) {}
lexer::token literal_node::get_token() const {
    return get_child<terminal_node>(0)->get_token();
}
int64_t literal_node::get_value() const {
    // We can assume that no overflows happen
    return std::strtoll(get_token().get_code_reference().str().data(), nullptr, 10);
}
void primary_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
primary_expression_node::primary_expression_node(source_management::SourceFragment fragment, std::unique_ptr<terminal_node> l_bracket, std::unique_ptr<additive_expression_node> additive_expression, std::unique_ptr<terminal_node> r_bracket)
    : non_terminal_node(PRIMARY_EXPRESSION, fragment, std::move(l_bracket), std::move(additive_expression), std::move(r_bracket)),
      statement_type(primary_expression_type::ADDITIVE_EXPRESSION) {}
primary_expression_node::primary_expression_node(source_management::SourceFragment fragment, std::unique_ptr<literal_node> literal)
    : non_terminal_node(PRIMARY_EXPRESSION, fragment, std::move(literal)),
      statement_type(primary_expression_type::LITERAL) {}
primary_expression_node::primary_expression_node(source_management::SourceFragment fragment, std::unique_ptr<identifier_node> identifier)
    : non_terminal_node(PRIMARY_EXPRESSION, fragment, std::move(identifier)),
      statement_type(primary_expression_type::IDENTIFIER) {}
const identifier_node* primary_expression_node::get_identifier() const {
    assert(statement_type == primary_expression_type::IDENTIFIER);
    return get_child<identifier_node>(0);
}
const literal_node* primary_expression_node::get_literal() const {
    assert(statement_type == primary_expression_type::LITERAL);
    return get_child<literal_node>(0);
}
const additive_expression_node* primary_expression_node::get_expression() const {
    assert(statement_type == primary_expression_type::ADDITIVE_EXPRESSION);
    return get_child<additive_expression_node>(1);
}
void compound_statement_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
compound_statement_node::compound_statement_node(source_management::SourceFragment fragment, std::unique_ptr<terminal_node> begin_keyword, std::unique_ptr<statement_list_node> statements, std::unique_ptr<terminal_node> end_keyword)
    : non_terminal_node(COMPOUND_STATEMENT, fragment, std::move(begin_keyword), std::move(statements), std::move(end_keyword)) {}
const statement_list_node* compound_statement_node::get_statement_list() const {
    return get_child<statement_list_node>(1);
}
void assignment_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
assignment_expression_node::assignment_expression_node(source_management::SourceFragment fragment, std::unique_ptr<identifier_node> identifier, std::unique_ptr<terminal_node> assignment_operator, std::unique_ptr<additive_expression_node> additive_expression)
    : non_terminal_node(ASSIGNMENT_EXPRESSION, fragment, std::move(identifier), std::move(assignment_operator), std::move(additive_expression)) {}
const identifier_node* assignment_expression_node::get_identifier() const {
    return get_child<identifier_node>(0);
}
const additive_expression_node* assignment_expression_node::get_expression() const {
    return get_child<additive_expression_node>(2);
}
void identifier_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
identifier_node::identifier_node(node_ptr token, source_management::SourceFragment source) : non_terminal_node(IDENTIFIER, source, std::move(token)) {}
lexer::token identifier_node::get_token() const {
    return get_child<terminal_node>(0)->get_token();
}
std::string_view identifier_node::get_name() const {
    return get_token().get_code_reference().str();
}
void parameter_declaration_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
parameter_declaration_node::parameter_declaration_node(source_management::SourceFragment fragment, std::unique_ptr<terminal_node> paramKeyword, std::unique_ptr<declarator_list_node> declarators, std::unique_ptr<terminal_node> statementTerminator)
    : non_terminal_node(PARAMETER_DECLARATIONS, fragment, std::move(paramKeyword), std::move(declarators), std::move(statementTerminator)) {}
const declarator_list_node* parameter_declaration_node::get_declarator_list() const {
    return get_child<declarator_list_node>(1);
}
void additive_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
additive_expression_node::additive_expression_node(source_management::SourceFragment fragment, std::unique_ptr<multiplicative_expression_node> multiplicative_expression, std::unique_ptr<terminal_node> binary_operator, std::unique_ptr<additive_expression_node> additive_expression)
    : non_terminal_node(ADDITIVE_EXPRESSION, fragment, std::move(multiplicative_expression)) {
    if (binary_operator && additive_expression) {
        op = binary_operator->get_token().Type() == lexer::PLUS_OP ? OperationType::PLUS : OperationType::MINUS;
        has_subexpression = this->add_child(std::move(binary_operator), std::move(additive_expression));
    }
}
additive_expression_node::OperationType additive_expression_node::get_operation() const {
    assert(has_subexpression);
    return op;
}
const multiplicative_expression_node* additive_expression_node::get_lhs_expression() const {
    return get_child<multiplicative_expression_node>(0);
}
const terminal_node* additive_expression_node::get_operator() const {
    return has_subexpression ? get_child<terminal_node>(1) : nullptr;
}
const additive_expression_node* additive_expression_node::get_rhs_expression() const {
    return has_subexpression ? get_child<additive_expression_node>(2) : nullptr;
}
void multiplicative_expression_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
multiplicative_expression_node::multiplicative_expression_node(source_management::SourceFragment fragment, std::unique_ptr<unary_expression_node> unary_expression, std::unique_ptr<terminal_node> binary_operator, std::unique_ptr<multiplicative_expression_node> multiplicative_expression)
    : non_terminal_node(MULTIPLICATIVE_EXPRESSION, fragment, std::move(unary_expression)) {
    if (binary_operator && multiplicative_expression) {
        op = binary_operator->get_token().Type() == lexer::MULT_OP ? OperationType::MULTIPLY : OperationType::DIVIDE;
        has_subexpression = this->add_child(std::move(binary_operator), std::move(multiplicative_expression));
    }
}
const unary_expression_node* multiplicative_expression_node::get_lhs_expression() const {
    return get_child<unary_expression_node>(0);
}
const terminal_node* multiplicative_expression_node::get_operator() const {
    return get_child<terminal_node>(1);
}
multiplicative_expression_node::OperationType multiplicative_expression_node::get_operation() const {
    assert(has_subexpression);
    return op;
}
const multiplicative_expression_node* multiplicative_expression_node::get_rhs_expression() const {
    return get_child<multiplicative_expression_node>(2);
}
void declarator_list_node::accept(parse_tree_visitor& visitor) const {
    visitor.visit(*this);
}
declarator_list_node::declarator_list_node(source_management::SourceFragment fragment, std::unique_ptr<identifier_node> identifier, std::vector<std::pair<std::unique_ptr<terminal_node>, std::unique_ptr<identifier_node>>> otherIdentifiers) : non_terminal_node(DECLARATOR_LIST, fragment, std::move(identifier)) {
    for (auto& entry : otherIdentifiers) {
        add_child(std::move(entry.first), std::move(entry.second));
    }
}
auto declarator_list_node::get_number_of_declarations() const -> size_type {
    assert(get_number_of_dynamic_children() % 2 == 0);
    return 1 + (get_number_of_dynamic_children() / 2);
}
const identifier_node* declarator_list_node::get_declaration(unsigned long index) const {
    return get_child<identifier_node>(index * 2);
}

node_base::node_base(grammar_type type, source_management::SourceFragment source) : type(type), codeReference(source) {}
grammar_type node_base::get_type() const {
    return type;
}
source_management::SourceFragment node_base::getCodeReference() const {
    return codeReference;
}
auto non_terminal_node::insert_child(node_ptr_container_type::iterator position, std::unique_ptr<node_base> child) -> node_ptr_container_type::iterator {
    ++dynamic_child_count;
    return children.insert(position, std::move(child));
}
auto non_terminal_node::get_number_of_dynamic_children() const -> node_ptr_container_type::size_type {
    return dynamic_child_count;
}
} // namespace pljit::parser