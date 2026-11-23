// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. nlohmann::json holds
// any JSON value, and get<Book>() turns one into a Book with the function from_json
// below, which checks each field.
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using nlohmann::json;

struct Book {
    std::string title;
    std::string author;
    int year;
    std::vector<std::string> tags;
};

// get<Book>() finds this function by the type of its second argument. at() throws
// out_of_range when a key is missing, and get_to throws type_error for a wrong type.
void from_json(const json& j, Book& book) {
    j.at("title").get_to(book.title);
    j.at("author").get_to(book.author);
    j.at("year").get_to(book.year);
    book.tags = j.value("tags", std::vector<std::string>{});  // the default if missing
}

// ordered_json keeps its keys in the order they went in; json would sort them.
nlohmann::ordered_json report(const std::vector<Book>& books) {
    const Book* oldest = nullptr;  // the first of the oldest, when two have the same year
    std::map<std::string, std::vector<std::string>> tags;  // keeps its keys in order
    std::vector<std::string> untagged;
    for (const Book& book : books) {
        if (oldest == nullptr || book.year < oldest->year) oldest = &book;
        for (const std::string& tag : book.tags) tags[tag].push_back(book.title);
        if (book.tags.empty()) untagged.push_back(book.title);
    }
    nlohmann::ordered_json result;
    result["count"] = books.size();
    // Not oldest ? ... : nullptr, whose type would be std::string, from a null pointer.
    result["oldest"] = nullptr;  // written as null
    if (oldest) result["oldest"] = oldest->title + ", by " + oldest->author;
    result["tags"] = tags;  // a map becomes an object, and a vector an array
    result["untagged"] = untagged;
    return result;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: books FILE\n";
        return 2;
    }
    std::string path = argv[1];
    std::ifstream in(path);
    if (!in) {
        std::cerr << "books: " << path << ": " << std::strerror(errno) << '\n';
        return 1;
    }
    std::vector<Book> books;
    try {
        books = json::parse(in).at("books").get<std::vector<Book>>();
    } catch (const json::exception& error) {
        // Such as "[json.exception.out_of_range.403] key 'year' not found".
        std::cerr << "books: " << path << ": " << error.what() << '\n';
        return 1;
    }
    std::cout << report(books).dump(2) << '\n';  // 2: indent by two spaces
}
