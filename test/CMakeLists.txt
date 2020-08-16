set(TEST_SOURCES
    # add your *.cpp files here
    Tester.cpp
    source_management/TestSourceManagement.cpp
    lexer/TestLexer.cpp
    parser/TestParser.cpp
    semantic_analysis/TestSemanticAnalysis.cpp
    TestExecutor.cpp
    TestInterface.cpp
    TestOptimization.cpp)

add_executable(tester ${TEST_SOURCES})
target_link_libraries(tester PUBLIC
    pljit_core
    GTest::GTest)

target_include_directories(tester PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
