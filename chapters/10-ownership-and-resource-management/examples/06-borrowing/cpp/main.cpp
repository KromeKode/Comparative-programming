#include <iostream>
#include <optional>
#include <string>
#include <string_view>

// Returns a or b, whichever is longer. A string_view is a pointer and a
// length that borrows someone else's characters, so the result is good only
// as long as both strings are. The compiler does not check that.
std::string_view longest(std::string_view a, std::string_view b) {
    return b.size() > a.size() ? b : a;
}

// Walks the words of a string without copying them. It borrows the text.
class Words {
  public:
    explicit Words(std::string_view text) : rest_(text) {}

    std::optional<std::string_view> next() {
        std::size_t start = rest_.find_first_not_of(' ');
        if (start == std::string_view::npos) {
            return std::nullopt;
        }
        rest_.remove_prefix(start);
        std::size_t end = rest_.find(' ');
        std::string_view word = rest_.substr(0, end);
        rest_.remove_prefix(word.size());
        return word;
    }

  private:
    std::string_view rest_;
};

int main() {
    std::string a = "apple";
    std::string b = "fig";
    std::string_view result = longest(a, b);
    std::cout << "longest(\"" << a << "\", \"" << b << "\") = " << result
              << '\n';

    const std::string text = "the quick brown fox";
    std::cout << "words of \"" << text << "\":";
    Words words(text);
    while (auto word = words.next()) {
        std::cout << " [" << *word << ']';
    }
    std::cout << '\n';

    // The words are views into text, not copies of it.
    Words again(text);
    again.next();
    if (auto second = again.next()) {
        std::cout << '"' << *second << "\" starts "
                  << second->data() - text.data() << " bytes into the text\n";
    }
}
