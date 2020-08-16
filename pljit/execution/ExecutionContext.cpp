#include "ExecutionContext.hpp"

namespace pljit::execution {
int64_t ExecutionContext::get_value(unsigned int variable_id) const {
    return symbols[variable_id];
}

void ExecutionContext::set_value(unsigned int variable_id, int64_t value) {
    symbols[variable_id] = value;
}

void ExecutionContext::set_result(std::optional<int64_t> res) {
    result = res;
}

std::optional<int64_t> ExecutionContext::get_result() const {
    return result;
}

ExecutionContext::operator bool() const {
    return result.has_value();
}

ExecutionContext::ExecutionContext(const semantic_analysis::symbol_table& symbolTable, const std::vector<int64_t>& parameters) {
    symbols.resize(symbolTable.size());
    std::copy(parameters.begin(), parameters.end(), symbols.begin());

    for (auto next_constant = symbolTable.constants_begin(); next_constant != symbolTable.constants_end(); ++next_constant) {
        assert(next_constant->initialized);
        symbols[next_constant->id] = next_constant->get_value();
    }
}

} // namespace pljit::execution