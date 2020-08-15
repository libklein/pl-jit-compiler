
#ifndef PLJIT_SYMBOL_TABLE_HPP
#define PLJIT_SYMBOL_TABLE_HPP

#include "pljit/source_management/source_code.hpp"
#include <string_view>

namespace pljit::semantic_analysis {

    struct symbol {
        enum symbol_type {
            CONSTANT,
            PARAMETER,
            VARIABLE
        };

        source_management::source_code::SourceFragment declaration;
        symbol_type type;
        std::size_t id; // TODO nessesary?
        bool initialized;
        int64_t constant_value;

        std::string_view get_name() const;

        int64_t get_value() const;
    };

    /**
     * Keeps track of symbols. Ordered: First all Params, then all Variables and then all constants
     */
    class symbol_table {
        public:
        using size_type = std::vector<symbol>::size_type;

        private:
        std::vector<symbol> symbols;
        size_type number_of_variables = 0;
        size_type number_of_parameters = 0;
        size_type number_of_constants = 0;
        //std::unordered_map<std::string_view, symbol> identifier_mapping;

        public:
        using symbol_handle = std::vector<symbol>::size_type;
        symbol_handle insert(source_management::source_code::SourceFragment decl, symbol::symbol_type type, std::optional<int64_t> initial_value);
        std::optional<symbol_handle> find(std::string_view name) const;
        symbol& get(symbol_handle handle);
        const symbol& get(symbol_handle handle) const;

        size_type size() const;

        size_type get_number_of_parameters() const;
        size_type get_number_of_variables() const;
        size_type get_number_of_constants() const;

        auto begin() {
            return symbols.begin();
        }

        auto end() {
            return symbols.end();
        }

        auto constants_begin() const {
            return std::next(symbols.begin(), get_number_of_parameters() + get_number_of_variables());
        }

        auto constants_end() const {
            return std::next(symbols.begin(), get_number_of_parameters() + get_number_of_variables() + get_number_of_constants());
        }
    };

} // namespace pljit::semantic_analysis

#endif //PLJIT_SYMBOL_TABLE_HPP
