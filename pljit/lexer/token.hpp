//
// Created by patrick on 7/26/20.
//

#ifndef PLJIT_TOKEN_HPP
#define PLJIT_TOKEN_HPP

#include "pljit/source_management/SourceCode.hpp"

namespace pljit::lexer {

enum TokenType {
    PROGRAM_TERMINATOR,
    STATEMENT_TERMINATOR,
    SEPARATOR,
    PARAM,
    VAR,
    CONST,
    INIT_ASSIGNMENT_OP,
    BEGIN,
    END,
    RETURN,
    VAR_ASSIGNMENT_OP,
    PLUS_OP,
    MINUS_OP,
    MULT_OP,
    DIV_OP,
    L_BRACKET,
    R_BRACKET,
    LITERAL,
    IDENTIFIER,
    EOS
};

class token {
    using SourceFragment = source_management::SourceFragment;
    TokenType type;
    SourceFragment source;

    public:
    token(TokenType type, SourceFragment source);

    TokenType Type() const;

    SourceFragment get_code_reference() const;
};

} // namespace pljit::lexer

#endif //PLJIT_TOKEN_HPP
