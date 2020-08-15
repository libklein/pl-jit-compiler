
#ifndef PLJIT_EXECUTIONCONTEXT_HPP
#define PLJIT_EXECUTIONCONTEXT_HPP

#include <cstdint>
#include <optional>
#include <vector>
#include "pljit/semantic_analysis/symbol_table.hpp"

namespace pljit::semantic_analysis {
    class symbol_table;
} // namespace pljit::semantic_analysis

namespace pljit::execution {

class ExecutionContext {
    // TODO Keep track of variables, parameters and constants
    std::vector<int64_t> symbols;
    std::optional<int64_t> result;

    public:
    template <class... Args>
    explicit ExecutionContext(const pljit::semantic_analysis::symbol_table& symbolTable, Args... param) : symbols(0), result{} {
        static_assert(std::conjunction_v<std::is_convertible<Args, int64_t>...>, "Parameters must be integers!");
        assert(symbolTable.get_number_of_parameters() == sizeof...(param));
        symbols.reserve(symbolTable.size()); // Avoid reallocation
        // Add parameters
        (symbols.push_back(param), ...);
        symbols.resize(symbolTable.size());

        for(auto next_constant = symbolTable.constants_begin(); next_constant != symbolTable.constants_end(); ++next_constant) {
            assert(next_constant->initialized);
            symbols[next_constant->id] = next_constant->get_value();
        }
    }

    void set_value(unsigned variable_id, int64_t value);
    int64_t get_value(unsigned variable_id) const;
    std::optional<int64_t> get_result() const;
    void set_result(std::optional<int64_t>);
};

} // namespace pljit::execution

#endif //PLJIT_EXECUTIONCONTEXT_HPP
