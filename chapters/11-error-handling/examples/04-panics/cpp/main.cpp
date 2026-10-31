#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

struct User {
    int id;
    std::string name;
};

const std::vector<User> users{{1, "ada"}};

std::optional<User> find_user(int id) {
    for (const User &user : users) {
        if (user.id == id) {
            return user;
        }
    }
    return std::nullopt;
}

template <typename Action> void attempt(const std::string &label, Action action) {
    std::cout << label << ": ";
    try {
        std::cout << action() << '\n';
    } catch (const std::out_of_range &) {
        std::cout << "threw std::out_of_range\n";
    } catch (const std::bad_optional_access &) {
        std::cout << "threw std::bad_optional_access\n";
    }
}

int main() {
    std::vector<int> numbers{10, 20, 30};
    std::size_t index = 5;
    int zero = 0;

    // The checked versions throw an exception.
    attempt("numbers.at(5)", [&] { return numbers.at(index); });
    attempt("find_user(7).value().name",
            [] { return find_user(7).value().name; });

    // The unchecked versions, numbers[5] and *find_user(7), are undefined
    // behavior, and so is dividing by zero. Test first instead.
    if (zero != 0) {
        std::cout << "10 / zero: " << 10 / zero << '\n';
    } else {
        std::cout << "10 / zero: skipped, because zero is 0\n";
    }
}
