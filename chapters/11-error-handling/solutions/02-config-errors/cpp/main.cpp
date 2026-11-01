#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

struct Config {
    std::string name;
    long port = 0;
    bool debug = false;
    long timeout = 30;
};

// One struct per kind of error, and a variant that holds any one of them.
struct SyntaxError {
    int line;
};
struct UnknownKey {
    int line;
    std::string key;
};
struct DuplicateKey {
    int line;
    std::string key;
    int first;
};
struct BadValue {
    int line;
    std::string key;
    std::string problem;
};
struct MissingKey {
    std::string key;
};
using ConfigError =
    std::variant<SyntaxError, UnknownKey, DuplicateKey, BadValue, MissingKey>;

// The names in the same order as the variant, so index() picks the name.
const std::array<std::string, 5> kind_names{
    "syntax", "unknown key", "duplicate key", "bad value", "missing key"};

// Lets std::visit take one lambda per type.
template <class... Ts> struct overloaded : Ts... {
    using Ts::operator()...;
};

std::string line_prefix(int line) { return "line " + std::to_string(line) + ": "; }

std::string describe(const ConfigError &error) {
    return std::visit(
        overloaded{
            [](const SyntaxError &e) {
                return line_prefix(e.line) + "expected \"key = value\"";
            },
            [](const UnknownKey &e) {
                return line_prefix(e.line) + "unknown key \"" + e.key + "\"";
            },
            [](const DuplicateKey &e) {
                return line_prefix(e.line) + "\"" + e.key +
                       "\" is already set on line " + std::to_string(e.first);
            },
            [](const BadValue &e) {
                return line_prefix(e.line) + e.key + ": " + e.problem;
            },
            [](const MissingKey &e) {
                return "missing key \"" + e.key + "\"";
            },
        },
        error);
}

// The value parsers throw this; load catches it and records a BadValue.
struct ValueProblem : std::runtime_error {
    using std::runtime_error::runtime_error;
};

long parse_whole(const std::string &text, long low, long high) {
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](char c) {
            return c >= '0' && c <= '9';
        })) {
        throw ValueProblem("\"" + text + "\" is not a whole number");
    }
    long value = 0;
    for (char c : text) {
        // Stop growing past the limit, so a long number cannot overflow.
        value = std::min(value * 10 + (c - '0'), high + 1);
    }
    if (value < low || value > high) {
        throw ValueProblem(text + " is not between " + std::to_string(low) +
                           " and " + std::to_string(high));
    }
    return value;
}

bool parse_bool(const std::string &text) {
    if (text != "true" && text != "false") {
        throw ValueProblem("\"" + text + "\" is not true or false");
    }
    return text == "true";
}

std::string trim(const std::string &text) {
    std::size_t start = text.find_first_not_of(' ');
    if (start == std::string::npos) {
        return "";
    }
    return text.substr(start, text.find_last_not_of(' ') - start + 1);
}

std::variant<Config, std::vector<ConfigError>>
load(const std::vector<std::string> &lines) {
    Config config;
    std::map<std::string, int> first_line;
    std::vector<ConfigError> errors;
    const std::array<std::string, 4> keys{"name", "port", "debug", "timeout"};
    for (std::size_t i = 0; i < lines.size(); i++) {
        int number = static_cast<int>(i) + 1;
        std::string line = trim(lines[i]);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::size_t equals = line.find('=');
        std::string key = trim(line.substr(0, equals));
        if (equals == std::string::npos || key.empty()) {
            errors.emplace_back(SyntaxError{number});
            continue;
        }
        std::string text = trim(line.substr(equals + 1));
        if (std::find(keys.begin(), keys.end(), key) == keys.end()) {
            errors.emplace_back(UnknownKey{number, key});
        } else if (first_line.contains(key)) {
            errors.emplace_back(DuplicateKey{number, key, first_line[key]});
        } else {
            first_line[key] = number;
            try {
                if (key == "name") {
                    if (text.empty()) {
                        throw ValueProblem("must not be empty");
                    }
                    config.name = text;
                } else if (key == "port") {
                    config.port = parse_whole(text, 1, 65535);
                } else if (key == "debug") {
                    config.debug = parse_bool(text);
                } else {
                    config.timeout = parse_whole(text, 1, 3600);
                }
            } catch (const ValueProblem &problem) {
                errors.emplace_back(BadValue{number, key, problem.what()});
            }
        }
    }
    for (const std::string key : {"name", "port"}) {
        if (!first_line.contains(key)) {
            errors.emplace_back(MissingKey{key});
        }
    }
    if (!errors.empty()) {
        return errors;
    }
    return config;
}

int main() {
    std::vector<std::pair<std::string, std::vector<std::string>>> files;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.starts_with("== ")) {
            files.push_back({line.substr(3), {}});
        } else if (!files.empty()) {
            files.back().second.push_back(line);
        }
    }
    std::array<int, 5> counts{};
    int loaded = 0;
    for (const auto &[name, lines] : files) {
        auto result = load(lines);
        if (const Config *config = std::get_if<Config>(&result)) {
            loaded++;
            std::cout << name << ": ok\n"
                      << "  name = " << config->name << '\n'
                      << "  port = " << config->port << '\n'
                      << "  debug = " << (config->debug ? "true" : "false")
                      << '\n'
                      << "  timeout = " << config->timeout << '\n';
            continue;
        }
        const auto &errors = std::get<std::vector<ConfigError>>(result);
        std::cout << name << ": " << errors.size() << " error"
                  << (errors.size() == 1 ? "" : "s") << '\n';
        for (const ConfigError &error : errors) {
            std::cout << "  " << describe(error) << '\n';
            counts[error.index()]++;
        }
    }
    std::cout << "files loaded: " << loaded << " of " << files.size() << '\n';
    for (std::size_t k = 0; k < kind_names.size(); k++) {
        std::cout << kind_names[k] << ": " << counts[k]
                  << (k + 1 < kind_names.size() ? ", " : "\n");
    }
}
