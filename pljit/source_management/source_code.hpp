//
// Created by patrick on 7/24/20.
//

#ifndef PLJIT_SOURCE_CODE_HPP
#define PLJIT_SOURCE_CODE_HPP

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>

namespace pljit::source_management {

/***
 *
 */
class source_code {
    public:
    class SourceFragment;

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
        const source_code* source;

        size_t line;
        size_t cursor;

        std::ostream& output_to_stream(std::ostream& os) const {
            os << "Position " << line << ":" << cursor << '\n';
            os << source->get(line); // Includes newline
            std::fill_n(std::ostream_iterator<char>(os), cursor, ' ');
            os << '^';
            return os;
        }

        public:
        // TODO Behaves like an iterator
        SourcePosition(const source_code* source, size_t line, size_t cursor) noexcept : source(source), line(line), cursor(cursor) {};

        size_t get_line() const {
            return line;
        }

        size_t get_cursor() const {
            return cursor;
        }

        char operator*() {
            return source->get(line, cursor);
        };

        SourcePosition& operator++() {
            ++cursor;
            // TODO Perhaps check against \n?
            if(cursor == source->line_length(line)) {
                ++line;
                cursor = 0;
            }
            return *this;
        }

        SourcePosition& operator--() {
            if(cursor == 0) {
                --line;
                cursor = source->line_length(line) - 1;
            } else {
                --cursor;
            }
            return *this;
        }

        SourcePosition operator++(int) {
            SourcePosition other = *this;
            ++(*this);
            return other;
        }

        SourcePosition operator--(int) {
            SourcePosition other = *this;
            --(*this);
            return other;
        }

        bool operator==(const SourcePosition& rhs) const {
            return line == rhs.line && cursor == rhs.cursor;
        }

        bool operator!=(const SourcePosition& rhs) const {
            return !(rhs == *this);
        }

        bool operator<(const SourcePosition& rhs) const {
            return (line < rhs.line) || (line >= rhs.line && cursor < rhs.cursor);
        }

        bool operator>(const SourcePosition& rhs) const {
            return rhs < *this;
        }
        bool operator<=(const SourcePosition& rhs) const {
            return !(rhs < *this);
        }
        bool operator>=(const SourcePosition& rhs) const {
            return !(*this < rhs);
        }

        friend std::ostream& operator<<(std::ostream& os, const SourcePosition& position) {
            return position.output_to_stream(os);
        }
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
        explicit SourceFragment(SourcePosition begin) : begin_pos(begin), end_pos(begin) {};
        SourceFragment(SourcePosition begin, SourcePosition end) : begin_pos(begin), end_pos(end) {
            assert(begin <= end);
        };

        auto begin() const {
            return begin_pos;
        }

        auto end() const {
            return end_pos;
        }

        std::size_t size() const {
            return end_pos.cursor - begin_pos.cursor;
        }

        std::string_view str() const {
            return begin_pos.source->get(begin_pos.get_line(), begin_pos.get_cursor(), end_pos.get_line(), end_pos.get_cursor());
        }

        bool operator==(const SourceFragment& rhs) const {
            return begin_pos == rhs.begin_pos &&
                end_pos == rhs.end_pos;
        }

        bool operator!=(const SourceFragment& rhs) const {
            return !(rhs == *this);
        }

        friend std::ostream& operator<<(std::ostream& os, const SourceFragment& fragment) {
            return fragment.output_to_stream(os);
        }
    };

    public:
    using offset_t = SourcePosition::offset_t;

    private:
    std::string code;
    std::vector<SourcePosition::offset_t> lines;

    public:
    explicit source_code(std::string source_code) : code(std::move(source_code)) {
        if(code.empty()) return;

        if(code.back() != '\n') {
            code.push_back('\n');
        }
        auto prev_line_begin = code.begin();
        auto cur_line_end = prev_line_begin;

        do {
            cur_line_end = std::find(prev_line_begin, code.end(), '\n');
            lines.emplace_back(++cur_line_end - code.begin());
            prev_line_begin = cur_line_end;
        } while(cur_line_end != code.end());
    };
    explicit source_code(const char* code) : source_code(std::string(code)) {};
    explicit source_code(std::string_view code) : source_code(std::string(code)) {};
    explicit source_code(std::filesystem::path path) {
        throw std::runtime_error("Cannot read "+path.string()+": Not implemented");
    };

    auto begin() const {
        return SourcePosition(this, 0, 0);
    }

    auto end() const {
        return SourcePosition(this, number_of_lines(), 0);
    }

    SourcePosition::offset_t number_of_lines() const {
        return lines.size();
    }

    SourcePosition::offset_t line_length(SourcePosition::offset_t line) const {
        return get(line).size();
    }

    private:
    SourcePosition::offset_t get_line_offset(offset_t line) const {
        assert(line <= lines.size());
        if(line == 0) {
            return 0;
        } else if (line == number_of_lines()) { // Will not be reached if code is empty
            return lines.back();
        } else {
            return lines[line-1];
        }
    }
    protected:
    /// Returns the character in line line at position cursor
    char get(offset_t line, offset_t cursor) const {
        auto line_view = get(line);
        assert(cursor < line_view.size());
        return line_view[cursor];
    };

    /// Get substring
    std::string_view get(offset_t from_line, offset_t from_cursor, offset_t to_line, offset_t to_cursor) const {
        auto from_offset = get_line_offset(from_line) + from_cursor;
        return static_cast<std::string_view>(code).substr(from_offset,
                                                          get_line_offset(to_line) + to_cursor - from_offset);
    }

    /// Returns the line
    std::string_view get(offset_t line) const {
        auto line_offset = get_line_offset(line);
        return static_cast<std::string_view>(code).substr(line_offset, lines[line] - line_offset);
    };

};

using SourcePosition = source_code::SourcePosition;
using SourceFragment = source_code::SourceFragment;

} // namespace pljit::source_management

#endif //PLJIT_SOURCE_CODE_HPP
