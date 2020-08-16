#include <gtest/gtest.h>
#include <thread>

#include "pljit/Pljit.hpp"

using namespace pljit;

TEST(InterfaceTest, SingleThread) {
    pljit::Pljit compiler;

    auto handle = compiler.register_function("PARAM a, b;BEGIN RETURN a + b END.");
    auto result = handle(1, 2);
    ASSERT_TRUE(result);
    EXPECT_EQ(*result.get_result(), 3);
    result = handle(3, 5);
    ASSERT_TRUE(result);
    EXPECT_EQ(*result.get_result(), 8);
}

TEST(InterfaceTest, SingleThreadInvalidProgram) {
    pljit::Pljit compiler;

    // Failure in lexer
    auto handle = compiler.register_function("PAR?AM a, b;BEGIN; RETURN a + b END.");
    auto result = handle(10, 10);
    EXPECT_FALSE(result);

    // Failure in parser
    handle = compiler.register_function("PARAM a, b;BEGIN; RETURN a + b END.");
    result = handle(10, 10);
    EXPECT_FALSE(result);

    // Failure in semantic analyzer
    handle = compiler.register_function("PARAM a;BEGIN RETURN a + b END.");
    result = handle(10);
    EXPECT_FALSE(result);
}

TEST(InterfaceTest, MultithreadedCompilation) {
    pljit::Pljit compiler;
    auto handle = compiler.register_function("BEGIN RETURN 10 END.");
    ASSERT_NO_FATAL_FAILURE({
        std::vector<std::thread> thread_pool;
        for (unsigned i = 0; i < 1024; ++i) {
            thread_pool.emplace_back([handle]() mutable {
                auto res = handle();
                ASSERT_TRUE(res);
                EXPECT_EQ(*res.get_result(), 10);
            });
        }

        for (auto& thread : thread_pool) {
            thread.join();
        }
    });
}