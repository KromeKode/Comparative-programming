#include <cstdint>
#include <iostream>
#include <unordered_map>

std::uint64_t fib(int n) {
    // A static local is created once, on the first call, and kept between calls.
    static std::unordered_map<int, std::uint64_t> memo;
    if (n < 2) {
        return static_cast<std::uint64_t>(n);
    }
    if (auto found = memo.find(n); found != memo.end()) {
        return found->second;
    }
    const std::uint64_t value = fib(n - 1) + fib(n - 2);
    memo[n] = value;
    return value;
}

int main() {
    int n;
    while (std::cin >> n) {
        std::cout << "fib(" << n << ") = " << fib(n) << '\n';
    }
}
