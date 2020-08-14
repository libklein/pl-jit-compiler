
#ifndef PLJIT_PARSE_TREE_NODES_HPP
#define PLJIT_PARSE_TREE_NODES_HPP

#include <pljit/lexer/token.hpp>
#include <utility>

namespace pljit::parser {

    class parse_tree_visitor;

    enum grammar_type {
        FUNCTION_DEFINITION,
        PARAMETER_DECLARATIONS,
        VARIABLE_DECLARATIONS,
        CONSTANT_DECLARATIONS,
        DECLARATOR_LIST,
        INIT_DECLARATOR_LIST,
        INIT_DECLARATOR,
        COMPOUND_STATEMENT,
        STATEMENT_LIST,
        STATEMENT,
        ASSIGNMENT_EXPRESSION,
        MULTIPLICATIVE_EXPRESSION,
        ADDITIVE_EXPRESSION,
        UNARY_EXPRESSION,
        PRIMARY_EXPRESSION,

        LITERAL,
        IDENTIFIER,
        GENERIC_NODE
    };

    struct node_base {
        protected:
        grammar_type type;
        explicit node_base(grammar_type type) : type(type) {};
        public:

        //virtual std::string_view get_name() const = 0;

        grammar_type get_type() const {
            return type;
        };

        virtual void accept(parse_tree_visitor& visitor) = 0;

        virtual ~node_base() = default;
    };

    using node_ptr = std::unique_ptr<node_base>;

    class non_terminal_node : public node_base {
        protected:
        using node_ptr_container_type = std::vector<node_ptr>;

        node_ptr_container_type children;
        node_ptr_container_type::size_type dynamic_child_count = 0;

        using node_base::node_base;

        template<class ...Args>
        explicit non_terminal_node(grammar_type type, Args... args) : node_base(type), children(0) {
            static_assert(std::conjunction_v<std::is_convertible<Args, node_ptr>...>,
                "Non-terminal parse tree node cannot contain non-nodes");
            assert((args && ...));
            children.reserve(sizeof...(args));
            (children.emplace_back(std::move(args)),...);
        }

        template<class ...Args>
        bool add_child(Args... args) {
            if(!(args && ...)) {
                return false;
            }
            dynamic_child_count += sizeof...(args);
            (children.emplace_back(std::move(args)),...);
            return true;
        }

        auto insert_child(decltype(children)::iterator position, std::unique_ptr<node_base> child) {
            ++dynamic_child_count;
            return children.insert(position, std::move(child));
        }

        public:
        auto number_of_dynamic_children() const {
            return dynamic_child_count;
        }

        const node_ptr_container_type& get_children() const {
            return children;
        }
    };

    struct terminal_node : public node_base {
        lexer::token token;

        public:
        static constexpr std::string_view NAME = "terminal";

        explicit terminal_node(lexer::token token) : node_base(GENERIC_NODE), token(token) {}

        void accept(parse_tree_visitor& visitor) override;

        const lexer::token& get_token() const {
            return token;
        }
    };

    struct literal_node : public non_terminal_node {
        public:
        static constexpr std::string_view NAME = "literal";

        explicit literal_node(node_ptr token) : non_terminal_node(LITERAL, std::move(token)) {}
        void accept(parse_tree_visitor& visitor) override;
    };

    struct identifier_node : public non_terminal_node {
        public:
        static constexpr std::string_view NAME = "identifier";

        explicit identifier_node(node_ptr token) : non_terminal_node(IDENTIFIER, std::move(token)) {}
        void accept(parse_tree_visitor& visitor) override;
    };

    struct init_declarator_node : public non_terminal_node {
        static constexpr std::string_view NAME = "init-declarator";

        init_declarator_node(std::unique_ptr<identifier_node> identifier,
                             std::unique_ptr<terminal_node> assignmentOperator,
                             std::unique_ptr<literal_node> literal)
            : non_terminal_node(INIT_DECLARATOR,
                                           std::move(identifier), std::move(assignmentOperator), std::move(literal)) {}

        void accept(parse_tree_visitor& visitor) override;
    };

    struct declarator_list_node : public non_terminal_node {
        static constexpr std::string_view NAME = "declarator-list";

        declarator_list_node(std::unique_ptr<identifier_node> identifier,
        std::vector<std::pair<
            std::unique_ptr<terminal_node>,
            std::unique_ptr<identifier_node>
        >>  otherIdentifiers) : non_terminal_node(DECLARATOR_LIST, std::move(identifier)) {
            for(auto& entry : otherIdentifiers) {
                add_child(std::move(entry.first), std::move(entry.second));
            }
        }

        void accept(parse_tree_visitor& visitor) override;
    };

struct init_declarator_list_node : public non_terminal_node {
    static constexpr std::string_view NAME = "init-declarator-list";

    init_declarator_list_node(std::unique_ptr<init_declarator_node> identifier,
                                  std::vector<std::pair<
                                      std::unique_ptr<terminal_node>,
                                          std::unique_ptr<init_declarator_node>
                                      >>  otherIdentifiers) : non_terminal_node(INIT_DECLARATOR_LIST, std::move(identifier)) {
            for(auto& entry : otherIdentifiers) {
                add_child(std::move(entry.first), std::move(entry.second));
            }
        }

    void accept(parse_tree_visitor& visitor) override;
};

struct parameter_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "parameter-declaration";

    parameter_declaration_node(std::unique_ptr<terminal_node> paramKeyword,
                               std::unique_ptr<declarator_list_node>  declarators,
                               std::unique_ptr<terminal_node> statementTerminator)
        : non_terminal_node(PARAMETER_DECLARATIONS, std::move(paramKeyword), std::move(declarators), std::move(statementTerminator)) {}

    void accept(parse_tree_visitor& visitor) override;
};

struct variable_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "variable-declaration";

    variable_declaration_node(std::unique_ptr<terminal_node> variable_keyword,
                              std::unique_ptr<declarator_list_node>  declarators,
                              std::unique_ptr<terminal_node> statement_terminator)
        : non_terminal_node(VARIABLE_DECLARATIONS, std::move(variable_keyword), std::move(declarators), std::move(statement_terminator)) {}

    void accept(parse_tree_visitor& visitor) override;
};

struct constant_declaration_node : public non_terminal_node {
    static constexpr std::string_view NAME = "constant-declaration";

    constant_declaration_node(std::unique_ptr<terminal_node> const_keyword,
                                  std::unique_ptr<init_declarator_list_node> declarators,
                                  std::unique_ptr<terminal_node> statement_terminator)
            : non_terminal_node(CONSTANT_DECLARATIONS, std::move(const_keyword), std::move(declarators), std::move(statement_terminator)) {}

    void accept(parse_tree_visitor& visitor) override;
};

struct multiplicative_expression_node;

struct additive_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "additive-expression";

    bool has_subexpression = false;
        additive_expression_node(std::unique_ptr<multiplicative_expression_node> multiplicative_expression,
        std::unique_ptr<terminal_node> binary_operator,
            std::unique_ptr<additive_expression_node> additive_expression)
        : non_terminal_node(ADDITIVE_EXPRESSION, std::move(multiplicative_expression)) {
            if(binary_operator && additive_expression) {
                has_subexpression = this->add_child(std::move(binary_operator), std::move(additive_expression));
            }
        }

    void accept(parse_tree_visitor& visitor) override;
};

struct primary_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "primary-expression";

    // Use scoped enum to avoid confusion with node type
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
                                         std::unique_ptr<additive_expression_node> additive_expression,
                                         std::unique_ptr<terminal_node> r_bracket)
            : non_terminal_node(PRIMARY_EXPRESSION, std::move(l_bracket), std::move(additive_expression), std::move(r_bracket)),
              statement_type(primary_expression_type::ADDITIVE_EXPRESSION) {}

    void accept(parse_tree_visitor& visitor) override;
};

struct unary_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "unary-expression";

    bool has_unary_operator = false;
        unary_expression_node(std::unique_ptr<terminal_node> unary_operator, std::unique_ptr<primary_expression_node> primary_expression)
        : non_terminal_node(UNARY_EXPRESSION) {
            has_unary_operator = this->add_child(std::move(unary_operator));
            this->add_child(std::move(primary_expression));
        }

    void accept(parse_tree_visitor& visitor) override;
};

struct multiplicative_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "multiplicative-expression";

    bool has_subexpression = false;
        multiplicative_expression_node(std::unique_ptr<unary_expression_node> unary_expression,
        std::unique_ptr<terminal_node> binary_operator,
            std::unique_ptr<multiplicative_expression_node> multiplicative_expression)
        : non_terminal_node(MULTIPLICATIVE_EXPRESSION, std::move(unary_expression)) {
            if(binary_operator && multiplicative_expression) {
                has_subexpression = this->add_child(std::move(binary_operator), std::move(multiplicative_expression));
            }
        }

    void accept(parse_tree_visitor& visitor) override;
};

struct assignment_expression_node : public non_terminal_node {
    static constexpr std::string_view NAME = "assignment-expression";

    assignment_expression_node(std::unique_ptr<identifier_node> identifier, std::unique_ptr<terminal_node> assignment_operator, std::unique_ptr<additive_expression_node> additive_expression)
        : non_terminal_node(ASSIGNMENT_EXPRESSION, std::move(identifier), std::move(assignment_operator), std::move(additive_expression)) {}

    void accept(parse_tree_visitor& visitor) override;
};

struct statement_node : public non_terminal_node {
    static constexpr std::string_view NAME = "statement";

    // Use scoped enum to avoid confusion with node type
        enum class statement_type {
            RETURN_STATEMENT,
            ASSIGNMENT_STATEMENT
        };

        statement_type statement_type;
        statement_node(std::unique_ptr<assignment_expression_node> assignment) : non_terminal_node(STATEMENT, std::move(assignment)), statement_type(statement_type::RETURN_STATEMENT) {}
        statement_node(std::unique_ptr<terminal_node> return_keyword, std::unique_ptr<additive_expression_node> additive_expression) : non_terminal_node(STATEMENT, std::move(return_keyword), std::move(additive_expression)), statement_type(statement_type::ASSIGNMENT_STATEMENT) {}

    void accept(parse_tree_visitor& visitor) override;
};

struct statement_list_node : public non_terminal_node {
    static constexpr std::string_view NAME = "statement-list";

    statement_list_node(std::unique_ptr<statement_node> statement,
                            std::vector<std::pair<std::unique_ptr<terminal_node>, std::unique_ptr<statement_node>>> additional_statements)
        : non_terminal_node(STATEMENT_LIST, std::move(statement)) {
            for(auto& [separator, next_statement] : additional_statements) {
                add_child(std::move(separator), std::move(next_statement));
            }
        }

    void accept(parse_tree_visitor& visitor) override;
};

struct compound_statement_node : public non_terminal_node {
    static constexpr std::string_view NAME = "compound-statement";

    compound_statement_node(std::unique_ptr<terminal_node> begin_keyword,
        std::unique_ptr<statement_list_node> statements,
            std::unique_ptr<terminal_node> end_keyword)
        : non_terminal_node(COMPOUND_STATEMENT, std::move(begin_keyword), std::move(statements), std::move(end_keyword)) {}

    void accept(parse_tree_visitor& visitor) override;
};

struct function_defition_node : public non_terminal_node {
    static constexpr std::string_view NAME = "function-definition";

    bool has_parameter_declaration = false;
    bool has_variable_declaration = false;
    bool has_constant_declaration = false;
    function_defition_node(std::unique_ptr<parameter_declaration_node>  parameterDeclarations, std::unique_ptr<variable_declaration_node>  variableDeclaration, std::unique_ptr<constant_declaration_node>  constantDeclarations, std::unique_ptr<compound_statement_node> compundStatement, std::unique_ptr<terminal_node> programTerminator)
            : non_terminal_node(FUNCTION_DEFINITION, std::move(compundStatement), std::move(programTerminator)) {
        if(constantDeclarations) {
            has_constant_declaration = true;
            this->insert_child(children.begin(), std::move(constantDeclarations));
        }
        if(variableDeclaration) {
            has_variable_declaration = true;
            this->insert_child(children.begin(), std::move(variableDeclaration));
        }
        if(parameterDeclarations) {
            has_parameter_declaration = true;
            this->insert_child(children.begin(), std::move(parameterDeclarations));
        }
    }

    void accept(parse_tree_visitor& visitor) override;
};
} // namespace pljit::parser

#endif //PLJIT_PARSE_TREE_NODES_HPP
