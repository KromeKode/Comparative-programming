// Prints how one JSON file differs from another: what was removed, what was added, and
// what changed. ../README.md says what it should print.
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using nlohmann::json;

const std::string_view usage = "usage: jsondiff OLD NEW";
const std::regex identifier("[A-Za-z_][A-Za-z0-9_]*");

// All of a stream's bytes, or nothing if reading fails.
std::optional<std::string> read_all(std::istream& in) {
    std::string data;
    char buffer[4096];
    // read fills the buffer, or fails at the end with gcount() bytes read.
    while (in.read(buffer, sizeof buffer) || in.gcount() > 0) data.append(buffer, in.gcount());
    // Opening a folder works, and reading it fails, which sets badbit.
    if (in.bad()) return std::nullopt;
    return data;
}

// The part of a path that leads to a key: .name, or ["first name"]. dump writes a value
// as compact JSON; a json object is a std::map, which keeps its keys in order.
std::string step(const std::string& key) {
    return std::regex_match(key, identifier) ? "." + key : "[" + json(key).dump() + "]";
}

// Adds a line to out for each difference between before and after.
void diff(const std::string& path, const json& before, const json& after,
          std::vector<std::string>& out) {
    if (before.is_object() && after.is_object()) {
        // A std::set keeps its keys in order too: std::string's < compares bytes, which
        // for UTF-8 is the order of the characters' codes.
        std::set<std::string> keys;
        for (const auto& item : before.items()) keys.insert(item.key());
        for (const auto& item : after.items()) keys.insert(item.key());
        for (const std::string& key : keys) {
            std::string here = path + step(key);
            if (!after.contains(key)) {
                out.push_back("- " + here + ": " + before.at(key).dump());
            } else if (!before.contains(key)) {
                out.push_back("+ " + here + ": " + after.at(key).dump());
            } else {
                diff(here, before.at(key), after.at(key), out);
            }
        }
    } else if (before.is_array() && after.is_array()) {
        for (std::size_t i = 0; i < std::max(before.size(), after.size()); i++) {
            std::string here = path + "[" + std::to_string(i) + "]";
            if (i >= after.size()) {
                out.push_back("- " + here + ": " + before[i].dump());
            } else if (i >= before.size()) {
                out.push_back("+ " + here + ": " + after[i].dump());
            } else {
                diff(here, before[i], after[i], out);
            }
        }
    } else if (before != after) {  // == compares the kinds of value, then the values
        out.push_back("~ " + path + ": " + before.dump() + " -> " + after.dump());
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << usage << '\n';
        return 2;
    }
    json values[2];
    for (int i = 0; i < 2; i++) {
        const char* path = argv[i + 1];
        std::ifstream in(path, std::ios::binary);
        std::optional<std::string> text = in ? read_all(in) : std::nullopt;
        if (!text) {
            // The standard says nothing of errno for streams, but on Linux it says why.
            std::cerr << "jsondiff: " << path << ": " << std::strerror(errno) << '\n';
            return 2;
        }
        // With false for allow_exceptions, parse gives a "discarded" value instead of
        // throwing an exception when the text isn't JSON.
        values[i] = json::parse(*text, nullptr, false);
        if (values[i].is_discarded()) {
            std::cerr << "jsondiff: " << path << ": not valid JSON\n";
            return 2;
        }
    }
    std::vector<std::string> lines;
    diff("$", values[0], values[1], lines);
    for (const std::string& line : lines) std::cout << line << '\n';
    return lines.empty() ? 0 : 1;
}
