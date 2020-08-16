
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
    source_management::SourceFragment codeReference;
    explicit node_base(grammar_type type, source_management::SourceFragment source);

    public:
    grammar_type get_type() const;

    source_management::SourceFragment getCodeReference() const;

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
    explicit non_terminal_node(grammar_type type, source_management::SourceFragment source, Args... args) : node_base(type, source), children(0) {
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

    node_ptr_container_type::iterator insert_child(node_ptr_container_type::iterator position, std::unique_ptr<node_base> child);

    public:
    using size_type = node_ptr_container_type::size_type;

    node_ptr_container_type::size_type get_number_of_dynamic_children() const;

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

    explicit terminal_node(lexer::token token);

    void accept(parse_tree_visitor& visitor) const override;

    const lexer::token& get_token() const;

    std::string_view get_text() const;
};

struct literal_node : public non_terminal_node {
    public:
    static constexpr std::string_view NAME = "literal";

    explicit literal_node(node_ptr token, source_management::SourceFragment source);
    void accept(parse_tree_visitor& visitor) const override;

    lexer::token get_token() const;

    int64_t get_value() const;
};

struct identifier_node : public non_terminal_node {
    public:
    static constexpr std::string_view NAME = "identifier";

    identifier_node(node_ptr token, source_management::SourceFragment source);
    void accept(parse_tree_visitor& visitor) const override;

    lexer::token get_token() const;

    std::string_view get_name() const;
};

struct init_declarator_node : public non_terminal_node {
    static constexpr std::string_view NAME = "init-declarator";

    init_declarator_node(source_management::SourceFragment fragment, std::unique_ptr<identifier_node> identifier,
                         std::unique_ptr<terminal_node>
                             assignmentOperator,
                         std::unique_ptr<literal_node> literal);

    void accept(parse_tree_visitor& visitor) const override;

    const identifier_node* get_identifier() const;

    const literal_node* get_value() const;
};

struct declarator_list_node : public non_terminal_node {
    static constexpr std::string_view NAME = "declarator-list";

    declarator_list_node(source_management::SourceFragment fragment, std::unique_ptr<identifier_node> identifier,
                         std::vector<std::pair<
                             std::unique_ptr<terminal_node>,
                             std::unique_ptr<identifier_node>>>
                             otherIdentifiers);

    non_terminal_node::size_type get_number_of_declarations() const;

    const identifier_node* get_declaration(decltype(children)::size_type index) const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct init_declarator_list_node : public non_terminal_node {
    static constexpr std::string_view NAME = "init-declarator-list";

    init_declarator_list_node(source_management::SourceFragment fragment,
                              std::unique_ptr<init_declarator_node>
                                  identifier,
                              std::vector<std::pair<
                                  std::unique_ptr<terminal_node>,
                                  std::unique_ptr<init_declarator_node>>>
                                  otherIdentifiers);

    size_type get_number_of_declarations() const;

    const init_declarator_node* get_declaration(decltype(children)::size_type index) const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct parameter_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "parameter-declaration";

    // TODO Superclass
    parameter_declaration_node(source_management::SourceFragment fragment,
                               std::unique_ptr<terminal_node>
                                   paramKeyword,
                               std::unique_ptr<declarator_list_node>
                                   declarators,
                               std::unique_ptr<terminal_node>
                                   statementTerminator);

    const declarator_list_node* get_declarator_list() const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct variable_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "variable-declaration";

    variable_declaration_node(source_management::SourceFragment fragment,
                              std::unique_ptr<terminal_node>
                                  variable_keyword,
                              std::unique_ptr<declarator_list_node>
                                  declarators,
                              std::unique_ptr<terminal_node>
                                  statement_terminator);

    const declarator_list_node* get_declarator_list() const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct constant_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "constant-declaration";

    constant_declaration_node(source_management::SourceFragment fragment,
                              std::unique_ptr<terminal_node>
                                  const_keyword,
                              std::unique_ptr<init_declarator_list_node>
                                  declarators,
                              std::unique_ptr<terminal_node>
                                  statement_terminator);

    const init_declarator_list_node* get_init_declarator_list() const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct additive_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "additive-expression";
    enum class OperationType {
        PLUS,
        MINUS
    };

    OperationType op;
    bool has_subexpression = false;
    additive_expression_node(source_management::SourceFragment fragment,
                             std::unique_ptr<multiplicative_expression_node>
                                 multiplicative_expression,
                             std::unique_ptr<terminal_node>
                                 binary_operator,
                             std::unique_ptr<additive_expression_node>
                                 additive_expression);

    OperationType get_operation() const;

    void accept(parse_tree_visitor& visitor) const override;

    const multiplicative_expression_node* get_lhs_expression() const;

    const terminal_node* get_operator() const;

    const additive_expression_node* get_rhs_expression() const;
};

struct primary_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "primary-expression";

    enum class primary_expression_type {
        IDENTIFIER,
        LITERAL,
        ADDITIVE_EXPRESSION
    };

    primary_expression_type statement_type;

    explicit primary_expression_node(source_management::SourceFragment fragment,
                                     std::unique_ptr<identifier_node>
                                         identifier);

    explicit primary_expression_node(source_management::SourceFragment fragment,
                                     std::unique_ptr<literal_node>
                                         literal);

    explicit primary_expression_node(source_management::SourceFragment fragment,
                                     std::unique_ptr<terminal_node>
                                         l_bracket,
                                     std::unique_ptr<additive_expression_node>
                                         additive_expression,
                                     std::unique_ptr<terminal_node>
                                         r_bracket);

    const identifier_node* get_identifier() const;

    const literal_node* get_literal() const;

    const additive_expression_node* get_expression() const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct unary_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "unary-expression";
    enum class OperationType {
        PLUS,
        MINUS
    };

    OperationType op = OperationType::PLUS;
    bool has_unary_operator = false;
    unary_expression_node(source_management::SourceFragment fragment, std::unique_ptr<terminal_node> unary_operator, std::unique_ptr<primary_expression_node> primary_expression);

    void accept(parse_tree_visitor& visitor) const override;

    OperationType get_operation() const;

    const terminal_node* get_unary_operator() const;

    const primary_expression_node* get_expression() const;
};

struct multiplicative_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "multiplicative-expression";
    enum class OperationType {
        MULTIPLY,
        DIVIDE
    };

    OperationType op;

    // TODO Expression base class
    bool has_subexpression = false;
    multiplicative_expression_node(source_management::SourceFragment fragment,
                                   std::unique_ptr<unary_expression_node>
                                       unary_expression,
                                   std::unique_ptr<terminal_node>
                                       binary_operator,
                                   std::unique_ptr<multiplicative_expression_node>
                                       multiplicative_expression);

    const unary_expression_node* get_lhs_expression() const;

    const terminal_node* get_operator() const;

    OperationType get_operation() const;

    const multiplicative_expression_node* get_rhs_expression() const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct assignment_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "assignment-expression";

    assignment_expression_node(source_management::SourceFragment fragment, std::unique_ptr<identifier_node> identifier, std::unique_ptr<terminal_node> assignment_operator, std::unique_ptr<additive_expression_node> additive_expression);

    void accept(parse_tree_visitor& visitor) const override;

    const identifier_node* get_identifier() const;

    const additive_expression_node* get_expression() const;
};

struct statement_node : public non_terminal_node {
    static constexpr std::string_view NAME = "statement";

    // Use scoped enum to avoid confusion with node type
    enum class statement_type {
        RETURN_STATEMENT,
        ASSIGNMENT_STATEMENT
    };

    statement_type statement_type;
    explicit statement_node(source_management::SourceFragment fragment, std::unique_ptr<assignment_expression_node> assignment);
    statement_node(source_management::SourceFragment fragment, std::unique_ptr<terminal_node> return_keyword, std::unique_ptr<additive_expression_node> additive_expression) : non_terminal_node(STATEMENT, fragment, std::move(return_keyword), std::move(additive_expression)), statement_type(statement_type::RETURN_STATEMENT) {}

    const assignment_expression_node* get_assignment() const;

    const additive_expression_node* get_return_expression() const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct statement_list_node : public non_terminal_node {
    static constexpr std::string_view NAME = "statement-list";

    statement_list_node(source_management::SourceFragment fragment, std::unique_ptr<statement_node> statement,
                        std::vector<std::pair<std::unique_ptr<terminal_node>, std::unique_ptr<statement_node>>>
                            additional_statements);

    size_type get_number_of_statements() const;

    const statement_node* get_statement(decltype(children)::size_type index) const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct compound_statement_node : public non_terminal_node {
    static constexpr std::string_view NAME = "compound-statement";

    compound_statement_node(source_management::SourceFragment fragment,
                            std::unique_ptr<terminal_node>
                                begin_keyword,
                            std::unique_ptr<statement_list_node>
                                statements,
                            std::unique_ptr<terminal_node>
                                end_keyword);

    const statement_list_node* get_statement_list() const;

    void accept(parse_tree_visitor& visitor) const override;
};

struct function_definition_node : public non_terminal_node {
    static constexpr std::string_view NAME = "function-definition";

    bool has_parameter_declaration = false;
    bool has_variable_declaration = false;
    bool has_constant_declaration = false;
    function_definition_node(source_management::SourceFragment fragment, std::unique_ptr<parameter_declaration_node> parameterDeclarations, std::unique_ptr<variable_declaration_node> variableDeclaration, std::unique_ptr<constant_declaration_node> constantDeclarations, std::unique_ptr<compound_statement_node> compundStatement, std::unique_ptr<terminal_node> programTerminator);

    void accept(parse_tree_visitor& visitor) const override;

    const compound_statement_node* get_compund_statement() const;

    const constant_declaration_node* get_constant_declarations() const;

    const variable_declaration_node* get_variable_declarations() const;

    const parameter_declaration_node* get_parameter_declarations() const;
};
} // namespace pljit::parser

#endif //PLJIT_PARSE_TREE_NODES_HPP
