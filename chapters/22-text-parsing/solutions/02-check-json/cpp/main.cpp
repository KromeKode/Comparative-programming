// Reads one JSON text on stdin, strictly as RFC 8259 has it, and prints it compactly:
// no spaces, keys in the order they came, numbers as they were written, and strings
// written again with only the escapes they need. Or prints the first error's line and
// column, and exits with 1.
#include <cstdio>
#include <iostream>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

constexpr int max_depth = 1000;

// s, which is UTF-8, as a JSON string, with only the escapes it needs. The bytes of
// characters above U+007F are written as they are.
static std::string quote(std::string_view s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char escape[7];
                    std::snprintf(escape, sizeof escape, "\\u%04x", c);
                    out += escape;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out + "\"";
}

// Appends the code point as UTF-8: one to four bytes.
static void append_utf8(std::string& out, char32_t c) {
    if (c < 0x80) {
        out += static_cast<char>(c);
    } else if (c < 0x800) {
        out += static_cast<char>(0xC0 | c >> 6);
        out += static_cast<char>(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        out += static_cast<char>(0xE0 | c >> 12);
        out += static_cast<char>(0x80 | (c >> 6 & 0x3F));
        out += static_cast<char>(0x80 | (c & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | c >> 18);
        out += static_cast<char>(0x80 | (c >> 12 & 0x3F));
        out += static_cast<char>(0x80 | (c >> 6 & 0x3F));
        out += static_cast<char>(0x80 | (c & 0x3F));
    }
}

struct JsonError : std::runtime_error {
    std::size_t at;  // a byte offset into the text
    JsonError(std::size_t at, const std::string& message)
        : std::runtime_error(message), at(at) {}
};

class Parser {
  public:
    explicit Parser(std::string_view text) : text_(text) {}

    std::string parse() {
        space();
        std::string out = value();
        space();
        if (i_ < text_.size()) throw fail("the end of the input");
        return out;
    }

  private:
    std::string_view text_;  // UTF-8, so a character is one to four bytes
    std::size_t i_ = 0;
    int depth_ = 0;

    // The byte at i, or -1 at the end.
    int peek() const { return i_ < text_.size() ? static_cast<unsigned char>(text_[i_]) : -1; }

    JsonError fail(const std::string& expected) const {
        std::string found = "the end of the input";
        if (i_ < text_.size()) {
            // The whole character: its first byte, and the 10xxxxxx bytes after it.
            std::size_t n = 1;
            auto byte = [&](std::size_t k) { return static_cast<unsigned char>(text_[k]); };
            while (i_ + n < text_.size() && (byte(i_ + n) & 0xC0) == 0x80) n++;
            found = quote(text_.substr(i_, n));
        }
        return JsonError(i_, "expected " + expected + ", found " + found);
    }

    void space() {
        while (peek() == ' ' || peek() == '\t' || peek() == '\n' || peek() == '\r') i_++;
    }

    bool is_digit() const { return peek() >= '0' && peek() <= '9'; }

    std::string value() {
        if (peek() == '{' || peek() == '[') {
            if (++depth_ > max_depth) {
                throw JsonError(i_, "nesting deeper than " + std::to_string(max_depth));
            }
            std::string out = peek() == '{' ? object() : array();
            depth_--;
            return out;
        }
        if (peek() == '"') return quote(string());
        if (peek() == '-' || is_digit()) return number();
        for (std::string_view word : {"true", "false", "null"}) {
            if (text_.substr(i_).starts_with(word)) {
                i_ += word.size();
                return std::string(word);
            }
        }
        throw fail("a value");
    }

    std::string object() {
        i_++;  // the {
        space();
        if (peek() == '}') {
            i_++;
            return "{}";
        }
        std::string out = "{";
        std::unordered_set<std::string> keys;
        for (;;) {
            if (peek() != '"') throw fail(keys.empty() ? "a string key or \"}\"" : "a string key");
            std::size_t at = i_;
            std::string key = string();
            // insert's second is false if the set had it already.
            if (!keys.insert(key).second) throw JsonError(at, "duplicate key " + quote(key));
            space();
            if (peek() != ':') throw fail("\":\"");
            i_++;
            space();
            out += quote(key) + ":" + value();
            space();
            if (peek() == '}') {
                i_++;
                return out + "}";
            }
            if (peek() != ',') throw fail("\",\" or \"}\"");
            out += ",";
            i_++;
            space();
        }
    }

    std::string array() {
        i_++;  // the [
        space();
        if (peek() == ']') {
            i_++;
            return "[]";
        }
        std::string out = "[";
        for (;;) {
            out += value();
            space();
            if (peek() == ']') {
                i_++;
                return out + "]";
            }
            if (peek() != ',') throw fail("\",\" or \"]\"");
            out += ",";
            i_++;
            space();
        }
    }

    void digits() {
        if (!is_digit()) throw fail("a digit");
        while (is_digit()) i_++;
    }

    std::string number() {
        std::size_t start = i_;
        if (peek() == '-') i_++;
        if (peek() == '0') {
            i_++;  // a 0 can't have digits after it
        } else {
            digits();
        }
        if (peek() == '.') {
            i_++;
            digits();
        }
        if (peek() == 'e' || peek() == 'E') {
            i_++;
            if (peek() == '+' || peek() == '-') i_++;
            digits();
        }
        return std::string(text_.substr(start, i_ - start));  // as it was written
    }

    // The four hex digits at text[at], or nothing if they aren't.
    std::optional<char32_t> hex4(std::size_t at) const {
        if (at + 4 > text_.size()) return std::nullopt;
        char32_t n = 0;
        for (char c : text_.substr(at, 4)) {
            if (c >= '0' && c <= '9') n = n * 16 + (c - '0');
            else if (c >= 'a' && c <= 'f') n = n * 16 + (c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') n = n * 16 + (c - 'A' + 10);
            else return std::nullopt;
        }
        return n;
    }

    // The string at i, with its escapes undone, as UTF-8.
    std::string string() {
        std::size_t start = i_;
        i_++;  // the "
        std::string out;
        for (;;) {
            int c = peek();
            if (c == -1) throw JsonError(start, "string is never closed");
            if (c == '"') {
                i_++;
                return out;
            }
            if (c < 0x20) throw JsonError(i_, "control character in a string");
            if (c != '\\') {
                out += static_cast<char>(c);  // a byte of UTF-8, which goes as it is
                i_++;
                continue;
            }
            if (i_ + 1 == text_.size()) throw JsonError(start, "string is never closed");
            std::size_t at = i_;
            char escape = text_[i_ + 1];
            i_ += 2;
            switch (escape) {
                case '"': out += '"'; continue;
                case '\\': out += '\\'; continue;
                case '/': out += '/'; continue;
                case 'b': out += '\b'; continue;
                case 'f': out += '\f'; continue;
                case 'n': out += '\n'; continue;
                case 'r': out += '\r'; continue;
                case 't': out += '\t'; continue;
                case 'u': break;
                default: throw JsonError(at, "invalid escape");
            }
            std::optional<char32_t> code = hex4(i_);
            if (!code) throw JsonError(at, "invalid escape");
            i_ += 4;
            if (*code >= 0xDC00 && *code <= 0xDFFF) throw JsonError(at, "unpaired surrogate");
            if (*code >= 0xD800 && *code <= 0xDBFF) {
                // A character above U+FFFF is two escapes, a high surrogate and a low one.
                std::optional<char32_t> low;
                if (text_.substr(i_).starts_with("\\u")) low = hex4(i_ + 2);
                if (!low || *low < 0xDC00 || *low > 0xDFFF) {
                    throw JsonError(at, "unpaired surrogate");
                }
                *code = 0x10000 + ((*code - 0xD800) << 10) + (*low - 0xDC00);
                i_ += 6;
            }
            append_utf8(out, *code);
        }
    }
};

int main() {
    std::string text{std::istreambuf_iterator<char>(std::cin), {}};
    try {
        std::cout << Parser(text).parse() << '\n';
    } catch (const JsonError& err) {
        // Lines end at "\n", and columns count characters from 1: each byte that isn't a
        // 10xxxxxx byte starts a character.
        int line = 1, column = 1;
        for (std::size_t i = 0; i < err.at; i++) {
            if (text[i] == '\n') {
                line++;
                column = 1;
            } else if ((static_cast<unsigned char>(text[i]) & 0xC0) != 0x80) {
                column++;
            }
        }
        std::cerr << "jsoncheck: line " << line << ", column " << column << ": " << err.what()
                  << '\n';
        return 1;
    }
}
