#include <iostream>
#include <source_location>
#include <string_view>

// C++ has C's preprocessor, but constants, functions, and templates do most
// of its jobs, and they follow the rules for types, scope, and evaluation.
constexpr int limit = 3;

// auto in a parameter (C++20) makes square a template: one copy per type.
constexpr auto square(auto x) { return x * x; }

int clicks = 0;
int bump_clicks() { return ++clicks; }

// As a default argument, std::source_location::current() is the place the
// call came from. Only the expression's text still needs a macro.
template <typename T>
void show(std::string_view text, const T& value,
          std::source_location where = std::source_location::current()) {
    std::cout << "line " << where.line() << ": " << text << " = " << value << "\n";
}

#define SHOW(expr) show(#expr, (expr))

// A variadic template takes any number of arguments of any types, and the
// fold expression prints each one. No format string can get out of step.
template <typename... Parts>
void log_line(const Parts&... parts) {
    std::cout << "log:";
    ((std::cout << " " << parts), ...);
    std::cout << "\n";
}

#ifndef VERBOSE
#define VERBOSE 0
#endif

constexpr bool verbose = VERBOSE;

int main() {
    SHOW(limit * 2);
    SHOW(square(1 + 2));
    SHOW(square(1.5));

    // square is a function, so its argument runs once.
    SHOW(square(bump_clicks()));
    SHOW(clicks);
    log_line("clicks =", clicks, "| limit =", limit);

    // if constexpr picks a branch while compiling. Unlike #if, the branch it
    // drops must still be valid code, since outside a template it is checked.
    if constexpr (verbose) {
        log_line("built with VERBOSE =", VERBOSE);
    } else {
        std::cout << "verbose is off, and the compiler dropped the other branch\n";
    }
}
