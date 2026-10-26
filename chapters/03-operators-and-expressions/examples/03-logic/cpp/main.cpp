#include <iostream>
#include <string>

int calls = 0;  // counts how many times the right side ran

bool big_ratio(int x) {
    ++calls;
    return 10 / x > 1;  // dividing by zero here would be undefined behavior
}

std::string display_name(const std::string& name) {
    return name.empty() ? "stranger" : name;
}

int main() {
    for (int x : {0, 5}) {
        // && stops as soon as the left side is false, so big_ratio never sees 0.
        bool ok = x != 0 && big_ratio(x);
        std::cout << "x = " << x << ": " << (ok ? "yes" : "no") << '\n';
    }
    std::cout << "right side ran " << calls << " of 2 times\n";
    std::cout << "name \"\": " << display_name("") << '\n';
    std::cout << "name \"Ada\": " << display_name("Ada") << '\n';
}
