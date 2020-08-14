
#ifndef PLJIT_SYMBOL_TABLE_HPP
#define PLJIT_SYMBOL_TABLE_HPP

#include "pljit/source_management/source_code.hpp"
#include <string_view>

namespace pljit::semantic_analysis {

    struct symbol {
        enum symbol_type {
            CONSTANT,
            VARIABLE
        };

        source_management::source_code::SourceFragment declaration;
        symbol_type type;
        std::size_t id;
        bool initialized;
    };

    class symbol_table {
        std::vector<symbol> symbols;
        //std::unordered_map<std::string_view, symbol> identifier_mapping;

        public:
        using symbol_handle = std::vector<symbol>::size_type;
        symbol_handle insert(std::string_view identifier, source_management::source_code::SourceFragment decl, symbol::symbol_type type);
        symbol& get(symbol_handle handle);
        const symbol& get(symbol_handle handle) const;

        auto begin() {
            return symbols.begin();
        }

        auto end() {
            return symbols.end();
        }
    };

} // namespace pljit::semantic_analysis

#endif //PLJIT_SYMBOL_TABLE_HPP
