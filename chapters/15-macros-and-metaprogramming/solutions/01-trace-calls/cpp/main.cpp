#include <cstddef>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using num = long long;

struct Tracer {
    bool on = false;
    int depth = 0;
    long calls = 0;

    void say(const std::string& text) const {
        if (on) std::cout << std::string(2 * depth, ' ') << text << "\n";
    }
};

Tracer tracer;

template <typename... Args>
std::string show_call(std::string_view name, const Args&... args) {
    std::ostringstream text;
    text << name << "(";
    std::string_view sep;
    ((text << sep << args, sep = ", "), ...);
    text << ")";
    return text.str();
}

// traced runs body as one traced call of the function named name. The fold
// expression in show_call prints however many arguments there are. The
// recursive calls in each body go through the traced function again.
template <typename Body, typename... Args>
num traced(std::string_view name, Body body, Args... args) {
    tracer.calls++;
    tracer.say(show_call(name, args...));
    tracer.depth++;
    num result = body();
    tracer.depth--;
    tracer.say("-> " + std::to_string(result));
    return result;
}

// __func__ is the name of the function it appears in, so each traced
// function passes its own name, its body as a lambda, and its arguments.
num fib(num n) {
    return traced(__func__, [&] { return n < 2 ? n : fib(n - 1) + fib(n - 2); }, n);
}

num gcd(num a, num b) {
    return traced(__func__, [&] { return b == 0 ? a : gcd(b, a % b); }, a, b);
}

num power(num base, num exponent) {
    auto body = [&]() -> num {
        if (exponent == 0) return 1;
        if (exponent % 2 == 0) {
            num half = power(base, exponent / 2);
            return half * half;
        }
        return base * power(base, exponent - 1);
    };
    return traced(__func__, body, base, exponent);
}

struct Entry {
    std::size_t arity;
    std::function<num(const std::vector<num>&)> call;
};

// sizeof...(Args) counts a function's parameters, and the index sequence
// 0, 1, ... unpacks a vector into that many arguments.
template <typename... Args, std::size_t... I>
Entry make_entry(num (*f)(Args...), std::index_sequence<I...>) {
    return {sizeof...(Args), [f](const std::vector<num>& args) { return f(args[I]...); }};
}

template <typename... Args>
Entry make_entry(num (*f)(Args...)) {
    return make_entry(f, std::index_sequence_for<Args...>{});
}

#define ENTRY(f) {#f, make_entry(f)}

const std::map<std::string, Entry, std::less<>> functions{ENTRY(fib), ENTRY(gcd), ENTRY(power)};

std::string plural(long count, std::string_view word) {
    return std::to_string(count) + " " + std::string(word) + (count == 1 ? "" : "s");
}

void run(const std::string& line) {
    std::istringstream in(line);
    std::string command, name;
    in >> command >> name;
    std::vector<num> args;
    for (num arg; in >> arg;) args.push_back(arg);
    if (command != "trace" && command != "count") {
        std::cout << "unknown command: " << command << "\n";
        return;
    }
    auto found = functions.find(name);
    if (found == functions.end()) {
        std::cout << "unknown function: " << name << "\n";
        return;
    }
    const Entry& entry = found->second;
    if (args.size() != entry.arity) {
        std::cout << name << " takes " << plural(static_cast<long>(entry.arity), "argument")
                  << ", got " << args.size() << "\n";
        return;
    }
    tracer = Tracer{command == "trace"};
    num result = entry.call(args);
    std::cout << name << "(";
    for (std::size_t i = 0; i < args.size(); i++) std::cout << (i ? ", " : "") << args[i];
    std::cout << ") = " << result << " in " << plural(tracer.calls, "call") << "\n";
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.find_first_not_of(" \t\r") != std::string::npos) run(line);
    }
}
