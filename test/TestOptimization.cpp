#include "pljit/semantic_analysis/ASTCreator.hpp"
#include <pljit/execution/ExecutionContext.hpp>
#include <pljit/lexer/lexer.hpp>
#include <pljit/optimization/passes/constant_propagation.hpp>
#include <pljit/optimization/passes/dead_code_elimination.hpp>
#include <pljit/optimization/passes/UnaryPlusRemoval.hpp>
#include <pljit/parser/parser.hpp>
#include <pljit/semantic_analysis/AST.hpp>
#include <pljit/semantic_analysis/dot_print_visitor.hpp>
#include <pljit/source_management/source_code.hpp>
#include <gtest/gtest.h>

using namespace pljit;
using namespace pljit::semantic_analysis;
using namespace pljit::source_management;
using namespace pljit::optimization;
using namespace pljit::lexer;

class Optimization : public ::testing::Test {
    protected:
    source_code code;

    std::unique_ptr<FunctionNode> create_ast(std::string_view source_string) {
        code = source_code(source_string);
        pljit::lexer::lexer lexer (code);
        pljit::parser::parser parser(lexer);
        auto parse_tree = parser.parse_function_definition();
        EXPECT_TRUE(parse_tree.get());
        return ASTCreator::CreateAST(*parse_tree);
    }
};

namespace {
    std::string to_dot(ASTNode& ast) {
        std::stringstream output_stream;
        pljit::semantic_analysis::dot_print_visitor dot_printer(output_stream);
        ast.accept(dot_printer);
        return output_stream.str();
    }
} // namespace

TEST_F(Optimization, DeadCodeElimination) {
    auto ref_ast = create_ast("PARAM width, height, depth;\n"
               "VAR volume, some;\n"
               "CONST density = 2400;\n"
               "BEGIN\n"
               "volume := width * height * depth;\n"
               "some := volume + width * 10 + height;\n"
               "RETURN\ndensity * volume\n"
               "END.");


    auto optimized_ast = create_ast("PARAM width, height, depth;\n"
                              "VAR volume, some;\n"
                              "CONST density = 2400;\n"
                              "BEGIN\n"
                              "volume := width * height * depth;\n"
                              "some := volume + width * 10 + height;\n"
                              "RETURN density * volume;\n"
                                "some := volume + width * 10 + height\n"
                              "END.");

    ASSERT_NE(to_dot(*ref_ast), to_dot(*optimized_ast));

    pljit::optimization::passes::dead_code_elimination dead_code;
    dead_code.optimize_ast(optimized_ast);

    ASSERT_EQ(to_dot(*ref_ast), to_dot(*optimized_ast));
}

TEST_F(Optimization, ConstantPropagation) {
    auto ref_ast = create_ast("PARAM width;\n"
                                             "VAR volume, some;\n"
                                             "CONST density = 10;\n"
                                             "BEGIN\n"
                                             "volume := 10;\n"
                                             "some := 20;\n"
                                             "RETURN 100 \n"
                                             "END.");


    auto optimized_ast = create_ast("PARAM width;\n"
                                             "VAR volume, some;\n"
                                             "CONST density = 10;\n"
                                             "BEGIN\n"
                                             "volume := density;\n"
                                             "some := volume + 10;\n"
                                             "RETURN density * volume\n"
                                             "END.");

    ASSERT_NE(to_dot(*ref_ast), to_dot(*optimized_ast));

    pljit::optimization::passes::constant_propagation cp;
    cp.optimize_ast(optimized_ast);

    // Unary plus removal to ensure that the two graphs match
    pljit::optimization::passes::UnaryPlusRemoval upr;
    upr.optimize_ast(ref_ast);

    ASSERT_EQ(to_dot(*ref_ast), to_dot(*optimized_ast));
}

TEST_F(Optimization, ConstantPropagationDivisionByZero) {
    auto ref_ast = create_ast("PARAM width;\n"
                                             "VAR volume, some;\n"
                                             "CONST density = 0;\n"
                                             "BEGIN\n"
                                             "volume := 0;\n"
                                             "some := 10;\n"
                                             "RETURN 0 / 0 \n"
                                             "END.");

    auto optimized_ast = create_ast("PARAM width;\n"
                                                         "VAR volume, some;\n"
                                                         "CONST density = 0;\n"
                                                         "BEGIN\n"
                                                         "volume := density;\n"
                                                         "some := volume + 10;\n"
                                                         "RETURN density / volume\n"
                                                         "END.");

    ASSERT_NE(to_dot(*ref_ast), to_dot(*optimized_ast));

    pljit::optimization::passes::constant_propagation cp;
    cp.optimize_ast(optimized_ast);

    // Unary plus removal to ensure that the two graphs match
    pljit::optimization::passes::UnaryPlusRemoval upr;
    upr.optimize_ast(ref_ast);

    ASSERT_EQ(to_dot(*ref_ast), to_dot(*optimized_ast));
}
