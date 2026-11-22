#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

struct Test {
    const char* name;
    void (*run)();
};

// A function-local static is made the first time the function is called, so
// the list exists before any test is added to it, whatever the order.
std::vector<Test>& all_tests() {
    static std::vector<Test> tests;
    return tests;
}

// Making a Registrar adds a test to the list.
struct Registrar {
    Registrar(const char* name, void (*run)()) { all_tests().push_back({name, run}); }
};

// TEST declares the function, then defines a global Registrar for it. Global
// objects are made before main runs, so every test adds itself to the list,
// and the body written after TEST(name) becomes the function's body.
#define TEST(name)                                         \
    static void test_##name();                             \
    static Registrar registrar_##name(#name, test_##name); \
    static void test_##name()

struct CheckFailed {
    const char* text;
};

#define CHECK(expr)                            \
    do {                                       \
        if (!(expr)) throw CheckFailed{#expr}; \
    } while (0)

TEST(addition) {
    CHECK(2 + 2 == 4);
    CHECK(-3 + 3 == 0);
}

TEST(strings) {
    std::string word = "abc";
    std::ranges::transform(word, word.begin(), [](unsigned char c) { return std::toupper(c); });
    CHECK(word == "ABC");
    CHECK(word.size() == 3);
}

TEST(rounding) {
    CHECK(0.5 + 0.25 == 0.75);
    CHECK(0.1 + 0.2 == 0.3);
}

TEST(sorting) {
    std::vector numbers{3, 1, 2};
    std::ranges::sort(numbers);
    CHECK((numbers == std::vector{1, 2, 3}));
}

int main() {
    int failed = 0;
    std::cout << "running " << all_tests().size() << " tests\n";
    for (const Test& test : all_tests()) {
        try {
            test.run();
            std::cout << "PASS " << test.name << "\n";
        } catch (const CheckFailed& failure) {
            std::cout << "FAIL " << test.name << ": " << failure.text << "\n";
            failed++;
        }
    }
    std::cout << all_tests().size() - failed << " passed, " << failed << " failed\n";
    return 0;  // a real runner would return 1 when a test fails
}
