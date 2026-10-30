#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

std::function<long long()> make_counter(long long start, long long step) {
    // next is the lambda's own copy, and mutable lets each call change it.
    return [next = start, step]() mutable {
        const long long value = next;
        next += step;
        return value;
    };
}

int main() {
    std::unordered_map<std::string, std::function<long long()>> counters;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream words(line);
        std::string command, name;
        words >> command >> name;
        if (command == "new") {
            long long start = 0, step = 0;
            words >> start >> step;
            counters[name] = make_counter(start, step);
        } else if (command == "next") {
            if (auto found = counters.find(name); found != counters.end()) {
                std::cout << name << ": " << found->second() << '\n';
            } else {
                std::cout << name << ": no such counter\n";
            }
        }
    }
}
