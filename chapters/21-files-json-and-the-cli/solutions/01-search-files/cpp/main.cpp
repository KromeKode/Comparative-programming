// Prints the lines that contain a pattern, from each file named on the command line, or
// from stdin. ../README.md says what it should print.
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

const std::string_view usage = "usage: search [-icnv] PATTERN [FILE...]";
const std::string_view stdin_name = "(standard input)";

struct Options {
    bool ignore_case = false, count = false, numbers = false, invert = false;
};

int usage_error(std::string_view message) {
    std::cerr << "search: " << message << '\n' << usage << '\n';
    return 2;
}

// A to Z made a to z, and every other byte as it was. std::tolower would depend on the
// locale, and could change bytes above 127.
std::string lower(std::string bytes) {
    for (char& c : bytes) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return bytes;
}

// All of a stream's bytes, or nothing if reading fails. A std::string holds any bytes,
// '\0' and bytes that aren't UTF-8 too.
std::optional<std::string> read_all(std::istream& in) {
    std::string data;
    char buffer[4096];
    // read fills the buffer, or fails at the end with gcount() bytes read.
    while (in.read(buffer, sizeof buffer) || in.gcount() > 0) data.append(buffer, in.gcount());
    // Opening a folder works, and reading it fails, which sets badbit.
    if (in.bad()) return std::nullopt;
    return data;
}

// Writes the lines of data that are selected, and returns how many there are.
long search(const std::string& data, std::string_view name, const std::string& pattern,
            const Options& options) {
    std::string prefix = name.empty() ? "" : std::string(name) + ":";
    long count = 0, number = 0;
    for (std::size_t start = 0; start < data.size();) {
        std::size_t end = data.find('\n', start);
        if (end == std::string::npos) end = data.size();  // a last line without a newline
        std::string line = data.substr(start, end - start);
        number++;
        start = end + 1;
        bool found = (options.ignore_case ? lower(line) : line).find(pattern) != std::string::npos;
        if (found == options.invert) continue;
        count++;
        if (!options.count) {
            std::cout << prefix;
            if (options.numbers) std::cout << number << ':';
            std::cout << line << '\n';  // '\n', not std::endl, which would flush each line
        }
    }
    if (options.count) std::cout << prefix << count << '\n';
    return count;
}

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    Options options;
    std::size_t i = 0;
    // Options come first; the first argument that isn't one is the pattern.
    for (; i < args.size() && args[i].size() > 1 && args[i][0] == '-'; i++) {
        if (args[i] == "--") {
            i++;
            break;
        }
        for (char letter : args[i].substr(1)) {
            switch (letter) {
            case 'i': options.ignore_case = true; break;
            case 'c': options.count = true; break;
            case 'n': options.numbers = true; break;
            case 'v': options.invert = true; break;
            default: return usage_error(std::string("unknown option -") + letter);
            }
        }
    }
    if (i == args.size()) return usage_error("no pattern");
    std::string pattern = options.ignore_case ? lower(args[i]) : args[i];
    std::vector<std::string> files(args.begin() + i + 1, args.end());
    if (files.empty()) files.push_back("-");

    long selected = 0;
    int status = 0;
    for (const std::string& file : files) {
        std::optional<std::string> data;
        if (file == "-") {
            data = read_all(std::cin);
        } else if (std::ifstream in(file, std::ios::binary); in) {
            data = read_all(in);
        }
        if (!data) {
            std::cout.flush();  // what came before, before the error
            // The standard says nothing of errno for streams, but on Linux it says why.
            std::cerr << "search: " << file << ": " << std::strerror(errno) << '\n';
            status = 2;
            continue;
        }
        std::string_view name = files.size() == 1 ? "" : file == "-" ? stdin_name : file;
        selected += search(*data, name, pattern, options);
    }
    if (status == 0 && selected == 0) status = 1;
    return status;
}
