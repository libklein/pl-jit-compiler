#include "symbol_table.hpp"

namespace pljit::semantic_analysis {

auto symbol_table::insert(pljit::source_management::SourceFragment decl, symbol::symbol_type type, std::optional<int64_t> value) -> symbol_handle {
    assert(type != symbol::CONSTANT || value);
    auto id = symbols.size();
    symbols.push_back({decl, type, id, type != symbol::VARIABLE, value ? *value : 0});
    switch (type) {
        case symbol::CONSTANT: ++number_of_constants; break;
        case symbol::PARAMETER: ++number_of_parameters; break;
        case symbol::VARIABLE: ++number_of_variables; break;
    }
    return id;
}

symbol& symbol_table::get(symbol_table::symbol_handle handle) {
    return symbols[handle];
}

const symbol& symbol_table::get(symbol_table::symbol_handle handle) const {
    return symbols[handle];
}
symbol_table::size_type symbol_table::size() const {
    return symbols.size();
}

auto symbol_table::find(std::string_view name) const -> std::optional<symbol_handle> {
    if (auto symbol_iter = std::find_if(symbols.begin(), symbols.end(), [name](const symbol& s) { return s.get_name() == name; });
        symbol_iter != symbols.end()) {
        return symbol_iter->id;
    }
    return std::nullopt;
}

auto symbol_table::get_number_of_parameters() const -> size_type {
    return number_of_parameters;
}

auto symbol_table::get_number_of_variables() const -> size_type {
    return number_of_variables;
}

auto symbol_table::get_number_of_constants() const -> size_type {
    return number_of_constants;
}
auto symbol_table::constants_begin() const -> std::vector<symbol>::const_iterator {
    return std::next(symbols.begin(), get_number_of_parameters() + get_number_of_variables());
}
auto symbol_table::constants_end() const -> std::vector<symbol>::const_iterator {
    return std::next(symbols.begin(), get_number_of_parameters() + get_number_of_variables() + get_number_of_constants());
}
auto symbol_table::end() -> std::vector<symbol>::iterator {
    return symbols.end();
}
auto symbol_table::begin() -> std::vector<symbol>::iterator {
    return symbols.begin();
}

int64_t symbol::get_value() const {
    assert(type == CONSTANT);
    return constant_value;
}
std::string_view symbol::get_name() const {
    return declaration.str();
}
void symbol::set_initialized() {
    initialized = true;
}

} // namespace pljit::semantic_analysis