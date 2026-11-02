#include <concepts>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// A concept names a constraint. Summable asks for a + that gives back a T,
// and for T{} to make a zero to start from.
template <typename T>
concept Summable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    T{};
};

template <typename T>
concept Printable = requires(std::ostream& out, const T& value) { out << value; };

// Money keeps whole cents, so adding never rounds.
struct Money {
    long cents = 0;

    Money operator+(Money other) const { return {cents + other.cents}; }
    auto operator<=>(const Money&) const = default;
};

std::ostream& operator<<(std::ostream& out, Money money) {
    long rest = money.cents % 100;
    return out << "$" << money.cents / 100 << "." << (rest < 10 ? "0" : "") << rest;
}

template <Summable T>
T total(const std::vector<T>& items) {
    T sum{};
    for (const T& item : items) sum = sum + item;
    return sum;
}

// Several constraints, joined with && in a requires clause.
template <typename T>
    requires std::totally_ordered<T> && Printable<T>
std::string describe_range(const std::vector<T>& items) {
    T low = items.front(), high = items.front();
    for (const T& item : items) {
        if (item < low) low = item;
        if (item > high) high = item;
    }
    std::ostringstream text;
    text << low << " to " << high;
    return text.str();
}

template <Printable T>
std::string list(const std::vector<T>& items) {
    std::ostringstream text;
    for (std::size_t i = 0; i < items.size(); i++) text << (i ? ", " : "") << items[i];
    return text.str();
}

int main() {
    std::vector numbers{1, 2, 3};
    std::vector halves{0.5, 0.25};
    std::vector prices{Money{150}, Money{225}};
    std::cout << "total of " << list(numbers) << ": " << total(numbers) << "\n";
    std::cout << "total of " << list(halves) << ": " << total(halves) << "\n";
    std::cout << "total of " << list(prices) << ": " << total(prices) << "\n";

    std::vector digits{3, 9, 1};
    std::vector<std::string> fruit{"pear", "fig", "apple"};
    std::vector costs{Money{200}, Money{99}};
    std::cout << "range of " << list(digits) << ": " << describe_range(digits) << "\n";
    std::cout << "range of " << list(fruit) << ": " << describe_range(fruit) << "\n";
    std::cout << "range of " << list(costs) << ": " << describe_range(costs) << "\n";
}
