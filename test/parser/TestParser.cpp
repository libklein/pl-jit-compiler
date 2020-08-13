
#include <pljit/lexer/lexer.hpp>
#include <pljit/parser/dot_print_visitor.hpp>
#include <pljit/parser/parse_tree_nodes.hpp>
#include <pljit/parser/parser.hpp>
#include <pljit/source_management/source_code.hpp>
#include <gtest/gtest.h>

using namespace pljit::source_management;
using namespace pljit::parser;
using namespace pljit::lexer;

class ParserTest : public ::testing::Test {
    protected:
    const source_code code{"PARAM width, height, depth;\n"
                           "VAR volume;\n"
                           "CONST density = 2400;\n"
                           /*"BEGIN\n"
                           "volume :=width * height * depth;\n"
                           "RETURN\ndensity * volume\n"
                           "END*/"."};
    //const source_code code{"PARAM width, height, depth;."};
};

TEST_F(ParserTest, BasicInputTest) {
    lexer l(code);
    parser parser(l);
    auto parse_tree = parser.parse_test_function_definition();
    ASSERT_TRUE(parse_tree);
}

TEST_F(ParserTest, InputAfterProgramEnd) {
    source_code code("PARAM width, height, depth;.PARAM width;.");
    lexer l(code);
    parser parser(l);
    auto parse_tree = parser.parse_test_function_definition();
    ASSERT_FALSE(parse_tree);
}

TEST_F(ParserTest, MissingListItem) {
    source_code code("PARAM width, , depth;.");
    lexer l(code);
    parser parser(l);
    auto parse_tree = parser.parse_test_function_definition();
    ASSERT_FALSE(parse_tree);
}

TEST_F(ParserTest, InvalidTokenStream) {
    source_code code("PARAM wid!th, height, depth;.");
    lexer l(code);
    parser parser(l);
    auto parse_tree = parser.parse_test_function_definition();
    ASSERT_FALSE(parse_tree);
}

TEST_F(ParserTest, TestDotVisitor) {
    lexer l(code);
    parser parser(l);
    auto parse_tree = parser.parse_test_function_definition();
    ASSERT_TRUE(parse_tree);
    dot_print_visitor visitor;
    parse_tree->accept(visitor);
}
