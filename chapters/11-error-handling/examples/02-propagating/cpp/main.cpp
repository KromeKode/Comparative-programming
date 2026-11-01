#include <charconv>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Pretend files, so the example does not touch the disk.
const std::map<std::string, std::string> files{{"good.txt", "1\n2\n3"},
                                               {"bad.txt", "4\nfive\n6"}};

// One exception class per kind of failure, so callers can catch just one.
struct NotFound : std::runtime_error {
    using std::runtime_error::runtime_error;
};
struct NotANumber : std::runtime_error {
    using std::runtime_error::runtime_error;
};

const std::string &read_file(const std::string &name) {
    auto found = files.find(name);
    if (found == files.end()) {
        throw NotFound(name + ": no such file");
    }
    return found->second;
}

long parse_number(const std::string &text) {
    long value = 0;
    const char *end = text.data() + text.size();
    auto [stop, error] = std::from_chars(text.data(), end, value);
    if (error != std::errc() || stop != end) {
        throw NotANumber("\"" + text + "\" is not a number");
    }
    return value;
}

long sum_file(const std::string &name) {
    // A NotFound from read_file passes straight through: no code needed.
    std::istringstream lines(read_file(name));
    long total = 0;
    std::string text;
    for (int line = 1; std::getline(lines, text); line++) {
        try {
            total += parse_number(text);
        } catch (const NotANumber &) {
            // Throw a new exception that carries the current one inside it.
            std::throw_with_nested(
                std::runtime_error(name + ": line " + std::to_string(line)));
        }
    }
    return total;
}

// A missing file counts as 0, but any other error stops the sum.
long sum_all(const std::vector<std::string> &names) {
    long total = 0;
    for (const std::string &name : names) {
        try {
            total += sum_file(name);
        } catch (const NotFound &) {
        }
    }
    return total;
}

// Join an exception's message with the messages of the ones nested inside it.
std::string describe(const std::exception &error) {
    std::string text = error.what();
    try {
        std::rethrow_if_nested(error);
    } catch (const std::exception &inner) {
        text += ": " + describe(inner);
    }
    return text;
}

template <typename Function>
void show(const std::string &label, Function function) {
    std::cout << "  " << label << " -> ";
    try {
        long total = function();
        std::cout << total << '\n';
    } catch (const std::exception &error) {
        std::cout << "error: " << describe(error) << '\n';
    }
}

int main() {
    std::cout << "one file at a time:\n";
    for (std::string name : {"good.txt", "bad.txt", "missing.txt"}) {
        show(name, [&] { return sum_file(name); });
    }
    std::cout << "several files, where a missing file counts as 0:\n";
    show("good.txt + missing.txt",
         [] { return sum_all({"good.txt", "missing.txt"}); });
    show("good.txt + bad.txt", [] { return sum_all({"good.txt", "bad.txt"}); });
}
