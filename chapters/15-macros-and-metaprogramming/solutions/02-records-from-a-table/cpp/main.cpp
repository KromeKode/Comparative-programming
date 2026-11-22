#include <cstddef>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

struct Book {
    std::string title;
    std::string author;
    long long year = 0;
    long long pages = 0;
    bool in_print = false;
};

// A pointer to a member, such as &Book::year, names a field of every Book,
// and its type says the field's type: long long Book::*.
template <typename T>
struct Field {
    std::string_view name;
    T Book::*member;
};

// C++20 can't list a struct's members, so each one is paired with its name.
// A misspelled member fails to compile, but nothing checks the names.
constexpr std::tuple book_fields{
    Field{"title", &Book::title},
    Field{"author", &Book::author},
    Field{"year", &Book::year},
    Field{"pages", &Book::pages},
    Field{"in_print", &Book::in_print},
};

// Calls action with the field called name, and says whether there was one.
// std::apply hands every field to the lambda, and the fold expression tries
// them in turn, stopping at the first match.
template <typename Action>
bool with_field(std::string_view name, Action action) {
    auto try_each = [&](const auto&... field) {
        return ((field.name == name && (action(field), true)) || ...);
    };
    return std::apply(try_each, book_fields);
}

// One parser per type. The compiler picks the overload from the member's type.
using Problem = std::optional<std::string>;

Problem parse(std::string_view text, std::string& out) {
    out = text;
    return std::nullopt;
}

Problem parse(std::string_view text, long long& out) {
    std::string_view digits = text.starts_with('-') ? text.substr(1) : text;
    if (digits.empty() || digits.find_first_not_of("0123456789") != std::string_view::npos) {
        return "\"" + std::string(text) + "\" is not a whole number";
    }
    out = std::stoll(std::string(text));
    return std::nullopt;
}

Problem parse(std::string_view text, bool& out) {
    if (text != "true" && text != "false") {
        return "\"" + std::string(text) + "\" is not true or false";
    }
    out = text == "true";
    return std::nullopt;
}

std::string show(const std::string& value) { return "\"" + value + "\""; }
std::string show(long long value) { return std::to_string(value); }
std::string show(bool value) { return value ? "true" : "false"; }

std::string describe(const Book& book) {
    std::vector<std::string> parts;
    auto show_each = [&](const auto&... field) {
        (parts.push_back(std::string(field.name) + "=" + show(book.*field.member)), ...);
    };
    std::apply(show_each, book_fields);
    std::string text = "Book(";
    for (std::size_t i = 0; i < parts.size(); i++) text += (i > 0 ? ", " : "") + parts[i];
    return text + ")";
}

std::string trim(std::string_view text) {
    std::size_t start = text.find_first_not_of(" \t\r");
    if (start == std::string_view::npos) return "";
    std::size_t end = text.find_last_not_of(" \t\r");
    return std::string(text.substr(start, end - start + 1));
}

std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> parts;
    std::istringstream in(line);
    for (std::string part; std::getline(in, part, ',');) parts.push_back(trim(part));
    if (line.ends_with(',')) parts.push_back("");  // getline drops the last, empty part
    return parts;
}

std::string plural(int count, std::string_view word) {
    return std::to_string(count) + " " + std::string(word) + (count == 1 ? "" : "s");
}

int main() {
    std::vector<std::string> columns;
    bool have_header = false;
    int books = 0, bad = 0, row = 0;
    for (std::string line; std::getline(std::cin, line);) {
        if (trim(line).empty()) continue;
        std::vector<std::string> parts = split(line);
        if (!have_header) {
            for (std::size_t i = 0; i < parts.size(); i++) {
                if (!with_field(parts[i], [](const auto&) {})) {
                    std::cout << "unknown column: " << parts[i] << "\n";
                    return 0;
                }
                for (std::size_t j = 0; j < i; j++) {
                    if (parts[j] == parts[i]) {
                        std::cout << "repeated column: " << parts[i] << "\n";
                        return 0;
                    }
                }
            }
            columns = parts;
            have_header = true;
            continue;
        }
        row++;
        Problem problem;
        if (parts.size() != columns.size()) {
            problem = "expected " + std::to_string(columns.size()) + " values, got " +
                      std::to_string(parts.size());
        }
        Book book;
        for (std::size_t i = 0; i < columns.size() && !problem; i++) {
            with_field(columns[i], [&](const auto& field) {
                if (auto wrong = parse(parts[i], book.*field.member)) {
                    problem = columns[i] + ": " + *wrong;
                }
            });
        }
        if (problem) {
            std::cout << "row " << row << ": " << *problem << "\n";
            bad++;
        } else {
            std::cout << describe(book) << "\n";
            books++;
        }
    }
    if (have_header) std::cout << plural(books, "book") << ", " << plural(bad, "bad row") << "\n";
}
