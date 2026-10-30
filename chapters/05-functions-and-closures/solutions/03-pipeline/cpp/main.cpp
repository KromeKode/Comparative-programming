#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using Step = std::function<long long(long long)>;

// Each lambda captures its own n.
Step make_step(const std::string& op, long long n) {
    if (op == "add") return [n](long long x) { return x + n; };
    if (op == "sub") return [n](long long x) { return x - n; };
    if (op == "mul") return [n](long long x) { return x * n; };
    if (op == "min") return [n](long long x) { return std::min(x, n); };
    if (op == "max") return [n](long long x) { return std::max(x, n); };
    throw std::invalid_argument("unknown step: " + op);
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream words(line);
        long long start = 0;
        if (!(words >> start)) {
            continue;  // a blank line
        }
        std::vector<Step> steps;
        std::string op;
        long long n = 0;
        while (words >> op >> n) {
            steps.push_back(make_step(op, n));
        }
        long long value = start;
        for (const Step& step : steps) {
            value = step(value);
        }
        std::cout << start << " -> " << value << '\n';
    }
}
