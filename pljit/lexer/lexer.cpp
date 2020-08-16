//
// Created by patrick on 7/26/20.
//

#include "lexer.hpp"
#include <cctype>
#include <optional>

namespace pljit::lexer {

auto lexer::next() -> std::optional<token> {
    discardWhitespace();
    if (input_iter == eos) {
        return std::optional<token>{std::in_place, TokenType::EOS, source_management::SourceFragment{eos, eos}};
    } else if (!is_valid_symbol(*input_iter)) {
        return std::nullopt;
    }

    if (std::optional<token> singleton_token = parse_singleton(); singleton_token) return singleton_token;
    // Special case - :=
    if (*input_iter == ':') {
        auto begin_pos = input_iter;
        ++input_iter;
        // TODO Refactor?
        if (*input_iter != '=') {
            input_iter = begin_pos; // Reset iterator
            return std::nullopt;
        }
        ++input_iter;
        return std::optional<token>{std::in_place, TokenType::VAR_ASSIGNMENT_OP,
                                    source_management::SourceFragment{begin_pos, input_iter}};
    }

    if (std::isdigit(*input_iter)) {
        // Has to be a literal
        return std::optional<token>{std::in_place, TokenType::LITERAL, read_until([](char c) { return std::isdigit(c); })};
    }

    source_management::SourceFragment source_pos(read_until([](char c) { return std::isalpha(c); }));
    // If the token under construction matches a known keyword, return it.
    if (std::optional<token> keyword_token = parse_keyword(source_pos); keyword_token) return keyword_token;

    return std::optional<token>{std::in_place, TokenType::IDENTIFIER, source_pos};
}
void lexer::discardWhitespace() {
    for (; input_iter != eos && (*input_iter == ' ' || *input_iter == '\t' || *input_iter == '\n'); ++input_iter) {}
}

auto lexer::parse_singleton() -> std::optional<token> {
    auto source_begin = input_iter;
    TokenType type;

    switch (*input_iter) {
        case '.':
            ++input_iter;
            type = TokenType::PROGRAM_TERMINATOR;
            break;
        case ';':
            ++input_iter;
            type = TokenType::STATEMENT_TERMINATOR;
            break;
        case ',':
            ++input_iter;
            type = TokenType::SEPARATOR;
            break;
        case '=':
            ++input_iter;
            type = TokenType::INIT_ASSIGNMENT_OP;
            break;
        case '+':
            ++input_iter;
            type = TokenType::PLUS_OP;
            break;
        case '-':
            ++input_iter;
            type = TokenType::MINUS_OP;
            break;
        case '*':
            ++input_iter;
            type = TokenType::MULT_OP;
            break;
        case '/':
            ++input_iter;
            type = TokenType::DIV_OP;
            break;
        case '(':
            ++input_iter;
            type = TokenType::L_BRACKET;
            break;
        case ')':
            ++input_iter;
            type = TokenType::R_BRACKET;
            break;
        default: {
            return std::nullopt;
        }
    }

    return std::optional<token>{
        std::in_place, type, source_management::SourceFragment{source_begin, input_iter}};
}

auto lexer::parse_keyword(source_management::SourceFragment fragment) -> std::optional<token> {
    TokenType type = TokenType::EOS;
    switch (fragment.size()) {
        case 6: {
            if (fragment.str() == "RETURN") {
                type = RETURN;
            }
            break;
        }
        case 5: {
            if (fragment.str() == "PARAM") {
                type = PARAM;
            } else if (fragment.str() == "CONST") {
                type = CONST;
            } else if (fragment.str() == "BEGIN") {
                type = BEGIN;
            }
            break;
        }
        case 3: {
            if (fragment.str() == "VAR") {
                type = VAR;
            } else if (fragment.str() == "END") {
                type = END;
            }
            break;
        }
        default: {
            break;
        }
    }
    return type == EOS ? std::nullopt : std::optional<token>{std::in_place, type, fragment};
}

pljit::source_management::SourcePosition lexer::get_current_position() const {
    return input_iter;
}
bool lexer::is_valid_symbol(char c) {
    return std::isalnum(c) || c == '.' || c == ';' || c == ',' || c == '=' || c == ':' || c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')';
}

} // namespace pljit::lexer