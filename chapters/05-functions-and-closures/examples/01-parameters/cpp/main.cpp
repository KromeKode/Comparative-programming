#include <iostream>
#include <string>
#include <vector>

// Default arguments fill in from the right. There are no named arguments,
// so to change punctuation you must also pass greeting.
std::string greet(const std::string& name, const std::string& greeting = "Hello",
                  const std::string& punctuation = "!") {
    return greeting + ", " + name + punctuation;
}

// A variadic template takes any number of arguments, each with its real type
// checked. The fold expression (0 + ... + values) adds them all.
template <typename... Ints>
int total(Ints... values) {
    return (0 + ... + values);
}

// A second function with the same name but a different parameter list: an overload.
int total(const std::vector<int>& values) {
    int sum = 0;
    for (int value : values) {
        sum += value;
    }
    return sum;
}

int main() {
    std::cout << greet("Ada") << '\n';
    std::cout << greet("Grace", "Hi") << '\n';
    std::cout << greet("Linus", "Hello", "?") << '\n';

    std::cout << "total() = " << total() << '\n';
    std::cout << "total(1, 2, 3) = " << total(1, 2, 3) << '\n';
    const std::vector<int> numbers{4, 3, 2, 1};
    std::cout << "total of [4, 3, 2, 1] = " << total(numbers) << '\n';
}
