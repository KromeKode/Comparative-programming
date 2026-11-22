#include <iostream>
#include <set>
#include <string>

struct Book {
    std::string title;
    std::string author;
    int year;
    bool in_print;

    // = default asks the compiler to write these. == compares every field,
    // and <=> compares them in order, which gives <, <=, >, and >= too.
    bool operator==(const Book&) const = default;
    auto operator<=>(const Book&) const = default;
};

// C++20 can't list a struct's fields, so printing is written by hand, and
// nothing checks that it keeps up when a field is added.
std::ostream& operator<<(std::ostream& out, const Book& book) {
    return out << "Book{.title = \"" << book.title << "\", .author = \"" << book.author
               << "\", .year = " << book.year << ", .in_print = " << book.in_print << "}";
}

int main() {
    std::cout << std::boolalpha;
    Book dune{"Dune", "Frank Herbert", 1965, true};
    std::cout << "dune: " << dune << "\n";

    Book copy = dune;
    std::cout << "copy == dune: " << (copy == dune) << "\n";

    Book reissue = dune;
    reissue.year = 1966;
    std::cout << "reissue: " << reissue << "\n";
    std::cout << "reissue == dune: " << (reissue == dune) << "\n";
    std::cout << "dune < reissue: " << (dune < reissue) << "\n";

    // std::set keeps one of each, using the < that <=> gave Book.
    std::set<Book> distinct{dune, copy, reissue};
    std::cout << "distinct books: " << distinct.size() << "\n";
}
