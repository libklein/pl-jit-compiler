set(PLJIT_SOURCES
    source_management/SourceCode.cpp
    lexer/token.cpp
    lexer/lexer.cpp
    parser/parse_tree_nodes.cpp
    parser/parser.cpp
    parser/dot_print_visitor.cpp
    semantic_analysis/AST.cpp
    semantic_analysis/ASTCreator.cpp
    semantic_analysis/symbol_table.cpp
    semantic_analysis/dot_print_visitor.cpp
    optimization/optimization_pass.cpp
    optimization/passes/dead_code_elimination.cpp
    optimization/passes/constant_propagation.cpp
    optimization/passes/UnaryPlusRemoval.cpp
    Pljit.cpp
    execution/ExecutionContext.cpp)

add_library(pljit_core ${PLJIT_SOURCES})
target_include_directories(pljit_core PUBLIC ${CMAKE_SOURCE_DIR})

add_clang_tidy_target(lint_pljit_core ${PLJIT_SOURCES})
add_dependencies(lint lint_pljit_core)

add_executable(pljit main.cpp)
target_link_libraries(pljit PUBLIC pljit_core)
