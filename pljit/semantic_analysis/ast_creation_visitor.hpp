
#ifndef PLJIT_AST_CREATION_VISITOR_HPP
#define PLJIT_AST_CREATION_VISITOR_HPP

#include "pljit/parser/parse_tree_visitor.hpp"
#include "symbol_table.hpp"
#include "AST.hpp"
#include <unordered_map>
#include <optional>
#include <tuple>

namespace pljit::semantic_analysis {

    class ast_creation_visitor : private pljit::parser::parse_tree_visitor {
        private:
        using symbol_handle = symbol_table::symbol_handle;

        std::unique_ptr<FunctionNode> root;
        std::unique_ptr<ASTNode> next_node;
        symbol_table symbols;
        std::unordered_map<std::string_view, symbol_handle> identifier_mapping;
        symbol::symbol_type next_symbol_type;

        bool construction_failed = false;

        std::pair<symbol_handle, bool> register_symbol(const parser::identifier_node& node, symbol::symbol_type type
                                                       , std::optional<int64_t> value);
        std::optional<symbol_handle> lookup_identifier(std::string_view name) const;

        public:
        /***
         * Analyze a parse tree and return the root of the constructed tree and the symbol table. Should construction fail,
         * returns a nullptr instead of the tree's root.
         */
        static std::pair<std::unique_ptr<ASTRoot>, symbol_table> AnalyzeParseTree(const parser::function_definition_node& parse_tree);

        private:
        // We do not want users to be able to construct our visitor. Instead, they should use the supplied static method
        // as an interface to it.
        ast_creation_visitor() = default;

        std::pair<std::unique_ptr<ASTRoot>, symbol_table> release_result() {
            return {std::move(root), std::move(symbols)};
        }

        // These can still be accessed through a pointer to the base class. However, it is impossible
        // to get such a pointer using conventional methods as this class cannot be constructed.
        // We hide the visit functions for the sake of conciseness. TODO Makes sense?
        void visit(const parser::declarator_list_node& node) override;
        void visit(const parser::identifier_node& node) override;
        void visit(const parser::literal_node& node) override;
        void visit(const parser::terminal_node& node) override;
        void visit(const parser::additive_expression_node& node) override;
        void visit(const parser::assignment_expression_node& node) override;
        void visit(const parser::compound_statement_node& node) override;
        void visit(const parser::constant_declaration_node& node) override;
        void visit(const parser::init_declarator_list_node& node) override;
        void visit(const parser::init_declarator_node& node) override;
        void visit(const parser::multiplicative_expression_node& node) override;
        void visit(const parser::parameter_declaration_node& node) override;
        void visit(const parser::primary_expression_node& node) override;
        void visit(const parser::statement_list_node& node) override;
        void visit(const parser::statement_node& node) override;
        void visit(const parser::function_definition_node& node) override;
        void visit(const parser::unary_expression_node& node) override;
        void visit(const parser::variable_declaration_node& node) override;
    };

} // namespace pljit::semantic_analysis

#endif //PLJIT_AST_CREATION_VISITOR_HPP
