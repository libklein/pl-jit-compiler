
#ifndef PLJIT_ASTCREATOR_HPP
#define PLJIT_ASTCREATOR_HPP

// TODO Replace with ast_fwd
#include "pljit/semantic_analysis/symbol_table.hpp"
#include "pljit/semantic_analysis/AST.hpp"

//---------------------------------------------------------------------------
namespace pljit::semantic_analysis {
class ASTCreator {
    private:
    pljit::semantic_analysis::symbol_table symbols;
    using symbol_handle = pljit::semantic_analysis::symbol_table::size_type;
    std::unordered_map<std::string_view, symbol_handle> identifier_mapping;

    // Helpers for the symbol table
    std::pair<symbol_handle, bool> register_symbol(const pljit::parser::identifier_node& node, pljit::semantic_analysis::symbol::symbol_type type, std::optional<int64_t> value);

    bool analyze_declarations(const pljit::parser::declarator_list_node& node, pljit::semantic_analysis::symbol::symbol_type symbolType);
    bool analyze_declarations(const pljit::parser::init_declarator_list_node& node, pljit::semantic_analysis::symbol::symbol_type symbolType);

    std::unique_ptr<pljit::semantic_analysis::IdentifierNode> analyze_identifier(const pljit::parser::identifier_node& node);
    std::unique_ptr<pljit::semantic_analysis::LiteralNode> analyze_literal(const pljit::parser::literal_node& node);
    std::unique_ptr<pljit::semantic_analysis::ReturnStatementNode> analyze_return_statement(const pljit::parser::statement_node& node);
    std::unique_ptr<pljit::semantic_analysis::AssignmentNode> analyze_assignment_node(const pljit::parser::assignment_expression_node& node);

    std::unique_ptr<pljit::semantic_analysis::UnaryOperatorASTNode> analyze_expression(const pljit::parser::unary_expression_node& node);
    std::unique_ptr<pljit::semantic_analysis::ExpressionNode> analyze_expression(const pljit::parser::multiplicative_expression_node& node);
    std::unique_ptr<pljit::semantic_analysis::ExpressionNode> analyze_expression(const pljit::parser::additive_expression_node& node);
    std::unique_ptr<pljit::semantic_analysis::ExpressionNode> analyze_expression(const pljit::parser::primary_expression_node& node);
    std::unique_ptr<pljit::semantic_analysis::FunctionNode> analyze_function(const pljit::parser::function_definition_node& parseTree);

    public:
    static std::pair<std::unique_ptr<pljit::semantic_analysis::FunctionNode>, pljit::semantic_analysis::symbol_table> CreateAST(const pljit::parser::function_definition_node& parseTree);
};
} // namespace pljit::semantic_analysis
#endif //PLJIT_ASTCREATOR_HPP