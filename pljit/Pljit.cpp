
#include "Pljit.hpp"
#include "pljit/semantic_analysis/AST.hpp"
#include "pljit/lexer/lexer.hpp"
#include "pljit/parser/parser.hpp"
#include "pljit/semantic_analysis/ast_creation_visitor.hpp"
#include "Executor.hpp"

using namespace pljit;

std::optional<int64_t> Function::call_impl(std::initializer_list<int64_t> parameters) {
    if(!ast && !compilation_failed) {
        // Compile function first
        compile();
    }
    if(compilation_failed) return std::nullopt;
    std::vector<int64_t> param(parameters);
    return Executor::Execute(*ast, symbol_table, param);
}

void Function::compile() {
    std::unique_lock compile_lock{compilation_mutex};
    if(ast) return;
#ifndef NDEBUG
    compilation_passed++;
#endif
    pljit::lexer::lexer lexer (source_code);
    pljit::parser::parser parser(lexer);
    auto parse_tree = parser.parse_function_definition();

    if(!parse_tree) {
        compilation_failed = true;
        return;
    }

    std::tie(ast, symbol_table)= pljit::semantic_analysis::ast_creation_visitor::AnalyzeParseTree(*parse_tree);

    if(!ast) compilation_failed = true;

#ifndef NDEBUG
    if(compilation_passed > 1) {
        throw std::runtime_error("Function was compiled several times");
    };
#endif
}

Function::Function(std::string source) : source_code(std::move(source)) {

}

Function::~Function() = default;

function_handle Pljit::register_function(std::string source) {
    // TODO Thread safe
    registered_functions.emplace_back(std::make_unique<Function>(std::move(source)));
    return function_handle(this, registered_functions.size() - 1);
}
