#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const char* yes_no(bool condition) { return condition ? "yes" : "no"; }

// std::tolower takes an int that must fit in unsigned char, hence the cast.
char lower(char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
char upper(char c) { return static_cast<char>(std::toupper(static_cast<unsigned char>(c))); }

void print_list(const std::string& label, const std::vector<std::string>& items) {
    std::cout << label << ':';
    for (const auto& item : items) {
        std::cout << ' ' << item;
    }
    std::cout << '\n';
}

int main() {
    const std::string text = "  The quick  brown fox  ";

    const auto first = text.find_first_not_of(" \t\n");
    const auto last = text.find_last_not_of(" \t\n");
    const std::string trimmed = text.substr(first, last - first + 1);
    std::cout << "trimmed: [" << trimmed << "]\n";

    // >> reads one word at a time, skipping any amount of whitespace.
    std::istringstream stream(trimmed);
    std::string word;
    std::string words;
    while (stream >> word) {
        words += (words.empty() ? "" : "|") + word;
    }
    std::cout << "words: " << words << '\n';

    std::string shouted = trimmed;
    std::transform(shouted.begin(), shouted.end(), shouted.begin(), upper);
    std::cout << "upper: " << shouted << '\n';

    // find returns std::string::npos when the text is not there.
    std::cout << "contains \"quick\": " << yes_no(trimmed.find("quick") != std::string::npos) << '\n';
    std::cout << "index of \"brown\": " << trimmed.find("brown") << '\n';

    std::string replaced = trimmed;
    replaced.replace(replaced.find("quick"), std::string("quick").size(), "slow");
    std::cout << "replaced: " << replaced << '\n';

    std::cout << "starts with \"The\": " << yes_no(trimmed.starts_with("The")) << '\n';

    std::vector<std::string> fruits = {"banana", "apple", "Cherry"};
    std::sort(fruits.begin(), fruits.end());  // compares bytes
    print_list("sorted", fruits);
    std::sort(fruits.begin(), fruits.end(), [](const std::string& a, const std::string& b) {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                            [](char x, char y) { return lower(x) < lower(y); });
    });
    print_list("sorted ignoring case", fruits);
}
