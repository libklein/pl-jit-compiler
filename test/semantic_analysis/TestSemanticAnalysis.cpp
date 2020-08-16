#include <pljit/lexer/lexer.hpp>
#include <pljit/parser/parser.hpp>
#include <pljit/semantic_analysis/AST.hpp>
#include "pljit/semantic_analysis/ASTCreator.hpp"
#include <pljit/semantic_analysis/dot_print_visitor.hpp>
#include <pljit/source_management/source_code.hpp>
#include <gtest/gtest.h>

using namespace pljit::semantic_analysis;
using namespace pljit::source_management;
using namespace pljit::lexer;

class SemanticAnalysis : public ::testing::Test {
    protected:
    source_code code;
    symbol_table symbolTable;
    std::unique_ptr<FunctionNode> ast;

    std::unique_ptr<FunctionNode>& create_ast(std::string_view source_string) {
        code = source_code(source_string);
        pljit::lexer::lexer lexer (code);
        pljit::parser::parser parser(lexer);
        auto parse_tree = parser.parse_function_definition();
        EXPECT_TRUE(parse_tree);
        std::tie(ast, symbolTable) = ASTCreator::CreateAST(*parse_tree);
        return ast;
    }

    void TearDown() override {
        ast.reset();
    }
};

TEST_F(SemanticAnalysis, MissingReturnStatement) {
    create_ast("VAR density;"
                "BEGIN\n"
               "density := 10\n"
               "END.");
    EXPECT_FALSE(ast);
}

TEST_F(SemanticAnalysis, UndeclaredIdentifier) {
    create_ast("BEGIN\n"
               "RETURN density\n"
               "END.");
    EXPECT_FALSE(ast);
}

TEST_F(SemanticAnalysis, AssignmentToConstant) {
    create_ast("CONST density = 1;\n"
                "BEGIN\n"
               "density := 10;\n"
               "RETURN density\n"
               "END.");
    EXPECT_FALSE(ast);
}

TEST_F(SemanticAnalysis, UninitializedVariable) {
    create_ast("VAR density;\n"
               "BEGIN\n"
               "RETURN density\n"
               "END.");
    EXPECT_FALSE(ast);

    create_ast("PARAM density;\n"
               "BEGIN\n"
               "RETURN density\n"
               "END.");
    EXPECT_TRUE(ast);
}

TEST_F(SemanticAnalysis, RedeclarationIdentifier) {
    create_ast("PARAM d, d;"
                "BEGIN\n"
               "RETURN density\n"
               "END.");
    ASSERT_FALSE(ast);

    create_ast("PARAM d;\n"
               "CONST d = 2;"
               "BEGIN\n"
               "RETURN density\n"
               "END.");
    ASSERT_FALSE(ast);
}

TEST_F(SemanticAnalysis, SymbolTableCapturesAllDeclarations) {
    using symbol_type = symbol::symbol_type;
    create_ast("PARAM width, height, depth;\n"
               "VAR volume, some;\n"
               "CONST density = 2400;\n"
                "BEGIN RETURN 0 END.");
    ASSERT_TRUE(ast);
    EXPECT_EQ(symbolTable.get_number_of_parameters(), 3);
    EXPECT_EQ(symbolTable.get_number_of_variables(), 2);
    EXPECT_EQ(symbolTable.get_number_of_constants(), 1);
    EXPECT_EQ(symbolTable.size(), 6);

    std::vector<std::tuple<std::string, symbol::symbol_type, int64_t, bool>> expected_symbols {
        {"width", symbol_type::PARAMETER, 0, true},
        {"height", symbol_type::PARAMETER, 0, true},
        {"depth", symbol_type::PARAMETER, 0, true},
        {"volume", symbol_type::VARIABLE, 0, false},
        {"some", symbol_type::VARIABLE, 0, false},
        {"density", symbol_type::CONSTANT, 2400, true}
    };

    auto next_expected_symbol = expected_symbols.begin();
    for(const auto &symbol : symbolTable) {
        auto [expected_name, expected_type, expected_value, initialized] = *next_expected_symbol;
        EXPECT_EQ(symbol.get_name(), expected_name);
        EXPECT_EQ(symbol.type, expected_type);
        EXPECT_EQ(symbol.initialized, initialized);
        if(symbol.type == symbol_type::CONSTANT) {
            EXPECT_EQ(symbol.get_value(), expected_value);
        }
        ++next_expected_symbol;
    }
}

TEST_F(SemanticAnalysis, ConstantsHaveValueSet) {
    create_ast("CONST density = 2400, a = 10;\n"
               "BEGIN RETURN density END.");

    ASSERT_TRUE(ast);
    EXPECT_EQ(symbolTable.constants_begin()->get_value(), 2400);
    EXPECT_EQ(std::next(symbolTable.constants_begin())->get_value(), 10);
}

TEST_F(SemanticAnalysis, DOTVisitor) {
    create_ast("PARAM width, height, depth;\n"
               "VAR volume, some;\n"
               "CONST density = 2400;\n"
               "BEGIN\n"
               "volume := width * height * depth;\n"
               "some := volume + width * 10 + height;\n"
               "RETURN\ndensity * volume\n"
               "END.");

    ASSERT_TRUE(ast);

    // Create dot visitor
    dot_print_visitor dot_visitor;
    ast->accept(dot_visitor);
}