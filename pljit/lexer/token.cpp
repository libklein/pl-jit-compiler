//
// Created by patrick on 7/26/20.
//

#include "token.hpp"

namespace pljit::lexer {

token::token(TokenType type, token::SourceFragment source) : type(type), source(source){}

TokenType token::Type() const {
    return type;
}

token::SourceFragment token::get_code_reference() const {
    return source;
}

} // namespace pljit::lexer