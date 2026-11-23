// Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
// input if there are none. Section 20.5 runs it in a debugger, to watch the parser
// call itself. It expects well-formed expressions, and doesn't check.
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

// A recursive descent parser, with one member function for each rule of the grammar.
class Parser {
public:
    explicit Parser(std::string_view text) : text_(text) {}

    // expr = term { ("+" | "-") term }
    long long expr() {
        long long value = term();
        while (peek() == '+' || peek() == '-') {
            char op = text_[pos_++];
            long long right = term();
            value = op == '+' ? value + right : value - right;
        }
        return value;
    }

private:
    // Skips spaces, and gives the next character, or '\0' at the end.
    char peek() {
        while (pos_ < text_.size() && text_[pos_] == ' ') pos_++;
        return pos_ < text_.size() ? text_[pos_] : '\0';
    }

    // term = factor { "*" factor }
    long long term() {
        long long value = factor();
        while (peek() == '*') {
            pos_++;
            value *= factor();
        }
        return value;
    }

    // factor = number | "(" expr ")"
    long long factor() {
        if (peek() == '(') {
            pos_++;
            long long value = expr();
            peek();
            pos_++;  // the ')'
            return value;
        }
        long long value = 0;
        while (pos_ < text_.size() && std::isdigit(static_cast<unsigned char>(text_[pos_]))) {
            value = value * 10 + (text_[pos_] - '0');
            pos_++;
        }
        return value;
    }

    std::string_view text_;
    std::size_t pos_ = 0;
};

long long evaluate(std::string_view text) { return Parser(text).expr(); }

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) std::cout << evaluate(argv[i]) << '\n';
    if (argc > 1) return 0;
    std::string line;
    while (std::getline(std::cin, line)) std::cout << evaluate(line) << '\n';
}
