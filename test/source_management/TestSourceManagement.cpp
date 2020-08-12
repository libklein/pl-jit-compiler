#include "pljit/source_management/source_code.hpp"
#include <numeric>
#include <gtest/gtest.h>

using namespace pljit::source_management;

namespace {
template<class Iter1, class Iter2>
    bool range_equals(Iter1 lhs, Iter1 lhs_end, Iter2 rhs, Iter2 rhs_end) {
        //std::cout << "r1: " << std::distance(lhs, lhs_end) << " r2: " << std::distance(rhs, rhs_end) << std::endl;
        for (;lhs != lhs_end && rhs != rhs_end;
             ++lhs, ++rhs) {
            if (*lhs != *rhs) {
                return false;
            }
        }
        return lhs == lhs_end && rhs == rhs_end;
    }

    template<class T>
    std::string capture_output(const T& t) {
        std::stringstream ss;
        ss << t;
        return ss.str();
    }
} // namespace

TEST(SourceManagement, EmptyString) {
    source_code code(std::string(""));
    ASSERT_EQ(code.number_of_lines(), 0);
}

TEST(SourceManagement, Iteration) {
    std::vector<std::string> lines{
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n",
        "Donec commodo auctor arcu. Cras imperdiet nec eros at scelerisque. Praesent luctus bibendum orci,\n",
        "et varius erat varius at. Pellentesque sit amet vehicula justo, non consectetur ligula.\n",
        "Suspendisse in nisl auctor, rutrum libero eu, auctor ex. Sed faucibus, lectus a gravida facilisis,\n",
        "nisl felis malesuada nisi, non dignissim diam ipsum maximus dolor. Pellentesque pulvinar arcu et\n",
        "magna suscipit, quis posuere augue convallis.\n"
    };

    auto concatenated_code = std::accumulate(lines.begin(), lines.end(), std::string());
    source_code code(concatenated_code);

    // Test line detection/iteration
    ASSERT_EQ(code.number_of_lines(), lines.size());
    unsigned int line_offset = 0;
    for(const auto& line : lines) {
        ASSERT_EQ(line.size(), code.line_length(line_offset));
        ++line_offset;
    }

    // Test char-by-char iteration
    auto source_iter = code.begin();
    unsigned int string_lines = 0;
    unsigned int string_cursor = 0;
    for(auto string_iter = concatenated_code.begin(); string_iter != concatenated_code.end() && source_iter != code.end();
         ++string_iter, ++source_iter) {
        ASSERT_EQ(*string_iter, *source_iter);
        ASSERT_EQ(string_lines, source_iter.get_line());
        ASSERT_EQ(string_cursor, source_iter.get_cursor());
        if(*string_iter == '\n') {
            string_lines++;
            string_cursor = 0;
        } else {
            ++string_cursor;
        }
    }
    ASSERT_EQ(source_iter, code.end());

    // TODO Reverse iteration
}

TEST(SourceManagement, SingleLineWithoutNewline) {
    std::string code_string("Lorem ipsum dolor sit amet, consectetur adipiscing elit.");
    source_code code(code_string);
    ASSERT_EQ(code.number_of_lines(), 1);
    ASSERT_EQ(code.line_length(0), code_string.size() + 1);
    ASSERT_TRUE(range_equals(code.begin(), std::prev(code.end()), code_string.begin(), code_string.end()));
}

TEST(SourceManagement, EmptyLine) {
    std::string code_string("Lorem ipsum dolor sit amet, consectetur adipiscing elit.\n\nTest\n");
    source_code code(code_string);
    ASSERT_EQ(code.number_of_lines(), 3);
    ASSERT_EQ(code.line_length(1), 1);
    ASSERT_TRUE(range_equals(code.begin(), code.end(), code_string.begin(), code_string.end()));
}

TEST(SourceManagement, Encoding) {
    // TODO What if code contains special chars? \r for instance? How would pretty printing work?
}

TEST(SourceManagement, EmptyFragment) {
    std::string code_string("Lorem ipsum dolor sit amet, consectetur adipiscing elit.\nAB\nCDE\n");
    source_code code(code_string);
    SourceFragment fragment(code.begin(), code.begin());
    ASSERT_EQ(fragment.str(), "");
}

TEST(SourceManagement, SingleLineFragment) {
    std::string code_string("Lorem ipsum dolor sit amet, consectetur adipiscing elit.\nAB\nCDE\n");
    source_code code(code_string);
    auto fragment_begin = code.begin(), fragment_end = std::next(fragment_begin, 5);
    SourceFragment fragment(fragment_begin, fragment_end);
    ASSERT_EQ(fragment.str(), code_string.substr(0, std::distance(fragment_begin, fragment_end)));
}

TEST(SourceManagement, MultiLineFragment) {
    // TODO Implement MultiLineFragment test
    return;
    std::string code_string("Lorem ipsum dolor sit amet, consectetur adipiscing elit.\nAB\nCDE\n");
    source_code code(code_string);
    auto fragment_begin = code.begin(), fragment_end = code.end();
    SourceFragment fragment(fragment_begin, fragment_end);
    ASSERT_EQ(fragment.str(), code_string.substr(0, std::distance(fragment_begin, fragment_end)));
}

TEST(SourceManagement, PrettyPrinting) {
    std::string code_string("Lorem ipsum dolor sit amet, consectetur adipiscing elit.\nAB\nCDE\n");
    source_code code(code_string);

    {
        SourcePosition pos_line_1 = std::next(code.begin(), 10);
        EXPECT_EQ(capture_output(pos_line_1), "Position 0:10\nLorem ipsum dolor sit amet, consectetur adipiscing elit.\n          ^");
        SourcePosition pos_line_2 = std::next(code.begin(), 58);
        // Only second line should be shown
        EXPECT_EQ(capture_output(pos_line_2), "Position 1:1\nAB\n ^");
        SourcePosition pos_newline = std::next(code.begin(), 56);
        // Error caret should be after the ., i.e. where the newline would be
        EXPECT_EQ(capture_output(pos_newline), "Position 0:56\nLorem ipsum dolor sit amet, consectetur adipiscing elit.\n                                                        ^");
    }
    {
        // Empty fragement should not print anything
        SourceFragment fragment(code.begin(), code.begin());
        EXPECT_EQ(capture_output(fragment), "");
        // Single character fragment should behave just like position
        SourcePosition pos = std::next(code.begin(), 10);
        fragment = SourceFragment(pos, std::next(pos));
        EXPECT_EQ(capture_output(pos), capture_output(fragment));
    }
    {
        auto fragment_begin = code.begin(), fragment_end = std::next(fragment_begin, 5);
        SourceFragment fragment(fragment_begin, fragment_end);
        EXPECT_EQ(capture_output(fragment), "Position 0:0\nLorem ipsum dolor sit amet, consectetur adipiscing elit.\n^~~~~");
    }
}