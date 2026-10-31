#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

int main() {
    std::vector<std::string> inputs{"42", "4x2", "", "99999999999999999999"};

    // std::stoi throws an exception when it cannot parse. It stops at the first
    // character that is not a digit, so it also reports how much it used.
    std::cout << "std::stoi:\n";
    for (const std::string &text : inputs) {
        std::cout << "  \"" << text << "\" -> ";
        try {
            std::size_t used = 0;
            int value = std::stoi(text, &used);
            std::cout << value;
            if (used != text.size()) {
                std::cout << " (it used " << used << " of " << text.size()
                          << " characters)";
            }
            std::cout << '\n';
        } catch (const std::invalid_argument &e) {
            std::cout << "std::invalid_argument: " << e.what() << '\n';
        } catch (const std::out_of_range &e) {
            std::cout << "std::out_of_range: " << e.what() << '\n';
        }
    }

    // std::from_chars returns an error code instead, and never throws.
    std::cout << "std::from_chars:\n";
    for (const std::string &text : inputs) {
        int value = 0;
        const char *end = text.data() + text.size();
        auto [stop, error] = std::from_chars(text.data(), end, value);
        std::cout << "  \"" << text << "\" -> ";
        if (error != std::errc()) {
            std::cout << "error: " << std::make_error_code(error).message()
                      << '\n';
        } else if (stop != end) {
            std::cout << value << ", with characters left over\n";
        } else {
            std::cout << value << '\n';
        }
    }
}
