#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

constexpr std::int64_t max_value = std::numeric_limits<std::int32_t>::max();
constexpr std::int64_t min_value = std::numeric_limits<std::int32_t>::min();

// One exception class with a kind, in the style of std::system_error.
class ParseError : public std::runtime_error {
  public:
    enum class Kind { Empty, NoDigits, InvalidCharacter, TooLarge, TooSmall };

    ParseError(Kind kind, const std::string &message)
        : std::runtime_error(message), kind_(kind) {}

    Kind kind() const { return kind_; }

  private:
    Kind kind_;
};

std::int32_t parse_int(const std::string &text) {
    using Kind = ParseError::Kind;
    if (text.empty()) {
        throw ParseError(Kind::Empty, "empty input");
    }
    bool negative = text[0] == '-';
    std::size_t start = (text[0] == '+' || text[0] == '-') ? 1 : 0;
    if (start == text.size()) {
        throw ParseError(Kind::NoDigits, "no digits after the sign");
    }
    // Check every character first, so a bad one is reported even when the
    // digits before it are already too large.
    for (std::size_t i = start; i < text.size(); i++) {
        if (text[i] < '0' || text[i] > '9') {
            throw ParseError(Kind::InvalidCharacter,
                             "invalid character '" + std::string(1, text[i]) +
                                 "' at position " + std::to_string(i + 1));
        }
    }
    // Stop growing once the number is past every limit, so it cannot
    // overflow however many digits there are.
    std::int64_t magnitude = 0;
    for (std::size_t i = start; i < text.size(); i++) {
        magnitude = std::min(magnitude * 10 + (text[i] - '0'), -min_value + 1);
    }
    if (!negative && magnitude > max_value) {
        throw ParseError(Kind::TooLarge, "too large, the maximum is " +
                                             std::to_string(max_value));
    }
    if (negative && magnitude > -min_value) {
        throw ParseError(Kind::TooSmall, "too small, the minimum is " +
                                             std::to_string(min_value));
    }
    return static_cast<std::int32_t>(negative ? -magnitude : magnitude);
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::cout << '"' << line << "\" -> ";
        try {
            std::cout << parse_int(line) << '\n';
        } catch (const ParseError &error) {
            std::cout << "error: " << error.what() << '\n';
        }
    }
}
