//
// Created by patrick on 7/26/20.
//

#ifndef PLJIT_LEXER_HPP
#define PLJIT_LEXER_HPP

#include "pljit/source_management/SourceCode.hpp"
#include "token.hpp"

namespace pljit::lexer {

class lexer {
    pljit::source_management::SourcePosition input_iter;
    pljit::source_management::SourcePosition eos;

    void discardWhitespace();

    /***
     * Read predicate
     * @return Range in the input
     */
    template <class F>
    source_management::SourceFragment read_until(F&& predicate) {
        source_management::SourcePosition start = input_iter;
        for (; is_valid_symbol(*input_iter) && predicate(*input_iter); ++input_iter) {};
        return {start, input_iter};
    }

    std::optional<token> parse_singleton();

    static bool is_valid_symbol(char c);

    static std::optional<token> parse_keyword(source_management::SourceFragment fragment);

    public:
    explicit lexer(const source_management::SourceCode& code) : input_iter(code.begin()), eos(code.end()){};
    /// Consumes next token
    auto next() -> std::optional<token>;
    source_management::SourcePosition get_current_position() const;
};

} // namespace pljit::lexer

#endif //PLJIT_LEXER_HPP
