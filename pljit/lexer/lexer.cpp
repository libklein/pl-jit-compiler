//
// Created by patrick on 7/26/20.
//

#include <cctype>

#include "lexer.hpp"

auto pljit::lexer::lexer::next() -> pljit::lexer::token {
    discardWhitespace();
    if(input_iter == eos) {
        return token(TokenType::EOS, {eos, eos});
    }
    // TODO figure out
    source_management::SourceFragment source_pos(input_iter);
    switch (*input_iter) {
        case '.':
            source_pos = read_keyword(".");
            // TODO Exit / set done flag
            return token(TokenType::PROGRAM_TERMINATOR, source_pos);
        case ';':
            source_pos = read_keyword(";");
            return token(TokenType::STATEMENT_TERMINATOR, source_pos);
        case ',':
            source_pos = read_keyword(",");
            return token(TokenType::SEPARATOR, source_pos);
        case 'P':
            source_pos = read_keyword("PARAM");
            return token(TokenType::PARAM, source_pos);
        case 'V':
            source_pos = read_keyword("VAR");
            return token(TokenType::VAR, source_pos);
        case 'C':
            source_pos = read_keyword("CONST");
            return token(TokenType::CONST, source_pos);
        case '=':
            source_pos = read_keyword("=");
            return token(TokenType::CONST_ASSIGNMENT_OP, source_pos);
        case 'B':
            source_pos = read_keyword("BEGIN");
            return token(TokenType::BEGIN, source_pos);
        case 'E':
            source_pos = read_keyword("END");
            return token(TokenType::END, source_pos);
        case 'R':
            source_pos = read_keyword("RETURN");
            return token(TokenType::RETURN, source_pos);
        case ':':
            source_pos = read_keyword(":=");
            return token(TokenType::VAR_ASSIGNMENT_OP, source_pos);
        case '+':
            source_pos = read_keyword("+");
            return token(TokenType::PLUS_OP, source_pos);
        case '-':
            source_pos = read_keyword("-");
            return token(TokenType::MINUS_OP, source_pos);
        case '*':
            source_pos = read_keyword("*");
            return token(TokenType::MULT_OP, source_pos);
        case '/':
            source_pos = read_keyword("/");
            return token(TokenType::DIV_OP, source_pos);
        case '(':
            source_pos = read_keyword("(");
            return token(TokenType::L_BRACKET, source_pos);
        case ')':
            source_pos = read_keyword(")");
            return token(TokenType::R_BRACKET, source_pos);
        default: {}
    }

    if (std::isalpha(*input_iter)) {
        source_pos = read_until([](char c){ return std::isalpha(c); });
        return token(TokenType::IDENTIFIER, source_pos);
    } else if(std::isdigit(*input_iter)) {
        source_pos = read_until([](char c){ return std::isdigit(c); });
        return token(TokenType::LITERAL, source_pos);
    } else {
        // Error token?
        throw std::logic_error("Not found!");
    }
}
void pljit::lexer::lexer::discardWhitespace() {
    // TODO isspace correct? Task says spaces, tabs and newlines
    for(;input_iter != eos && std::isspace(*input_iter);++input_iter) {}
}
