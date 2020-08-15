#include <gtest/gtest.h>
#include <pljit/lexer/lexer.hpp>
#include <pljit/parser/parser.hpp>
#include <pljit/semantic_analysis/AST.hpp>
#include <pljit/semantic_analysis/dot_print_visitor.hpp>
#include <pljit/semantic_analysis/ast_creation_visitor.hpp>
#include <pljit/source_management/source_code.hpp>
#include <pljit/Executor.hpp>

using namespace pljit;
using namespace pljit::semantic_analysis;
using namespace pljit::source_management;
using namespace pljit::lexer;

class Execution : public ::testing::Test {
    protected:
    template<class... Args>
    std::optional<int64_t> execute(std::string_view source_string, Args&&... parameters) {
        source_code code(source_string);
        pljit::lexer::lexer lexer (code);
        pljit::parser::parser parser(lexer);
        auto parse_tree = parser.parse_function_definition();
        EXPECT_TRUE(parse_tree);
        auto [ast, symbolTable] = ast_creation_visitor::AnalyzeParseTree(*parse_tree);
        EXPECT_TRUE(ast);
        return Executor::Execute(*ast, symbolTable, std::forward<Args>(parameters)...);
    }
};

TEST_F(Execution, BasicExecution) {
    auto result = execute("PARAM width, height, depth;\n"
               "VAR volume, some;\n"
               "CONST density = 2400;\n"
               "BEGIN\n"
               "volume := width * height * depth;\n" // 1000
               "some := volume + width * 10 + height;\n" // 1110
               "RETURN\ndensity * volume\n" // 2400000
               "END.", 10, 10, 10);
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 2400000);
}

TEST_F(Execution, DivisionByZero) {
    auto result = execute("CONST a = 0; BEGIN RETURN 1000 / a END.");
    EXPECT_FALSE(result);

    result = execute("VAR a; BEGIN a := 10; RETURN 1000 / (a-10) END.");
    EXPECT_FALSE(result);

    result = execute("BEGIN RETURN 1000 / 0 END.");
    EXPECT_FALSE(result);

    result = execute("BEGIN RETURN 1000 / -0 END.");
    EXPECT_FALSE(result);
}

TEST_F(Execution, InvalidNumberOfParameters) {
    EXPECT_DEBUG_DEATH(execute("PARAM a; BEGIN RETURN 0 END."), "Assertion .* failed");
    EXPECT_DEBUG_DEATH(execute("PARAM a; BEGIN RETURN 0 END.", 1, 2), "Assertion .* failed");
}

