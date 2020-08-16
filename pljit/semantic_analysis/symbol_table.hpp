
#ifndef PLJIT_SYMBOL_TABLE_HPP
#define PLJIT_SYMBOL_TABLE_HPP

#include <string_view>
#include "pljit/source_management/SourceCode.hpp"

namespace pljit::semantic_analysis {

struct symbol {
    enum symbol_type {
        CONSTANT,
        PARAMETER,
        VARIABLE
    };

    source_management::SourceFragment declaration;
    symbol_type type;
    std::size_t id;
    bool initialized;
    int64_t constant_value;

    std::string_view get_name() const;
    int64_t get_value() const;

    void set_initialized();
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

    public:
    using symbol_handle = std::vector<symbol>::size_type;
    symbol_handle insert(source_management::SourceFragment decl, symbol::symbol_type type, std::optional<int64_t> initial_value);
    std::optional<symbol_handle> find(std::string_view name) const;
    symbol& get(symbol_handle handle);
    const symbol& get(symbol_handle handle) const;

    size_type size() const;

    size_type get_number_of_parameters() const;
    size_type get_number_of_variables() const;
    size_type get_number_of_constants() const;

    std::vector<symbol>::iterator begin();

    std::vector<symbol>::iterator end();

    std::vector<symbol>::const_iterator constants_begin() const;

    std::vector<symbol>::const_iterator constants_end() const;
};

} // namespace pljit::semantic_analysis

#endif //PLJIT_SYMBOL_TABLE_HPP
