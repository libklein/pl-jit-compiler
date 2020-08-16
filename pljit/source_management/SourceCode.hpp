//
// Created by patrick on 7/24/20.
//

#ifndef PLJIT_SOURCECODE_HPP
#define PLJIT_SOURCECODE_HPP

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>

namespace pljit::source_management {

class SourceFragment;
class SourceCode;

class SourcePosition {
    friend SourceFragment;
    using size_t = std::size_t;

    public:
    using offset_t = size_t;

    // TODO Fix pointer and reference type
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = char;
    using difference_type = std::ptrdiff_t;
    using pointer = char;
    using reference = char;

    private:
    const SourceCode* source;

    size_t line;
    size_t cursor;

    std::ostream& output_to_stream(std::ostream& os) const;

    public:
    SourcePosition(const SourceCode* source, size_t line, size_t cursor) noexcept;

    size_t get_line() const;

    size_t get_cursor() const;

    char operator*();

    SourcePosition& operator++();

    SourcePosition& operator--();

    SourcePosition operator++(int);

    SourcePosition operator--(int);

    bool operator==(const SourcePosition& rhs) const;

    bool operator!=(const SourcePosition& rhs) const;

    bool operator<(const SourcePosition& rhs) const;

    bool operator>(const SourcePosition& rhs) const;
    bool operator<=(const SourcePosition& rhs) const;
    bool operator>=(const SourcePosition& rhs) const;

    friend std::ostream& operator<<(std::ostream& os, const SourcePosition& position);
};

class SourceFragment {
    SourcePosition begin_pos;
    SourcePosition end_pos;

    std::ostream& output_to_stream(std::ostream& os) const {
        if(begin_pos == end_pos) return os;
        // TODO Does not work for multi line segments. But is that even required?
        os << begin_pos;
        std::fill_n(std::ostream_iterator<std::ostream::char_type>(os), std::max(static_cast<int>(end_pos.cursor - begin_pos.cursor) - 1, 0), '~');
        return os;
    }

    public:
    // TODO REMOVE THIS
    SourceFragment() : begin_pos(nullptr, 0, 0), end_pos(nullptr, 0, 0) {}
    explicit SourceFragment(SourcePosition begin);
    SourceFragment(SourcePosition begin, SourcePosition end);

    void extend(SourceFragment &other);

    auto begin() const;

    auto end() const;

    std::size_t size() const;

    std::string_view str() const;

    bool operator==(const SourceFragment& rhs) const;

    bool operator!=(const SourceFragment& rhs) const;

    friend std::ostream& operator<<(std::ostream& os, const SourceFragment& fragment);
};

/***
 *
 */
class SourceCode {
    friend SourcePosition;
    friend SourceFragment;
    public:
    using offset_t = SourcePosition::offset_t;

    private:
    std::string code;
    std::vector<SourcePosition::offset_t> lines;

    public:
    SourceCode() = default;
    explicit SourceCode(std::string source_code);
    explicit SourceCode(const char* code);
    explicit SourceCode(std::string_view code);

    SourcePosition begin() const;

    SourcePosition end() const;

    SourcePosition::offset_t number_of_lines() const;

    SourcePosition::offset_t line_length(SourcePosition::offset_t line) const;

    private:
    SourcePosition::offset_t get_line_offset(offset_t line) const;

    /// Returns the character in line line at position cursor
    char get(offset_t line, offset_t cursor) const;

    /// Get substring
    std::string_view get(offset_t from_line, offset_t from_cursor, offset_t to_line, offset_t to_cursor) const;

    /// Returns the line
    std::string_view get(offset_t line) const;

};

/*using SourcePosition = source_code::SourcePosition;
using SourceFragment = source_code::SourceFragment;*/

} // namespace pljit::source_management

#endif //PLJIT_SOURCECODE_HPP
