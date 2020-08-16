
#ifndef PLJIT_PARSE_TREE_NODES_HPP
#define PLJIT_PARSE_TREE_NODES_HPP

#include "parser_fwd.hpp"
#include <pljit/lexer/token.hpp>
#include <utility>

namespace pljit::parser {

class parse_tree_visitor;

class node_base {
    protected:
    grammar_type type;
    explicit node_base(grammar_type type) : type(type){};

    public:
    grammar_type get_type() const {
        return type;
    };

    virtual void accept(parse_tree_visitor& visitor) const = 0;

    virtual ~node_base() = default;
};

using node_ptr = std::unique_ptr<node_base>;

class non_terminal_node : public node_base {
    protected:
    using node_ptr_container_type = std::vector<node_ptr>;

    node_ptr_container_type children;
    node_ptr_container_type::size_type dynamic_child_count = 0;

    using node_base::node_base;

    template <class... Args>
    explicit non_terminal_node(grammar_type type, Args... args) : node_base(type), children(0) {
        static_assert(std::conjunction_v<std::is_convertible<Args, node_ptr>...>,
                      "Non-terminal parse tree node cannot contain non-nodes");
        assert((args && ...));
        children.reserve(sizeof...(args));
        (children.emplace_back(std::move(args)), ...);
    }

    template <class... Args>
    bool add_child(Args... args) {
        if (!(args && ...)) {
            return false;
        }
        dynamic_child_count += sizeof...(args);
        (children.emplace_back(std::move(args)), ...);
        return true;
    }

    auto insert_child(decltype(children)::iterator position, std::unique_ptr<node_base> child) {
        ++dynamic_child_count;
        return children.insert(position, std::move(child));
    }

    public:
    auto get_number_of_dynamic_children() const {
        return dynamic_child_count;
    }

    const node_ptr_container_type& get_children() const {
        return children;
    }

    template <class child_type>
    child_type* get_child(node_ptr_container_type::size_type index) const {
        return static_cast<child_type*>(children[index].get());
    }
};

struct terminal_node : public node_base {
    lexer::token token;

    public:
    static constexpr std::string_view NAME = "terminal";

    explicit terminal_node(lexer::token token) : node_base(TEXT_NODE), token(token) {}

    void accept(parse_tree_visitor& visitor) const override;

    const lexer::token& get_token() const {
        return token;
    }

    std::string_view get_text() const {
        return token.get_code_reference().str();
    }
};

struct literal_node : public non_terminal_node {
    public:
    static constexpr std::string_view NAME = "literal";

    explicit literal_node(node_ptr token) : non_terminal_node(LITERAL, std::move(token)) {}
    void accept(parse_tree_visitor& visitor) const override;

    lexer::token get_token() const {
        return get_child<terminal_node>(0)->get_token();
    }

    int64_t get_value() const {
        // We can assume that no overflows happen
        return std::strtoll(get_token().get_code_reference().str().data(), nullptr, 10);
    };
};

struct identifier_node : public non_terminal_node {
    public:
    static constexpr std::string_view NAME = "identifier";

    explicit identifier_node(node_ptr token) : non_terminal_node(IDENTIFIER, std::move(token)) {}
    void accept(parse_tree_visitor& visitor) const override;

    lexer::token get_token() const {
        return get_child<terminal_node>(0)->get_token();
    }

    std::string_view get_name() const {
        return get_token().get_code_reference().str();
    };
};

struct init_declarator_node : public non_terminal_node {
    static constexpr std::string_view NAME = "init-declarator";

    init_declarator_node(std::unique_ptr<identifier_node> identifier,
                         std::unique_ptr<terminal_node>
                             assignmentOperator,
                         std::unique_ptr<literal_node>
                             literal)
        : non_terminal_node(INIT_DECLARATOR,
                            std::move(identifier), std::move(assignmentOperator), std::move(literal)) {}

    void accept(parse_tree_visitor& visitor) const override;

    const identifier_node* get_identifier() const {
        return get_child<identifier_node>(0);
    }

    const literal_node* get_value() const {
        return get_child<literal_node>(2);
    }
};

struct declarator_list_node : public non_terminal_node {
    static constexpr std::string_view NAME = "declarator-list";

    declarator_list_node(std::unique_ptr<identifier_node> identifier,
                         std::vector<std::pair<
                             std::unique_ptr<terminal_node>,
                             std::unique_ptr<identifier_node>>>
                             otherIdentifiers) : non_terminal_node(DECLARATOR_LIST, std::move(identifier)) {
        for (auto& entry : otherIdentifiers) {
            add_child(std::move(entry.first), std::move(entry.second));
        }
    }

    auto get_number_of_declarations() const {
        assert(get_number_of_dynamic_children() % 2 == 0);
        return 1 + (get_number_of_dynamic_children() / 2);
    }

    const identifier_node* get_declaration(decltype(children)::size_type index) const {
        return get_child<identifier_node>(index * 2);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

// TODO Common base for list nodes
struct init_declarator_list_node : public non_terminal_node {
    static constexpr std::string_view NAME = "init-declarator-list";

    // TODO Remove type-safe constructors (sigh)
    init_declarator_list_node(std::unique_ptr<init_declarator_node> identifier,
                              std::vector<std::pair<
                                  std::unique_ptr<terminal_node>,
                                  std::unique_ptr<init_declarator_node>>>
                                  otherIdentifiers) : non_terminal_node(INIT_DECLARATOR_LIST, std::move(identifier)) {
        for (auto& entry : otherIdentifiers) {
            add_child(std::move(entry.first), std::move(entry.second));
        }
    }

    auto get_number_of_declarations() const {
        assert(get_number_of_dynamic_children() % 2 == 0);
        return 1 + (get_number_of_dynamic_children() / 2);
    }

    const init_declarator_node* get_declaration(decltype(children)::size_type index) const {
        return get_child<init_declarator_node>(index * 2);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct parameter_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "parameter-declaration";

    // TODO Superclass
    parameter_declaration_node(std::unique_ptr<terminal_node> paramKeyword,
                               std::unique_ptr<declarator_list_node>
                                   declarators,
                               std::unique_ptr<terminal_node>
                                   statementTerminator)
        : non_terminal_node(PARAMETER_DECLARATIONS, std::move(paramKeyword), std::move(declarators), std::move(statementTerminator)) {}

    const declarator_list_node* get_declarator_list() const {
        return get_child<declarator_list_node>(1);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct variable_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "variable-declaration";

    variable_declaration_node(std::unique_ptr<terminal_node> variable_keyword,
                              std::unique_ptr<declarator_list_node>
                                  declarators,
                              std::unique_ptr<terminal_node>
                                  statement_terminator)
        : non_terminal_node(VARIABLE_DECLARATIONS, std::move(variable_keyword), std::move(declarators), std::move(statement_terminator)) {}

    const declarator_list_node* get_declarator_list() const {
        return get_child<declarator_list_node>(1);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct constant_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "constant-declaration";

    constant_declaration_node(std::unique_ptr<terminal_node> const_keyword,
                              std::unique_ptr<init_declarator_list_node>
                                  declarators,
                              std::unique_ptr<terminal_node>
                                  statement_terminator)
        : non_terminal_node(CONSTANT_DECLARATIONS, std::move(const_keyword), std::move(declarators), std::move(statement_terminator)) {}

    const init_declarator_list_node* get_init_declarator_list() const {
        return get_child<init_declarator_list_node>(1);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct additive_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "additive-expression";
    enum class OperationType {
        PLUS, MINUS
    };

    OperationType op;
    bool has_subexpression = false;
    additive_expression_node(std::unique_ptr<multiplicative_expression_node> multiplicative_expression,
                             std::unique_ptr<terminal_node>
                                 binary_operator,
                             std::unique_ptr<additive_expression_node>
                                 additive_expression)
        : non_terminal_node(ADDITIVE_EXPRESSION, std::move(multiplicative_expression)) {
        if (binary_operator && additive_expression) {
            op = binary_operator->get_token().Type() == lexer::PLUS_OP ? OperationType::PLUS : OperationType::MINUS;
            has_subexpression = this->add_child(std::move(binary_operator), std::move(additive_expression));
        }
    }

    OperationType get_operation() const {
        assert(has_subexpression);
        return op;
    }

    void accept(parse_tree_visitor& visitor) const override;

    const multiplicative_expression_node* get_lhs_expression() const {
        return get_child<multiplicative_expression_node>(0);
    }

    const terminal_node* get_operator() const {
        return has_subexpression ? get_child<terminal_node>(1) : nullptr;
    }

    const additive_expression_node* get_rhs_expression() const {
        return has_subexpression ? get_child<additive_expression_node>(2) : nullptr;
    }
};

struct primary_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "primary-expression";

    enum class primary_expression_type {
        IDENTIFIER,
        LITERAL,
        ADDITIVE_EXPRESSION
    };

    primary_expression_type statement_type;

    explicit primary_expression_node(std::unique_ptr<identifier_node> identifier)
        : non_terminal_node(PRIMARY_EXPRESSION, std::move(identifier)),
          statement_type(primary_expression_type::IDENTIFIER) {}

    explicit primary_expression_node(std::unique_ptr<literal_node> literal)
        : non_terminal_node(PRIMARY_EXPRESSION, std::move(literal)),
          statement_type(primary_expression_type::LITERAL) {}

    explicit primary_expression_node(std::unique_ptr<terminal_node> l_bracket,
                                     std::unique_ptr<additive_expression_node>
                                         additive_expression,
                                     std::unique_ptr<terminal_node>
                                         r_bracket)
        : non_terminal_node(PRIMARY_EXPRESSION, std::move(l_bracket), std::move(additive_expression), std::move(r_bracket)),
          statement_type(primary_expression_type::ADDITIVE_EXPRESSION) {}

    const identifier_node* get_identifier() const {
        assert(statement_type == primary_expression_type::IDENTIFIER);
        return get_child<identifier_node>(0);
    }

    const literal_node* get_literal() const {
        assert(statement_type == primary_expression_type::LITERAL);
        return get_child<literal_node>(0);
    }

    const additive_expression_node* get_expression() const {
        assert(statement_type == primary_expression_type::ADDITIVE_EXPRESSION);
        return get_child<additive_expression_node>(1);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct unary_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "unary-expression";
    enum class OperationType {
        PLUS, MINUS
    };

    OperationType op = OperationType::PLUS;
    bool has_unary_operator = false;
    unary_expression_node(std::unique_ptr<terminal_node> unary_operator, std::unique_ptr<primary_expression_node> primary_expression)
        : non_terminal_node(UNARY_EXPRESSION) {
        has_unary_operator = this->add_child(std::move(unary_operator));
        this->add_child(std::move(primary_expression));
        if(has_unary_operator && get_unary_operator()->get_token().Type() == lexer::MINUS_OP) {
            op = OperationType::MINUS;
        }
    }

    void accept(parse_tree_visitor& visitor) const override;

    OperationType get_operation() const {
        return op;
    }

    const terminal_node* get_unary_operator() const {
        return get_child<terminal_node>(0);
    }

    const primary_expression_node* get_expression() const {
        return get_child<primary_expression_node>(has_unary_operator);
    }
};

struct multiplicative_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "multiplicative-expression";
    enum class OperationType {
        MULTIPLY, DIVIDE
    };

    OperationType op;

    // TODO Expression base class
    bool has_subexpression = false;
    multiplicative_expression_node(std::unique_ptr<unary_expression_node> unary_expression,
                                   std::unique_ptr<terminal_node>
                                       binary_operator,
                                   std::unique_ptr<multiplicative_expression_node>
                                       multiplicative_expression)
        : non_terminal_node(MULTIPLICATIVE_EXPRESSION, std::move(unary_expression)) {
        if (binary_operator && multiplicative_expression) {
            op = binary_operator->get_token().Type() == lexer::MULT_OP ? OperationType::MULTIPLY : OperationType::DIVIDE;
            has_subexpression = this->add_child(std::move(binary_operator), std::move(multiplicative_expression));
        }
    }

    const unary_expression_node* get_lhs_expression() const {
        return get_child<unary_expression_node>(0);
    }

    const terminal_node* get_operator() const {
        return get_child<terminal_node>(1);
    }

    OperationType get_operation() const {
        assert(has_subexpression);
        return op;
    }

    const multiplicative_expression_node* get_rhs_expression() const {
        return get_child<multiplicative_expression_node>(2);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct assignment_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "assignment-expression";

    assignment_expression_node(std::unique_ptr<identifier_node> identifier, std::unique_ptr<terminal_node> assignment_operator, std::unique_ptr<additive_expression_node> additive_expression)
        : non_terminal_node(ASSIGNMENT_EXPRESSION
                            , std::move(identifier)
                                , std::move(assignment_operator)
                                , std::move(additive_expression)) {}

    void accept(parse_tree_visitor& visitor) const override;

    const identifier_node* get_identifier() const {
        return get_child<identifier_node>(0);
    }

    const additive_expression_node* get_expression() const {
        return get_child<additive_expression_node>(2);
    }
};

struct statement_node : public non_terminal_node {
    static constexpr std::string_view NAME = "statement";

    // Use scoped enum to avoid confusion with node type
    enum class statement_type {
        RETURN_STATEMENT,
        ASSIGNMENT_STATEMENT
    };

    statement_type statement_type;
    explicit statement_node(std::unique_ptr<assignment_expression_node> assignment) : non_terminal_node(STATEMENT, std::move(assignment)), statement_type(statement_type::ASSIGNMENT_STATEMENT) {}
    statement_node(std::unique_ptr<terminal_node> return_keyword, std::unique_ptr<additive_expression_node> additive_expression) : non_terminal_node(STATEMENT, std::move(return_keyword), std::move(additive_expression)), statement_type(statement_type::RETURN_STATEMENT) {}

    const assignment_expression_node* get_assignment() const {
        assert(statement_type == statement_type::ASSIGNMENT_STATEMENT);
        return get_child<assignment_expression_node>(0);
    }

    const additive_expression_node* get_return_expression() const {
        assert(statement_type == statement_type::RETURN_STATEMENT);
        return get_child<additive_expression_node>(1);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct statement_list_node : public non_terminal_node {
    static constexpr std::string_view NAME = "statement-list";

    statement_list_node(std::unique_ptr<statement_node> statement,
                        std::vector<std::pair<std::unique_ptr<terminal_node>, std::unique_ptr<statement_node>>>
                            additional_statements)
        : non_terminal_node(STATEMENT_LIST, std::move(statement)) {
        for (auto& [separator, next_statement] : additional_statements) {
            add_child(std::move(separator), std::move(next_statement));
        }
    }

    auto get_number_of_statements() const {
        assert(get_number_of_dynamic_children() % 2 == 0);
        return 1 + (get_number_of_dynamic_children() / 2);
    }

    const statement_node* get_statement(decltype(children)::size_type index) const {
        return get_child<statement_node>(index * 2);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct compound_statement_node : public non_terminal_node {
    static constexpr std::string_view NAME = "compound-statement";

    compound_statement_node(std::unique_ptr<terminal_node> begin_keyword,
                            std::unique_ptr<statement_list_node>
                                statements,
                            std::unique_ptr<terminal_node>
                                end_keyword)
        : non_terminal_node(COMPOUND_STATEMENT, std::move(begin_keyword), std::move(statements), std::move(end_keyword)) {}

    const statement_list_node* get_statement_list() const {
        return get_child<statement_list_node>(1);
    }

    void accept(parse_tree_visitor& visitor) const override;
};

struct function_definition_node : public non_terminal_node {
    static constexpr std::string_view NAME = "function-definition";

    bool has_parameter_declaration = false;
    bool has_variable_declaration = false;
    bool has_constant_declaration = false;
    function_definition_node(std::unique_ptr<parameter_declaration_node> parameterDeclarations, std::unique_ptr<variable_declaration_node> variableDeclaration, std::unique_ptr<constant_declaration_node> constantDeclarations, std::unique_ptr<compound_statement_node> compundStatement, std::unique_ptr<terminal_node> programTerminator)
        : non_terminal_node(FUNCTION_DEFINITION, std::move(compundStatement), std::move(programTerminator)) {
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

    void accept(parse_tree_visitor& visitor) const override;

    const compound_statement_node* get_compund_statement() const {
        unsigned compound_statement_index = has_constant_declaration + has_variable_declaration + has_parameter_declaration;
        return get_child<compound_statement_node>(compound_statement_index);
    }

    const constant_declaration_node* get_constant_declarations() const {
        return has_constant_declaration ?
            get_child<constant_declaration_node>(has_variable_declaration + has_parameter_declaration) :
            nullptr;
    }

    const variable_declaration_node* get_variable_declarations() const {
        return has_variable_declaration ? get_child<variable_declaration_node>(has_parameter_declaration) : nullptr;
    }

    const parameter_declaration_node* get_parameter_declarations() const {
        return has_parameter_declaration ? get_child<parameter_declaration_node>(0) : nullptr;
    }
};
} // namespace pljit::parser

#endif //PLJIT_PARSE_TREE_NODES_HPP
