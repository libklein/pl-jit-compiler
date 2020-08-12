//
// Created by patrick on 7/26/20.
//

#ifndef PLJIT_LEXER_HPP
#define PLJIT_LEXER_HPP

#include "pljit/source_management/source_code.hpp"
#include "token.hpp"

namespace pljit::lexer {

class lexer {
    pljit::source_management::SourcePosition input_iter;
    pljit::source_management::SourcePosition eos;

    void discardWhitespace();

    /***
     * Read a fixed length string from the input stream
     * @return Range in the input
     */
    source_management::SourceFragment read_keyword(std::string_view expected_content) {
        source_management::SourcePosition start = input_iter;
        for(auto next_expected_character = expected_content.begin();
             next_expected_character != expected_content.end();
             ++next_expected_character, ++input_iter) {
            if(*next_expected_character != *input_iter) {
                std::cout << "Expected " << expected_content << " but got \n";
                std::cout << source_management::SourceFragment(start, std::next(input_iter)) << "\n";
                return source_management::SourceFragment{start};
            }
        }
        return {start, input_iter};
    }

    /***
     * Read predicate
     * @return Range in the input
     */
    template <class F>
    source_management::SourceFragment read_until(F predicate) {
        source_management::SourcePosition start = input_iter;
        for(;predicate(*input_iter);++input_iter) {};
        return {start, input_iter};
    }

    public:
    explicit lexer(const source_management::source_code& code) : input_iter(code.begin()), eos(code.end()) {};
    /// Consumes next token
    auto next() -> token;
};

} // namespace pljit::lexer

#endif //PLJIT_LEXER_HPP
