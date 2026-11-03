#include <charconv>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct CalcError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// What eval needs from a number type. N::parse is a static member: eval calls
// it on the type, before it has any N to call it on.
template <typename N>
concept Number = requires(const N a, const N b, const std::string& text) {
    { N::parse(text) } -> std::same_as<std::optional<N>>;
    { a + b } -> std::same_as<N>;
    { a - b } -> std::same_as<N>;
    { a * b } -> std::same_as<N>;
    { a / b } -> std::same_as<N>;  // throws CalcError when dividing by zero
    { a.str() } -> std::same_as<std::string>;
};

// A whole number: an optional minus sign and at least one digit. from_chars
// takes a leading '-' but not '+', and must use up the text.
std::optional<std::int64_t> parse_whole(const std::string& text) {
    std::int64_t value = 0;
    auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc() || end != text.data() + text.size()) return std::nullopt;
    return value;
}

struct Int {
    std::int64_t value;

    static std::optional<Int> parse(const std::string& text) {
        auto n = parse_whole(text);
        if (!n) return std::nullopt;
        return Int{*n};
    }

    Int operator+(Int other) const { return {value + other.value}; }
    Int operator-(Int other) const { return {value - other.value}; }
    Int operator*(Int other) const { return {value * other.value}; }

    // C++'s / rounds toward zero.
    Int operator/(Int other) const {
        if (other.value == 0) throw CalcError("division by zero");
        return {value / other.value};
    }

    std::string str() const { return std::to_string(value); }
};

// Always in lowest terms, with a positive bottom.
class Frac {
public:
    Frac(std::int64_t top, std::int64_t bottom) {
        if (bottom < 0) {
            top = -top;
            bottom = -bottom;
        }
        std::int64_t divisor = std::gcd(top, bottom);  // std::gcd(0, b) is b
        top_ = top / divisor;
        bottom_ = bottom / divisor;
    }

    static std::optional<Frac> parse(const std::string& text) {
        std::size_t slash = text.find('/');
        auto top = parse_whole(text.substr(0, slash));
        auto bottom = slash == std::string::npos ? std::optional<std::int64_t>(1)
                                                 : parse_whole(text.substr(slash + 1));
        // "1/-2" and "1/0" are not fractions: the bottom must be above zero.
        if (!top || !bottom || *bottom <= 0) return std::nullopt;
        return Frac(*top, *bottom);
    }

    Frac operator+(Frac b) const {
        return {top_ * b.bottom_ + b.top_ * bottom_, bottom_ * b.bottom_};
    }

    Frac operator-(Frac b) const {
        return {top_ * b.bottom_ - b.top_ * bottom_, bottom_ * b.bottom_};
    }

    Frac operator*(Frac b) const { return {top_ * b.top_, bottom_ * b.bottom_}; }

    Frac operator/(Frac b) const {
        if (b.top_ == 0) throw CalcError("division by zero");
        return {top_ * b.bottom_, bottom_ * b.top_};
    }

    std::string str() const {
        std::string text = std::to_string(top_);
        return bottom_ == 1 ? text : text + "/" + std::to_string(bottom_);
    }

private:
    std::int64_t top_, bottom_;
};

// Always 0 to 6.
class Mod7 {
public:
    // C++'s % keeps the sign of the left side, so -1 % 7 is -1. Adding 7 fixes it.
    explicit Mod7(std::int64_t n) : value_((n % 7 + 7) % 7) {}

    static std::optional<Mod7> parse(const std::string& text) {
        auto n = parse_whole(text);
        if (!n) return std::nullopt;
        return Mod7(*n);
    }

    Mod7 operator+(Mod7 other) const { return Mod7(value_ + other.value_); }
    Mod7 operator-(Mod7 other) const { return Mod7(value_ - other.value_); }
    Mod7 operator*(Mod7 other) const { return Mod7(value_ * other.value_); }

    // x to the power 5 is the inverse of x when 7 is prime: x * x^5 = x^6 = 1.
    Mod7 operator/(Mod7 other) const {
        if (other.value_ == 0) throw CalcError("division by zero");
        return *this * other * other * other * other * other;
    }

    std::string str() const { return std::to_string(value_); }

private:
    std::int64_t value_;
};

// Run the tokens on a stack of N. The same code works for every number type.
template <Number N>
N eval(const std::vector<std::string>& tokens) {
    std::vector<N> stack;
    for (const std::string& token : tokens) {
        if (token == "+" || token == "-" || token == "*" || token == "/") {
            if (stack.size() < 2) throw CalcError("not enough numbers");
            N right = stack.back();
            stack.pop_back();
            N left = stack.back();
            stack.pop_back();
            if (token == "+") stack.push_back(left + right);
            if (token == "-") stack.push_back(left - right);
            if (token == "*") stack.push_back(left * right);
            if (token == "/") stack.push_back(left / right);
        } else {
            std::optional<N> number = N::parse(token);
            if (!number) throw CalcError("bad token \"" + token + "\"");
            stack.push_back(*number);
        }
    }
    if (stack.size() != 1) {
        throw CalcError("expected one result, found " + std::to_string(stack.size()));
    }
    return stack.back();
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::string kind;
        if (!(in >> kind)) continue;
        std::vector<std::string> tokens;
        for (std::string token; in >> token;) tokens.push_back(token);
        try {
            if (kind == "int") {
                std::cout << eval<Int>(tokens).str() << "\n";
            } else if (kind == "frac") {
                std::cout << eval<Frac>(tokens).str() << "\n";
            } else if (kind == "mod7") {
                std::cout << eval<Mod7>(tokens).str() << "\n";
            } else {
                throw CalcError("unknown kind \"" + kind + "\"");
            }
        } catch (const CalcError& error) {
            std::cout << "error: " << error.what() << "\n";
        }
    }
}
