#include <algorithm>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

// An empty optional means "not set", which is different from every real value.
struct Config {
    std::optional<std::string> host;
    std::optional<int> port;
    std::optional<int> timeout;
};

std::string_view trim(std::string_view text) {
    auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        return {};
    }
    auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// The number, if text is digits only and from 1 to high.
std::optional<int> parse_number(std::string_view text, int high) {
    bool digits = !text.empty() && std::ranges::all_of(text, [](char c) {
        return c >= '0' && c <= '9';
    });
    if (!digits) {
        return std::nullopt;
    }
    int value = 0;
    auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || value < 1 || value > high) {
        return std::nullopt; // error is set when the number is too big for an int
    }
    return value;
}

// Sets setting from value, or reports a bad value and leaves it alone.
void set_number(std::optional<int> &setting, std::string_view key, std::string_view value,
                int high, int number) {
    auto parsed = parse_number(value, high);
    if (value.empty()) {
        setting.reset();
    } else if (!parsed) {
        std::cout << "line " << number << ": bad " << key << " \"" << value << "\"\n";
    } else {
        setting = parsed;
    }
}

int main() {
    Config config;
    std::string buffer;
    int number = 0;
    while (std::getline(std::cin, buffer)) {
        number++;
        std::string_view line = trim(buffer);
        if (line.empty() || line.front() == '#') {
            continue;
        }
        auto equals = line.find('=');
        if (equals == std::string_view::npos) {
            std::cout << "line " << number << ": expected key = value\n";
            continue;
        }
        auto key = trim(line.substr(0, equals));
        auto value = trim(line.substr(equals + 1));
        if (key == "host") {
            config.host = value.empty() ? std::nullopt : std::optional<std::string>(value);
        } else if (key == "port") {
            set_number(config.port, key, value, 65535, number);
        } else if (key == "timeout") {
            set_number(config.timeout, key, value, 86400, number);
        } else {
            std::cout << "line " << number << ": unknown key \"" << key << "\"\n";
        }
    }

    std::cout << "host: " << config.host.value_or("localhost (default)") << '\n';
    std::cout << "port: "
              << (config.port ? std::to_string(*config.port) : "8080 (default)") << '\n';
    std::cout << "timeout: "
              << (config.timeout ? std::to_string(*config.timeout) + "s" : "none") << '\n';
}
