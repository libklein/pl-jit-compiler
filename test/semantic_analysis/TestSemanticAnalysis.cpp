#include <pljit/lexer/lexer.hpp>
#include <pljit/parser/parser.hpp>
#include <pljit/semantic_analysis/AST.hpp>
#include <pljit/semantic_analysis/dot_print_visitor.hpp>
#include <pljit/semantic_analysis/ast_creation_visitor.hpp>
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
        ast_creation_visitor ast_creator;
        parse_tree->accept(ast_creator);
        std::tie(ast, symbolTable) = ast_creator.release_result();
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