#include <cstddef>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// A class template. Stack<int> and Stack<std::string> are two separate
// classes, each made by the compiler from this one definition.
template <typename T>
class Stack {
public:
    void push(T item) { items_.push_back(std::move(item)); }

    std::optional<T> pop() {
        if (items_.empty()) return std::nullopt;
        T top = std::move(items_.back());
        items_.pop_back();
        return top;
    }

    std::optional<T> peek() const {
        if (items_.empty()) return std::nullopt;
        return items_.back();
    }

    std::size_t size() const { return items_.size(); }

private:
    std::vector<T> items_;
};

// A function template. Nothing says T must support >; the compiler finds out
// when it makes a copy of this function for a type (section 13.4).
template <typename T>
std::optional<T> largest(const std::vector<T>& items) {
    if (items.empty()) return std::nullopt;
    T best = items.front();
    for (const T& item : items) {
        if (item > best) best = item;
    }
    return best;
}

// Two type parameters. swap returns a Pair with them the other way round.
template <typename A, typename B>
struct Pair {
    A first;
    B second;

    Pair<B, A> swap() const { return {second, first}; }
};

template <typename A, typename B>
std::ostream& operator<<(std::ostream& out, const Pair<A, B>& pair) {
    return out << "(" << pair.first << ", " << pair.second << ")";
}

template <typename T>
std::string show(const std::optional<T>& value) {
    if (!value) return "none";
    std::ostringstream text;
    text << *value;
    return text.str();
}

template <typename T>
void report(const std::string& name, Stack<T>& stack) {
    std::string popped = show(stack.pop());
    std::string top = show(stack.peek());
    std::cout << name << ": popped " << popped << ", top is " << top << ", "
              << stack.size() << " left\n";
}

int main() {
    Stack<int> numbers;
    numbers.push(3);
    numbers.push(1);
    numbers.push(4);
    report("numbers", numbers);

    Stack<std::string> words;
    words.push("pear");
    words.push("fig");
    report("words", words);

    Stack<double> empty;
    report("empty", empty);

    std::cout << "largest number: " << show(largest(std::vector{3, 1, 4, 1, 5})) << "\n";
    std::vector<std::string> fruit{"pear", "fig", "apple"};
    std::cout << "largest word: " << show(largest(fruit)) << "\n";
    std::cout << "largest of nothing: " << show(largest(std::vector<int>{})) << "\n";

    Pair pair{7, std::string("seven")};  // Pair<int, std::string>, deduced
    std::cout << pair << " swapped is " << pair.swap() << "\n";
}
