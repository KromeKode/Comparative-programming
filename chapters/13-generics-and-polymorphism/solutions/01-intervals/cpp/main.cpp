#include <algorithm>
#include <charconv>
#include <concepts>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct LineError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// = default compares the members in order, which is the right order here:
// major first, then minor, then patch, each as a number.
struct Version {
    unsigned long major, minor, patch;
    auto operator<=>(const Version&) const = default;
};

std::ostream& operator<<(std::ostream& out, const Version& v) {
    return out << v.major << "." << v.minor << "." << v.patch;
}

template <typename T>
struct Interval {
    T low, high;
};

// Sort by the low ends, then join each interval to the last one if they overlap.
template <std::totally_ordered T>
std::vector<Interval<T>> merge(std::vector<Interval<T>> intervals) {
    std::ranges::sort(intervals, {}, &Interval<T>::low);
    std::vector<Interval<T>> merged;
    for (const Interval<T>& interval : intervals) {
        if (!merged.empty() && interval.low <= merged.back().high) {
            merged.back().high = std::max(merged.back().high, interval.high);
        } else {
            merged.push_back(interval);
        }
    }
    return merged;
}

// from_chars takes a leading '-' but not '+' or spaces, and must use up the text.
template <typename Number>
std::optional<Number> parse_number(const std::string& text) {
    Number value{};
    auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc() || end != text.data() + text.size()) return std::nullopt;
    return value;
}

std::optional<long long> parse_int(const std::string& text) {
    return parse_number<long long>(text);
}

std::optional<std::string> parse_word(const std::string& text) {
    bool letters = !text.empty() && std::ranges::all_of(text, [](char c) {
        return c >= 'a' && c <= 'z';
    });
    if (!letters) return std::nullopt;
    return text;
}

std::optional<Version> parse_version(const std::string& text) {
    std::vector<unsigned long> parts;
    std::istringstream pieces(text);
    std::string piece;
    while (std::getline(pieces, piece, '.')) {
        // An unsigned type makes from_chars refuse a minus sign too.
        auto part = parse_number<unsigned long>(piece);
        if (!part) return std::nullopt;
        parts.push_back(*part);
    }
    if (parts.size() != 3 || text.back() == '.') return std::nullopt;
    return Version{parts[0], parts[1], parts[2]};
}

template <typename T>
using Parser = std::optional<T> (*)(const std::string&);

// Parse every interval on a line, merge them, and format the result.
template <std::totally_ordered T>
std::string run(const std::string& kind, Parser<T> parse, const std::vector<std::string>& words) {
    std::vector<Interval<T>> intervals;
    for (const std::string& word : words) {
        std::size_t dots = word.find("..");
        if (dots == std::string::npos || word.find("..", dots + 2) != std::string::npos) {
            throw LineError("bad interval \"" + word + "\"");
        }
        std::string low_text = word.substr(0, dots), high_text = word.substr(dots + 2);
        std::optional<T> low = parse(low_text), high = parse(high_text);
        if (!low) throw LineError("bad " + kind + " \"" + low_text + "\"");
        if (!high) throw LineError("bad " + kind + " \"" + high_text + "\"");
        if (*high < *low) throw LineError(word + " is backwards");
        intervals.push_back({*low, *high});
    }
    std::ostringstream text;
    text << kind;
    for (const Interval<T>& interval : merge(intervals)) {
        text << " " << interval.low << ".." << interval.high;
    }
    return text.str();
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::string kind, word;
        if (!(in >> kind)) continue;
        std::vector<std::string> words;
        while (in >> word) words.push_back(word);
        try {
            // Each branch fixes T, so the compiler makes one run for each kind.
            if (kind == "int") {
                std::cout << run(kind, parse_int, words) << "\n";
            } else if (kind == "word") {
                std::cout << run(kind, parse_word, words) << "\n";
            } else if (kind == "version") {
                std::cout << run(kind, parse_version, words) << "\n";
            } else {
                throw LineError("unknown kind \"" + kind + "\"");
            }
        } catch (const LineError& error) {
            std::cout << "error: " << error.what() << "\n";
        }
    }
}
