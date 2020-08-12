
#include <pljit/source_management/source_code.hpp>
#include <pljit/lexer/lexer.hpp>
#include <pljit/parser/parse_tree_nodes.hpp>
#include <pljit/parser/parser.hpp>
#include <gtest/gtest.h>

using namespace pljit::source_management;
using namespace pljit::parser;
using namespace pljit::lexer;

class ParserTest : public ::testing::Test {
    protected:
    /*const source_code code{"PARAM width, height, depth;\n"
                           "VAR volume;\n"
                           "CONST density = 2400;\n"
                           "BEGIN\n"
                           "volume :=width * height * depth;\n"
                           "RETURN\ndensity * volume\n"
                           "END."};*/
    const source_code code{"PARAM width, height, depth;."};
};

TEST_F(ParserTest, BasicInputTest) {
    lexer l(code);
    parser parser(l);
    auto parse_tree = parser.parse_test_function_definition();
}