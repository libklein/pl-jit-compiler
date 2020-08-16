#include "ExecutionContext.hpp"

int64_t pljit::execution::ExecutionContext::get_value(unsigned int variable_id) const {
    return symbols[variable_id];
}

void pljit::execution::ExecutionContext::set_value(unsigned int variable_id, int64_t value) {
    symbols[variable_id] = value;
}
void pljit::execution::ExecutionContext::set_result(std::optional<int64_t> res) {
    result = res;
}
std::optional<int64_t> pljit::execution::ExecutionContext::get_result() const {
    return result;
}
pljit::execution::ExecutionContext::operator bool() const {
    return result.has_value();
}
