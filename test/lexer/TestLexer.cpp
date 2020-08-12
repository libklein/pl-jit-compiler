
#include "pljit/lexer/lexer.hpp"
#include "pljit/source_management/source_code.hpp"
#include <array>
#include <gtest/gtest.h>

/*
 * TODO: Write tests
 * - Different amounts/characters of whitespace
 * - All tokens
 * - Error tokens
 */
using namespace pljit::source_management;
using namespace pljit::lexer;

class LexerTest : public ::testing::Test {
    protected:
    const source_code code{"PARAM width, height, depth;\n"
                    "VAR volume;\n"
                    "CONST density = 2400;\n"
                    "BEGIN\n"
                    "volume :=width * height * depth;\n"
                    "RETURN\ndensity * volume\n"
                    "END."};

    const std::vector<TokenType> tokens{
        PARAM, IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR, IDENTIFIER, STATEMENT_TERMINATOR,
        VAR, IDENTIFIER, STATEMENT_TERMINATOR,
        CONST, IDENTIFIER, CONST_ASSIGNMENT_OP, LITERAL, STATEMENT_TERMINATOR,
        BEGIN,
        IDENTIFIER, VAR_ASSIGNMENT_OP, IDENTIFIER, MULT_OP, IDENTIFIER, MULT_OP, IDENTIFIER, STATEMENT_TERMINATOR,
        RETURN, IDENTIFIER, MULT_OP, IDENTIFIER,
        END, PROGRAM_TERMINATOR
    };
};

namespace {
    template<class TokenIter>
    void assert_match(lexer& lexer, TokenIter begin, TokenIter end) {
        for(auto next_token = lexer.next(); next_token.Type() != TokenType::EOS; next_token = lexer.next(), ++begin) {
            if constexpr (std::is_same<typename TokenIter::value_type, token>::value) {
                ASSERT_EQ(next_token.Type(), begin->Type());
                ASSERT_EQ(begin->get_code_reference(), next_token.get_code_reference());
            } else {
                ASSERT_EQ(next_token.Type(), *begin);
            }
        }
        ASSERT_EQ(begin, end);
    }
} // namespace

TEST_F(LexerTest, SimpleProgramTest) {
    lexer lexer(code);
    assert_match(lexer, tokens.begin(), tokens.end());
}

TEST_F(LexerTest, IndividualTokenRecognition) {
    // TODO Test each token
}

TEST_F(LexerTest, NoWhitespace) {
    source_code source("BEGINPARAMCONSTdenEND**123a");
    lexer lexer(source);
    std::vector<token> expected = {
        token(BEGIN, {source.begin(), std::next(source.begin(), 5)}),
        token(PARAM, {std::next(source.begin(), 5), std::next(source.begin(), 10)}),
        token(CONST, {std::next(source.begin(), 10), std::next(source.begin(), 15)}),
        token(IDENTIFIER, {std::next(source.begin(), 15), std::next(source.begin(), 21)}),
        token(MULT_OP, {std::next(source.begin(), 21), std::next(source.begin(), 22)}),
        token(MULT_OP, {std::next(source.begin(), 22), std::next(source.begin(), 23)}),
        token(LITERAL, {std::next(source.begin(), 23), std::next(source.begin(), 26)}),
        token(IDENTIFIER, {std::next(source.begin(), 26), std::next(source.begin(), 27)}),
    };
    assert_match(lexer, expected.begin(), expected.end());
}