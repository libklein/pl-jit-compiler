
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
    source_code code;

    std::unique_ptr<function_definition_node> parse_code(std::string_view source_code_string) {
        code = source_code(source_code_string);
        lexer l(code);
        parser parser(l);
        return parser.parse_function_definition();
    }
};

TEST_F(ParserTest, BasicInputTest) {
    auto parse_tree = parse_code("PARAM width, height, depth;\n"
                                 "VAR volume, some;\n"
                                 "CONST density = 2400;\n"
                                 "BEGIN\n"
                                 "volume := width * height * depth;\n"
                                 "some := volume + width * 10 + height;\n"
                                 "RETURN\ndensity * volume\n"
                                 "END.");
    ASSERT_TRUE(parse_tree);
}

TEST_F(ParserTest, NegativeConst) {
    auto parse_tree = parse_code("CONST density = -2400;\n"
                                 "BEGIN\n"
                                 "RETURN density\n"
                                 "END.");
    ASSERT_FALSE(parse_tree);
}

TEST_F(ParserTest, SemicolonAfterReturn) {
    auto parse_tree = parse_code("BEGIN\n"
                                 "RETURN 10;\n"
                                 "END.");
    ASSERT_FALSE(parse_tree);
}

TEST_F(ParserTest, InputAfterProgramEnd) {
    auto parse_tree = parse_code("PARAM width, height, depth;.PARAM width;.");
    ASSERT_FALSE(parse_tree);
}

TEST_F(ParserTest, MissingListItem) {
    auto parse_tree = parse_code("PARAM width, , depth;.");
    ASSERT_FALSE(parse_tree);
}

TEST_F(ParserTest, InvalidTokenStream) {
    auto parse_tree = parse_code("PARAM wid!th, height, depth;.");
    ASSERT_FALSE(parse_tree);
}

TEST_F(ParserTest, TestDotVisitor) {
    auto parse_tree = parse_code("PARAM width, height, depth;\n"
                                 "VAR volume, some;\n"
                                 "CONST density = 2400;\n"
                                 "BEGIN\n"
                                 "volume := width * height * depth;\n"
                                 "some := volume + width * 10 + height;\n"
                                 "RETURN\ndensity * volume\n"
                                 "END.");
    ASSERT_TRUE(parse_tree);
    dot_print_visitor visitor;
    parse_tree->accept(visitor);
}
