#include <cstddef>
#include <deque>
#include <iostream>
#include <numeric>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

using Group = std::vector<long long>;

// Each adapter holds a position in any input range and moves it forward one
// item at a time. next() hands out the next group, or nothing at the end.
template <std::ranges::input_range R>
class Windows {
public:
    Windows(R& items, std::size_t k)
        : it_(std::ranges::begin(items)), end_(std::ranges::end(items)), k_(k) {}

    std::optional<Group> next() {
        if (window_.size() == k_) window_.pop_front();  // the oldest item leaves
        while (window_.size() < k_) {
            if (it_ == end_) return std::nullopt;
            window_.push_back(*it_);
            ++it_;
        }
        return Group(window_.begin(), window_.end());
    }

private:
    std::ranges::iterator_t<R> it_;
    std::ranges::sentinel_t<R> end_;
    std::size_t k_;
    std::deque<long long> window_;
};

template <std::ranges::input_range R>
class Chunks {
public:
    Chunks(R& items, std::size_t k)
        : it_(std::ranges::begin(items)), end_(std::ranges::end(items)), k_(k) {}

    std::optional<Group> next() {
        Group chunk;
        for (; chunk.size() < k_ && it_ != end_; ++it_) chunk.push_back(*it_);
        if (chunk.empty()) return std::nullopt;
        return chunk;
    }

private:
    std::ranges::iterator_t<R> it_;
    std::ranges::sentinel_t<R> end_;
    std::size_t k_;
};

template <std::ranges::input_range R>
class Runs {
public:
    explicit Runs(R& items) : it_(std::ranges::begin(items)), end_(std::ranges::end(items)) {}

    std::optional<Group> next() {
        if (it_ == end_) return std::nullopt;
        Group run{*it_};
        // A position can be read without moving it, so *it_ looks at the next
        // item and ++it_ only happens when that item joins the run.
        for (++it_; it_ != end_ && *it_ == run[0]; ++it_) run.push_back(*it_);
        return run;
    }

private:
    std::ranges::iterator_t<R> it_;
    std::ranges::sentinel_t<R> end_;
};

template <typename Groups>
std::string show(Groups groups) {
    std::string text;
    while (auto group = groups.next()) {
        if (!text.empty()) text += " ";
        text += "[";
        for (std::size_t i = 0; i < group->size(); i++) {
            text += (i > 0 ? " " : "") + std::to_string((*group)[i]);
        }
        text += "]";
    }
    return text;
}

std::string answer(std::istringstream& in, const std::string& command) {
    std::size_t k = 0;
    if (command != "runs") in >> k;
    // istream_view is a lazy input range: each step reads one more number.
    auto numbers = std::views::istream<long long>(in);
    if (command == "runs") return show(Runs(numbers));
    if (command == "windows") return show(Windows(numbers, k));
    if (command == "chunks") return show(Chunks(numbers, k));
    std::string text;
    Windows windows(numbers, k);
    while (auto window = windows.next()) {
        if (!text.empty()) text += " ";
        text += std::to_string(std::accumulate(window->begin(), window->end(), 0LL));
    }
    return text;
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::string command;
        if (!(in >> command)) continue;  // a blank line
        std::string text = answer(in, command);
        std::cout << (text.empty() ? "(none)" : text) << "\n";
    }
}
