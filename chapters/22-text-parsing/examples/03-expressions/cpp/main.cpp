// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
#include <charconv>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

struct CalcError : std::runtime_error {
    int column;  // counted in characters, from 1
    CalcError(int column, const std::string& message)
        : std::runtime_error(message), column(column) {}
};

// The text is UTF-8, so a character is one to four bytes, and all but the first are
// 10xxxxxx. This is how many bytes the character that starts at text[i] has.
static std::size_t char_length(std::string_view text, std::size_t i) {
    std::size_t n = 1;
    while (i + n < text.size() && (static_cast<unsigned char>(text[i + n]) & 0xC0) == 0x80) n++;
    return n;
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
enum class Kind { Number, Operator, End };  // Operator includes the parentheses

struct Token {
    Kind kind;
    std::string text;
    int column;
    std::int64_t value = 0;  // a number's value

    std::string describe() const {
        return kind == Kind::End ? "the end of the line" : "\"" + text + "\"";
    }
};

static bool is_digit(char c) { return '0' <= c && c <= '9'; }

static std::vector<Token> tokens(std::string_view line) {
    std::vector<Token> found;
    int column = 1;
    for (std::size_t i = 0; i < line.size(); column++) {
        char c = line[i];
        if (c == ' ' || c == '\t') {
            i++;
        } else if (is_digit(c)) {
            std::size_t start = i;
            while (i < line.size() && is_digit(line[i])) i++;
            std::int64_t value = 0;
            // from_chars takes only the digits, with no spaces or "+", and says
            // result_out_of_range if the number doesn't fit.
            auto [end, err] = std::from_chars(line.data() + start, line.data() + i, value);
            if (err == std::errc::result_out_of_range) {
                throw CalcError(column, "number too large for 64 bits");
            }
            std::string text(line.substr(start, i - start));
            found.push_back({Kind::Number, text, column, value});
            column += static_cast<int>(i - start) - 1;
        } else if (std::string_view("+-*/%^()").find(c) != std::string_view::npos) {
            found.push_back({Kind::Operator, std::string(1, c), column});
            i++;
        } else {
            std::string text(line.substr(i, char_length(line, i)));
            throw CalcError(column, "unexpected \"" + text + "\"");
        }
    }
    found.push_back({Kind::End, "", column});
    return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds: an Expr holds
// one of them, in a variant. A node holds the nodes under it through unique_ptrs, which
// free them when it goes. column is where the operator is, for an error such as overflow.
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;
struct Number {
    std::int64_t value;
};
struct Negate {
    ExprPtr operand;
    int column;
};
struct Binary {
    char op;
    ExprPtr left, right;
    int column;
};
struct Expr {
    std::variant<Number, Negate, Binary> node;
};

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
static std::optional<std::pair<int, int>> infix(const Token& token) {
    if (token.kind != Kind::Operator) return std::nullopt;
    switch (token.text[0]) {
        case '+': case '-': return std::pair{1, 2};
        case '*': case '/': case '%': return std::pair{3, 4};
        case '^': return std::pair{8, 7};
        default: return std::nullopt;
    }
}

// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
constexpr int prefix = 5;

class Parser {
  public:
    explicit Parser(std::string_view line) : tokens_(tokens(line)) {}

    ExprPtr parse() {
        ExprPtr expr = expression(0);
        const Token& token = next();
        if (token.kind != Kind::End) {
            throw CalcError(token.column, "expected an operator, found " + token.describe());
        }
        return expr;
    }

  private:
    std::vector<Token> tokens_;
    std::size_t at_ = 0;

    const Token& next() {
        const Token& token = tokens_[at_];
        if (token.kind != Kind::End) at_++;
        return token;
    }

    // Parses the longest expression whose operators hold at least min_power.
    ExprPtr expression(int min_power) {
        const Token& token = next();
        ExprPtr left;
        if (token.kind == Kind::Number) {
            left = std::make_unique<Expr>(Number{token.value});
        } else if (token.text == "-") {
            left = std::make_unique<Expr>(Negate{expression(prefix), token.column});
        } else if (token.text == "(") {
            left = expression(0);
            const Token& close = next();
            if (close.text != ")") {
                throw CalcError(close.column, "expected \")\", found " + close.describe());
            }
        } else {
            throw CalcError(token.column,
                            "expected a number, \"(\", or \"-\", found " + token.describe());
        }
        for (;;) {
            const Token& op = tokens_[at_];
            auto powers = infix(op);
            if (!powers || powers->first < min_power) return left;
            next();
            ExprPtr right = expression(powers->second);
            left = std::make_unique<Expr>(Binary{op.text[0], std::move(left), std::move(right),
                                                 op.column});
        }
    }
};

// Calls the lambda that takes the node's type: the usual way to visit a variant.
template <typename... F>
struct Overloaded : F... {
    using F::operator()...;
};

static std::string show(const Expr& expr) {
    return std::visit(
        Overloaded{
            [](const Number& n) { return std::to_string(n.value); },
            [](const Negate& n) { return "(-" + show(*n.operand) + ")"; },
            [](const Binary& b) {
                return "(" + show(*b.left) + " " + b.op + " " + show(*b.right) + ")";
            },
        },
        expr.node);
}

// Signed overflow is undefined behavior in C++, so the answer must be checked before
// it's made. GCC and Clang's __builtin_mul_overflow and the others work out the answer,
// and say whether it overflowed.
static std::int64_t power(std::int64_t base, std::int64_t exponent, int column) {
    std::int64_t result = 1;
    // Squares the base for each bit of the exponent, and stops at the first overflow.
    while (exponent > 0) {
        if (exponent % 2 == 1 && __builtin_mul_overflow(result, base, &result)) {
            throw CalcError(column, "overflow");
        }
        exponent /= 2;
        if (exponent > 0 && __builtin_mul_overflow(base, base, &base)) {
            throw CalcError(column, "overflow");
        }
    }
    return result;
}

static std::int64_t evaluate(const Expr& expr) {
    constexpr std::int64_t min = std::numeric_limits<std::int64_t>::min();
    return std::visit(
        Overloaded{
            [](const Number& n) { return n.value; },
            [](const Negate& n) {
                std::int64_t value = evaluate(*n.operand);
                if (value == min) throw CalcError(n.column, "overflow");
                return -value;
            },
            [](const Binary& b) {
                std::int64_t x = evaluate(*b.left), y = evaluate(*b.right), result = 0;
                bool overflow = false;
                if ((b.op == '/' || b.op == '%') && y == 0) {
                    throw CalcError(b.column, "division by zero");
                }
                switch (b.op) {
                    case '+': overflow = __builtin_add_overflow(x, y, &result); break;
                    case '-': overflow = __builtin_sub_overflow(x, y, &result); break;
                    case '*': overflow = __builtin_mul_overflow(x, y, &result); break;
                    // / and % round toward zero. min / -1 and min % -1 are undefined,
                    // since the quotient doesn't fit; the remainder is 0.
                    case '/': overflow = x == min && y == -1; result = overflow ? 0 : x / y; break;
                    case '%': result = y == -1 ? 0 : x % y; break;
                    default:  // '^'
                        if (y < 0) throw CalcError(b.column, "negative exponent");
                        result = power(x, y, b.column);
                }
                if (overflow) throw CalcError(b.column, "overflow");
                return result;
            },
        },
        expr.node);
}

int main() {
    int status = 0;
    std::string line;
    // getline splits at "\n" only, and keeps a "\r" before it.
    for (int number = 1; std::getline(std::cin, line); number++) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.find_first_not_of(" \t") == std::string::npos) continue;
        try {
            ExprPtr expr = Parser(line).parse();
            // Worked out first: in one << chain, show's text would be written before
            // evaluate threw.
            std::int64_t value = evaluate(*expr);
            std::cout << show(*expr) << " = " << value << '\n';
        } catch (const CalcError& err) {
            // The caret goes under the column. A tab before it stays a tab, so that the
            // caret lines up with the text whatever a tab's width is.
            std::string pad;
            for (std::size_t i = 0; static_cast<int>(pad.size()) < err.column - 1;
                 i += char_length(line, i)) {
                pad += line[i] == '\t' ? '\t' : ' ';
            }
            std::cerr << "calc: line " << number << ", column " << err.column << ": "
                      << err.what() << "\n  " << line << "\n  " << pad << "^\n";
            status = 1;
        }
    }
    return status;
}
