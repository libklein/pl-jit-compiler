#include "symbol_table.hpp"

using namespace pljit::semantic_analysis;

auto symbol_table::insert(std::string_view identifier, pljit::source_management::SourceFragment decl, symbol::symbol_type type) -> symbol_handle {
    auto id = symbols.size();
    symbols.push_back({decl, type, id, type != symbol::VARIABLE});
    return id;
}

symbol& symbol_table::get(symbol_table::symbol_handle handle) {
    return symbols[handle];
}

const symbol& symbol_table::get(symbol_table::symbol_handle handle) const {
    return symbols[handle];
}
