// Exercise 18.3: version ranges. The highest version that each range allows.
//
// The versions and ranges are semver.cpp's business; this file reads the input and
// prints the answers.
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "semver.hpp"

int main() {
    std::string line;
    if (!std::getline(std::cin, line)) return 0;
    std::vector<semver::Version> versions;
    std::istringstream words(line);
    for (std::string word; words >> word;) {
        std::optional<semver::Version> version = semver::parse_version(word);
        if (!version) {
            std::cout << "error: line 1: " << word << " isn't a version\n";
            return 0;
        }
        versions.push_back(*version);
    }
    if (versions.empty()) {
        std::cout << "error: line 1 must list the versions\n";
        return 0;
    }
    for (int number = 2; std::getline(std::cin, line); number++) {
        const char* space = " \t\n\v\f\r";
        std::size_t start = line.find_first_not_of(space);
        if (start == std::string::npos) continue;
        line = line.substr(start, line.find_last_not_of(space) + 1 - start);
        try {
            semver::Range range(line);
            std::optional<semver::Version> best = range.highest(versions);
            std::cout << line << " -> " << (best ? best->str() : "none") << '\n';
        } catch (const std::invalid_argument& error) {
            std::cout << "error: line " << number << ": " << error.what() << '\n';
        }
    }
}
