#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct User {
    std::string name;
    std::optional<std::string> nickname; // empty when the user has no nickname
};

// Returns a pointer to the user, or nullptr if nobody has that name.
const User *find(const std::vector<User> &users, std::string_view name) {
    auto it = std::ranges::find(users, name, &User::name);
    return it != users.end() ? &*it : nullptr;
}

int main() {
    std::vector<User> users = {
        {"ada", "Countess"},
        {"alan", std::nullopt},
        {"grace", "Amazing Grace"},
    };

    for (const auto &user : users) {
        // value_or gives a default when the optional is empty.
        std::cout << user.name << " is shown as " << user.nickname.value_or(user.name)
                  << '\n';
    }

    std::cout << "nickname lengths:";
    for (const auto &user : users) {
        std::cout << ' ' << (user.nickname ? user.nickname->size() : 0);
    }
    std::cout << '\n';

    for (std::string_view name : {"alan", "linus"}) {
        const User *user = find(users, name);
        std::cout << "find \"" << name << "\": " << (user ? user->name : "nobody") << '\n';
    }

    for (std::string_view name : {"ada", "alan", "linus"}) {
        const User *user = find(users, name);
        std::string nickname = "none";
        if (user != nullptr && user->nickname) {
            nickname = *user->nickname;
        }
        std::cout << "nickname of \"" << name << "\": " << nickname << '\n';
    }
}
