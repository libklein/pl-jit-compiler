
#include "pljit/lexer/lexer.hpp"
#include "pljit/source_management/SourceCode.hpp"
#include <array>
#include <gtest/gtest.h>

using namespace pljit::source_management;
using namespace pljit::lexer;

class LexerTest : public ::testing::Test {
    protected:
    const SourceCode code{"PARAM width, height, depth;\n"
                    "VAR volume;\n"
                    "CONST density = 2400;\n"
                    "BEGIN\n"
                    "volume :=width * height * depth;\n"
                    "RETURN\ndensity * volume\n"
                    "END."};

    const std::vector<TokenType> tokens{
        PARAM, IDENTIFIER, SEPARATOR, IDENTIFIER, SEPARATOR, IDENTIFIER, STATEMENT_TERMINATOR,
        VAR, IDENTIFIER, STATEMENT_TERMINATOR,
        CONST, IDENTIFIER, INIT_ASSIGNMENT_OP, LITERAL, STATEMENT_TERMINATOR,
        BEGIN,
        IDENTIFIER, VAR_ASSIGNMENT_OP, IDENTIFIER, MULT_OP, IDENTIFIER, MULT_OP, IDENTIFIER, STATEMENT_TERMINATOR,
        RETURN, IDENTIFIER, MULT_OP, IDENTIFIER,
        END, PROGRAM_TERMINATOR
    };

    const std::vector<std::pair<TokenType, std::string_view>> token_repr_map{
        {PROGRAM_TERMINATOR, "."},
        {STATEMENT_TERMINATOR, ";"},
        {SEPARATOR, ","},
        {PARAM, "PARAM"},
        {VAR, "VAR"},
        {CONST, "CONST"},
        {INIT_ASSIGNMENT_OP, "="},
        {BEGIN, "BEGIN"},
        {END, "END"},
        {RETURN, "RETURN"},
        {VAR_ASSIGNMENT_OP, ":="},
        {PLUS_OP, "+"},
        {MINUS_OP, "-"},
        {MULT_OP, "*"},
        {DIV_OP, "/"},
        {L_BRACKET, "("},
        {R_BRACKET, ")"},
        {LITERAL, "123632"},
        {IDENTIFIER, "cmsPARAMadk"}
    };
};

namespace {
    template<class TokenIter>
    void assert_match(lexer& lexer, TokenIter begin, TokenIter end) {
        for(auto next_token = lexer.next(); next_token->Type() != TokenType::EOS; next_token = lexer.next(), ++begin) {
            ASSERT_TRUE(next_token.has_value());
            ASSERT_NE(next_token->Type(), TokenType::EOS);
            if constexpr (std::is_same<typename TokenIter::value_type, token>::value) {
                ASSERT_EQ(next_token->Type(), begin->Type());
                ASSERT_EQ(begin->get_code_reference(), next_token->get_code_reference());
            } else {
                ASSERT_EQ(next_token->Type(), *begin);
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
    for(auto [expected_token_type, test_string] : token_repr_map) {
        SourceCode code(test_string.data());
        lexer l(code);
        auto token = l.next();
        EXPECT_TRUE(token.has_value());
        EXPECT_EQ(token->Type(), expected_token_type);
        EXPECT_EQ(token->get_code_reference(), SourceFragment(code.begin(), std::next(code.begin(), test_string.size())));
        EXPECT_EQ(token->get_code_reference().str(), test_string);
    }
}

TEST_F(LexerTest, InvalidSymbol) {
    std::vector<char> invalid_symbols{'?', '!', '{', '}', '\r', '\0', '\4', ':'};
    // Invalid symbol in keyword
    std::string template_code("BEGIN someidenti");
    for(char invalid_char : invalid_symbols) {
        // Invalid symbol at begin
        {
            SourceCode code(invalid_char+std::string("sas"));
            lexer l(code);
            auto next_token = l.next();
            EXPECT_FALSE(next_token);
            EXPECT_EQ(l.get_current_position(), code.begin());
        }
        // Invalid symbol wrapped by whitespace
        {
            SourceCode code(std::string(" \t\n") + invalid_char + std::string("   "));
            lexer l(code);
            auto next_token = l.next();
            EXPECT_FALSE(next_token);
            EXPECT_EQ(l.get_current_position(), std::next(code.begin(), 3));
        }
        // Invalid symbol in a keyword
        {
            SourceCode code(template_code + invalid_char + "sda;");
            lexer l(code);
            EXPECT_EQ(l.next()->Type(), BEGIN);
            // "somidenti" should be parsed correctly.
            auto next_token = l.next();
            EXPECT_EQ(next_token->Type(), IDENTIFIER);
            EXPECT_EQ(next_token->get_code_reference().str(), "someidenti");
            EXPECT_FALSE(l.next().has_value());
            EXPECT_EQ(l.get_current_position(), std::next(code.begin(), template_code.size()));
        }
    }
}

TEST_F(LexerTest, NoWhitespace) {
    SourceCode source("BEGINPARAM CONST,denEND**123a");
    lexer lexer(source);
    std::vector<token> expected = {
        token(IDENTIFIER, {source.begin(), std::next(source.begin(), 10)}),
        token(CONST, {std::next(source.begin(), 11), std::next(source.begin(), 16)}),
        token(SEPARATOR, {std::next(source.begin(), 16), std::next(source.begin(), 17)}),
        token(IDENTIFIER, {std::next(source.begin(), 17), std::next(source.begin(), 23)}),
        token(MULT_OP, {std::next(source.begin(), 23), std::next(source.begin(), 24)}),
        token(MULT_OP, {std::next(source.begin(), 24), std::next(source.begin(), 25)}),
        token(LITERAL, {std::next(source.begin(), 25), std::next(source.begin(), 28)}),
        token(IDENTIFIER, {std::next(source.begin(), 28), std::next(source.begin(), 29)})
    };
    assert_match(lexer, expected.begin(), expected.end());
}