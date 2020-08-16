#include "SourceCode.hpp"

namespace pljit::source_management {

std::ostream& SourcePosition::output_to_stream(std::ostream& os) const {
    os << "Position " << line << ":" << cursor << '\n';
    os << source->get(line); // Includes newline
    std::fill_n(std::ostream_iterator<char>(os), cursor, ' ');
    os << '^';
    return os;
}
SourcePosition::SourcePosition(const SourceCode* source, size_t line, size_t cursor) noexcept : source(source), line(line), cursor(cursor) {}
size_t SourcePosition::get_line() const {
    return line;
}
size_t SourcePosition::get_cursor() const {
    return cursor;
}
char SourcePosition::operator*() {
    return source->get(line, cursor);
}
SourcePosition& SourcePosition::operator++() {
    ++cursor;
    // TODO Perhaps check against \n?
    if(cursor == source->line_length(line)) {
        ++line;
        cursor = 0;
    }
    return *this;
}
SourcePosition& SourcePosition::operator--() {
    if(cursor == 0) {
        --line;
        cursor = source->line_length(line) - 1;
    } else {
        --cursor;
    }
    return *this;
}
SourcePosition SourcePosition::operator--(int) {
    SourcePosition other = *this;
    --(*this);
    return other;
}
SourcePosition SourcePosition::operator++(int) {
    SourcePosition other = *this;
    ++(*this);
    return other;
}
bool SourcePosition::operator==(const SourcePosition& rhs) const {
    return line == rhs.line && cursor == rhs.cursor;
}
bool SourcePosition::operator!=(const SourcePosition& rhs) const {
    return !(rhs == *this);
}
bool SourcePosition::operator<(const SourcePosition& rhs) const {
    return (line < rhs.line) || line > rhs.line || cursor < rhs.cursor;
}
bool SourcePosition::operator>(const SourcePosition& rhs) const {
    return rhs < *this;
}
bool SourcePosition::operator>=(const SourcePosition& rhs) const {
    return !(*this < rhs);
}
bool SourcePosition::operator<=(const SourcePosition& rhs) const {
    return !(rhs < *this);
}
std::ostream& operator<<(std::ostream& os, const SourcePosition& position) {
    return position.output_to_stream(os);
}
SourceFragment::SourceFragment(SourcePosition begin, SourcePosition end) : begin_pos(begin), end_pos(end) {
    assert(begin <= end);
}
SourceFragment::SourceFragment(SourcePosition begin) : begin_pos(begin), end_pos(begin) {}
auto SourceFragment::begin() const {
    return begin_pos;
}
auto SourceFragment::end() const {
    return end_pos;
}
std::size_t SourceFragment::size() const {
    return end_pos.cursor - begin_pos.cursor;
}
std::string_view SourceFragment::str() const {
    return begin_pos.source->get(begin_pos.get_line(), begin_pos.get_cursor(), end_pos.get_line(), end_pos.get_cursor());
}
bool SourceFragment::operator==(const SourceFragment& rhs) const {
    return begin_pos == rhs.begin_pos &&
           end_pos == rhs.end_pos;
}
bool SourceFragment::operator!=(const SourceFragment& rhs) const {
    return !(rhs == *this);
}
std::ostream& operator<<(std::ostream& os, const SourceFragment& fragment) {
    return fragment.output_to_stream(os);
}
void SourceFragment::extend(const SourceFragment& other) {
    if(other.end() > end()) {
        end_pos = other.end_pos;
    }
    if(other.begin() < begin()) {
        begin_pos = other.begin_pos;
    }
}
SourcePosition::offset_t SourceCode::line_length(SourcePosition::offset_t line) const {
    return get(line).size();
}
SourcePosition::offset_t SourceCode::get_line_offset(SourceCode::offset_t line) const {
    assert(line <= lines.size());
    if(line == 0) {
        return 0;
    } else if (line == number_of_lines()) { // Will not be reached if code is empty
        return lines.back();
    } else {
        return lines[line-1];
    }
}
char SourceCode::get(SourceCode::offset_t line, SourceCode::offset_t cursor) const {
    auto line_view = get(line);
    assert(cursor < line_view.size());
    return line_view[cursor];
}
std::string_view SourceCode::get(SourceCode::offset_t from_line, SourceCode::offset_t from_cursor, SourceCode::offset_t to_line, SourceCode::offset_t to_cursor) const {
    auto from_offset = get_line_offset(from_line) + from_cursor;
    return static_cast<std::string_view>(code).substr(from_offset,
                                                      get_line_offset(to_line) + to_cursor - from_offset);
}
std::string_view SourceCode::get(SourceCode::offset_t line) const {
    auto line_offset = get_line_offset(line);
    return static_cast<std::string_view>(code).substr(line_offset, lines[line] - line_offset);
}
SourcePosition::offset_t SourceCode::number_of_lines() const {
    return lines.size();
}
SourcePosition SourceCode::end() const {
    return SourcePosition(this, number_of_lines(), 0);
}
SourcePosition SourceCode::begin() const {
    return SourcePosition(this, 0, 0);
}
SourceCode::SourceCode(const char* code) : SourceCode(std::string(code)) {}
SourceCode::SourceCode(std::string_view code) : SourceCode(std::string(code)) {}
SourceCode::SourceCode(std::string source_code) : code(std::move(source_code)) {
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
}
} // namespace pljit::source_management