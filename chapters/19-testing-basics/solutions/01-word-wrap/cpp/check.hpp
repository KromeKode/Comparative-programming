// check.hpp: a test framework in one header, since C++'s standard library has
// none. It works the way GoogleTest and Catch2 do, in miniature.
//
// TEST(name) { ... } defines a test function and registers it, before main runs,
// by constructing a global object whose constructor adds the function to a list.
// CHECK(condition), CHECK_EQ(actual, expected), and CHECK_THROWS(statement, type)
// report a failure with its file and line, and carry on. run_all_tests() runs
// every registered test, and is what main returns: 0 if everything passed, 1 if
// anything failed.
#pragma once

#include <exception>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace check {

struct Test {
    std::string name;
    void (*body)();
};

// A function-local static, so the list exists before the first test registers
// itself, whichever file's globals are constructed first.
inline std::vector<Test>& registry() {
    static std::vector<Test> tests;
    return tests;
}

inline int failures = 0;

struct Registrar {
    Registrar(const char* name, void (*body)()) { registry().push_back({name, body}); }
};

inline void fail(const char* file, int line, const std::string& message) {
    std::cout << file << ":" << line << ": " << message << "\n";
    failures++;
}

template <typename T>
inline constexpr bool is_optional = false;
template <typename T>
inline constexpr bool is_optional<std::optional<T>> = true;

// Prints a value for a failure message: a string in quotes, an optional as its
// value or std::nullopt, a container such as a vector as {a, b, c}, and anything
// else with its operator<<.
template <typename T>
void print(std::ostream& out, const T& value) {
    if constexpr (std::is_convertible_v<const T&, std::string_view>) {
        out << '"' << std::string_view(value) << '"';
    } else if constexpr (is_optional<T>) {
        if (value) {
            print(out, *value);
        } else {
            out << "std::nullopt";
        }
    } else if constexpr (std::is_same_v<T, std::nullopt_t>) {
        out << "std::nullopt";
    } else if constexpr (requires { out << value; }) {
        out << value;
    } else {
        out << "{";
        const char* separator = "";
        for (const auto& item : value) {
            out << separator;
            print(out, item);
            separator = ", ";
        }
        out << "}";
    }
}

// Prints both values when they differ.
template <typename Actual, typename Expected>
void check_eq(const Actual& actual, const Expected& expected, const char* text, const char* file,
              int line) {
    if (actual == expected) return;
    std::cout << file << ":" << line << ": CHECK_EQ(" << text << ")\n  actual:   ";
    print(std::cout, actual);
    std::cout << "\n  expected: ";
    print(std::cout, expected);
    std::cout << "\n";
    failures++;
}

inline int run_all_tests() {
    int failed_tests = 0;
    for (const Test& test : registry()) {
        int before = failures;
        try {
            test.body();
        } catch (const std::exception& error) {
            fail(test.name.c_str(), 0, std::string("threw ") + error.what());
        }
        bool passed = failures == before;
        failed_tests += passed ? 0 : 1;
        std::cout << (passed ? "ok   " : "FAIL ") << test.name << "\n";
    }
    std::cout << registry().size() - failed_tests << " passed, " << failed_tests << " failed\n";
    return failed_tests == 0 ? 0 : 1;
}

}  // namespace check

#define TEST(name)                                               \
    static void name();                                          \
    static const check::Registrar name##_registrar(#name, name); \
    static void name()

#define CHECK(condition)                                                     \
    do {                                                                     \
        if (!(condition)) {                                                  \
            check::fail(__FILE__, __LINE__, "CHECK(" #condition ") failed"); \
        }                                                                    \
    } while (0)

#define CHECK_EQ(actual, expected) \
    check::check_eq((actual), (expected), #actual ", " #expected, __FILE__, __LINE__)

// The statement must throw an exception of that type, or of a type derived from it.
#define CHECK_THROWS(statement, exception_type)                                             \
    do {                                                                                    \
        bool check_threw = false;                                                           \
        try {                                                                               \
            statement;                                                                      \
        } catch (const exception_type&) {                                                   \
            check_threw = true;                                                             \
        }                                                                                   \
        if (!check_threw) {                                                                 \
            check::fail(__FILE__, __LINE__, "CHECK_THROWS(" #statement ", " #exception_type \
                        ") didn't throw");                                                  \
        }                                                                                   \
    } while (0)
