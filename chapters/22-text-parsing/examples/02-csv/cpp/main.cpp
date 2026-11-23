// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
#include <cstdio>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

enum class State {
    RecordStart,  // nothing of this record read yet
    FieldStart,   // just after a comma
    Unquoted,     // in a field that doesn't start with a quote
    Quoted,       // in a field that does
    Quote,        // just after a quote in a quoted field: its end, or the first of ""
    Cr,           // just after a "\r" outside quotes, which needs a "\n" next
};

struct CsvError : std::runtime_error {
    CsvError(int line, int column, const std::string& message)
        : std::runtime_error("line " + std::to_string(line) + ", column " +
                             std::to_string(column) + ": " + message) {}
};

static std::string fields(std::size_t n) {
    return n == 1 ? "1 field" : std::to_string(n) + " fields";
}

// Calls emit with each record in text. Throws a CsvError at the first error.
template <typename Emit>
void parse(const std::string& text, Emit emit) {
    using enum State;  // C++20: Quoted, not State::Quoted
    constexpr int end = -1;  // stands for the end, so that it goes through the machine too
    State state = RecordStart;
    std::vector<std::string> record;
    std::string field;
    std::size_t width = 0;  // how many fields the first record has, once it has ended
    int line = 1, column = 0;  // where c is: columns count characters, from 1
    int start_line = 0, start_column = 0;  // where a quoted field's quote is, or the "\r"
    int record_line = 1;
    for (std::size_t i = 0; i <= text.size(); i++) {
        int c = i < text.size() ? static_cast<unsigned char>(text[i]) : end;
        // The text is UTF-8, so a character is one to four bytes. All but the first are
        // 10xxxxxx, and they don't move the column. None of them is a quote, a comma, or
        // a line end, so they go into the field with the rest of their character.
        if ((c & 0xC0) != 0x80) column++;
        if (state == RecordStart) record_line = line;
        bool ended = false;  // whether c ends a record
        if (state == Quoted) {
            if (c == end) throw CsvError(start_line, start_column, "quoted field is never closed");
            if (c == '"') state = Quote;
            else field += static_cast<char>(c);
        } else if (state == Quote && c == '"') {
            field += '"';
            state = Quoted;
        } else if (state == Cr) {
            if (c != '\n') {
                throw CsvError(start_line, start_column,
                               "a carriage return without a line feed after it");
            }
            ended = true;
        } else if (state == RecordStart && c == end) {
            // the end, at the start or just after a line end: no record
        } else if (c == '"' && state != Quote) {
            if (state == Unquoted) throw CsvError(line, column, "a quote in an unquoted field");
            start_line = line;
            start_column = column;
            state = Quoted;
        } else if (c == ',') {
            record.push_back(std::move(field));
            field.clear();
            state = FieldStart;
        } else if (c == '\n' || c == end) {
            ended = true;
        } else if (c == '\r') {
            start_line = line;
            start_column = column;
            state = Cr;
        } else if (state == Quote) {
            throw CsvError(line, column, "expected a comma or a line end after a closing quote");
        } else {
            field += static_cast<char>(c);
            state = Unquoted;
        }
        if (ended) {
            record.push_back(std::move(field));
            field.clear();
            if (width == 0) {
                width = record.size();
            } else if (record.size() != width) {
                throw CsvError(record_line, 1, "record has " + fields(record.size()) +
                                                   ", but the first has " + std::to_string(width));
            }
            emit(record);
            record.clear();
            state = RecordStart;
        }
        if (c == '\n') {
            line++;
            column = 0;
        }
    }
}

// Writes s as a JSON string, as JavaScript's JSON.stringify does. Bytes of UTF-8 above
// 0x7F are written as they are.
static void write_json(std::ostream& out, const std::string& s) {
    out << '"';
    for (unsigned char c : s) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (c < 0x20) {
                    char escape[7];
                    std::snprintf(escape, sizeof escape, "\\u%04x", c);
                    out << escape;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    out << '"';
}

int main() {
    // The whole of stdin, as bytes.
    std::string text{std::istreambuf_iterator<char>(std::cin), {}};
    try {
        parse(text, [](const std::vector<std::string>& record) {
            std::cout << '[';
            for (std::size_t i = 0; i < record.size(); i++) {
                if (i > 0) std::cout << ',';
                write_json(std::cout, record[i]);
            }
            std::cout << "]\n";
        });
    } catch (const CsvError& err) {
        std::cout.flush();
        std::cerr << "csv2json: " << err.what() << '\n';
        return 1;
    }
    return 0;
}
