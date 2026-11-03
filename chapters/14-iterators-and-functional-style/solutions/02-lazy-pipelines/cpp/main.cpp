#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Each view has its own type, and the standard library has no type that can
// hold any of them, so a pipeline built at run time needs its own common type.
// This one is a pull function: each call hands out the next item, or nothing.
using Items = std::function<std::optional<long long>()>;
using Words = std::vector<std::string>;

bool is_prime(long long n) {
    if (n < 2) return false;
    for (long long d = 2; d * d <= n; d++) {
        if (n % d == 0) return false;
    }
    return true;
}

Items naturals() {
    return [n = 0LL]() mutable -> std::optional<long long> { return ++n; };
}

Items source(const Words& words) {
    const std::string& name = words[0];
    if (name == "naturals") return naturals();
    if (name == "primes") {
        return [next = naturals()]() -> std::optional<long long> {
            while (true) {
                if (long long n = *next(); is_prime(n)) return n;
            }
        };
    }
    if (name == "fibonacci") {
        return [a = 0LL, b = 1LL]() mutable -> std::optional<long long> {
            return std::exchange(a, std::exchange(b, a + b));
        };
    }
    if (name == "range") {
        return [n = std::stoll(words[1]), high = std::stoll(words[2])]() mutable
                   -> std::optional<long long> {
            if (n > high) return std::nullopt;
            return n++;
        };
    }
    if (name == "repeat") {
        return [n = std::stoll(words[1])]() -> std::optional<long long> { return n; };
    }
    throw std::invalid_argument("unknown source " + name);
}

std::function<bool(long long)> predicate(const Words& words) {
    const std::string& name = words[0];
    if (name == "even") return [](long long x) { return x % 2 == 0; };
    if (name == "odd") return [](long long x) { return x % 2 != 0; };
    if (name == "prime") return is_prime;
    if (name == "above") return [n = std::stoll(words[1])](long long x) { return x > n; };
    if (name == "below") return [n = std::stoll(words[1])](long long x) { return x < n; };
    throw std::invalid_argument("unknown predicate " + name);
}

std::function<long long(long long)> function(const Words& words) {
    const std::string& name = words[0];
    if (name == "square") return [](long long x) { return x * x; };
    if (name == "negate") return [](long long x) { return -x; };
    if (name == "add") return [n = std::stoll(words[1])](long long x) { return x + n; };
    if (name == "mul") return [n = std::stoll(words[1])](long long x) { return x * n; };
    throw std::invalid_argument("unknown function " + name);
}

// Each stage wraps the pull function it is given in a new one. A lambda that
// keeps state between calls, such as a count, has to be mutable.
Items stage(Items items, const Words& words) {
    const std::string& name = words[0];
    const Words rest(words.begin() + 1, words.end());
    if (name == "map") {
        return [items = std::move(items), f = function(rest)]() -> std::optional<long long> {
            if (auto x = items()) return f(*x);
            return std::nullopt;
        };
    }
    if (name == "filter") {
        return [items = std::move(items), keep = predicate(rest)]() -> std::optional<long long> {
            while (auto x = items()) {
                if (keep(*x)) return x;
            }
            return std::nullopt;
        };
    }
    if (name == "takewhile") {
        return [items = std::move(items), keep = predicate(rest), done = false]() mutable
                   -> std::optional<long long> {
            if (done) return std::nullopt;
            if (auto x = items(); x && keep(*x)) return x;
            done = true;  // once one item fails, the rest are not looked at
            return std::nullopt;
        };
    }
    if (name == "dropwhile") {
        return [items = std::move(items), drop = predicate(rest), dropping = true]() mutable
                   -> std::optional<long long> {
            auto x = items();
            while (dropping && x && drop(*x)) x = items();
            dropping = false;
            return x;
        };
    }
    long long n = std::stoll(rest[0]);
    if (name == "take") {
        // Checking the count before pulling is what stops take from asking for
        // one item more than it needs.
        return [items = std::move(items), n]() mutable -> std::optional<long long> {
            if (n == 0) return std::nullopt;
            n--;
            return items();
        };
    }
    if (name == "skip") {
        return [items = std::move(items), n]() mutable -> std::optional<long long> {
            for (; n > 0; n--) {
                if (!items()) return std::nullopt;
            }
            return items();
        };
    }
    if (name == "step") {
        return [items = std::move(items), n, first = true]() mutable -> std::optional<long long> {
            for (long long i = 1; !first && i < n; i++) {
                if (!items()) return std::nullopt;
            }
            first = false;
            return items();
        };
    }
    throw std::invalid_argument("unknown stage " + name);
}

Words split(const std::string& text) {
    std::istringstream in(text);
    Words words;
    for (std::string word; in >> word;) words.push_back(word);
    return words;
}

std::string run(const std::string& line) {
    std::vector<Words> parts;
    std::istringstream in(line);
    for (std::string part; std::getline(in, part, '|');) parts.push_back(split(part));
    // A sum or count at the end is not a stage: it says what to do with the items.
    std::string finish = "list";
    if (parts.back() == Words{"sum"} || parts.back() == Words{"count"}) {
        finish = parts.back()[0];
        parts.pop_back();
    }
    Items items = source(parts[0]);
    for (std::size_t i = 1; i < parts.size(); i++) items = stage(std::move(items), parts[i]);
    // Nothing has run yet. Only the loop below pulls items through.
    long long total = 0, count = 0;
    std::string text;
    while (auto item = items()) {
        total += *item;
        count++;
        text += (text.empty() ? "" : " ") + std::to_string(*item);
    }
    if (finish == "sum") return std::to_string(total);
    if (finish == "count") return std::to_string(count);
    return text.empty() ? "(empty)" : text;
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.find_first_not_of(" \t\r") != std::string::npos) std::cout << run(line) << "\n";
    }
}
