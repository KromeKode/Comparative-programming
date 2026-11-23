// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. C++'s standard library has nothing to read options with, so this reads them
// itself, the way getopt_long does: -t f, -tf, --to f, and --to=f all work, options
// can come after the temperatures, and -- ends them.
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <format>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

const std::string_view usage = "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...";
const std::string_view help = R"(
Converts temperatures, such as 21.5c, -40f, or 300k.

options:
  -h, --help           show this help and exit
  -t, --to UNIT        the unit to convert to: c, f, or k (default: c)
  -d, --digits DIGITS  digits after the point, 0 to 9 (default: 1))";

struct Temperature {
    std::string text;
    double value;
    char unit;
};

struct CommandLine {
    char to = 'c';
    int digits = 1;
    std::vector<std::string> temperatures;
};

double to_celsius(double t, char unit) {
    switch (unit) {
    case 'f': return (t - 32) * 5 / 9;
    case 'k': return t - 273.15;
    default: return t;
    }
}

double from_celsius(double t, char unit) {
    switch (unit) {
    case 'f': return t * 9 / 5 + 32;
    case 'k': return t + 273.15;
    default: return t;
    }
}

bool is_unit(std::string_view text) {
    return text == "c" || text == "f" || text == "k";
}

// Prints a message and the usage line to stderr, and exits with status 2.
[[noreturn]] void usage_error(std::string_view message) {
    std::cerr << "temp: " << message << '\n' << usage << '\n';
    std::exit(2);
}

CommandLine parse_command_line(const std::vector<std::string>& args) {
    CommandLine command_line;
    for (std::size_t i = 0; i < args.size(); i++) {
        const std::string& arg = args[i];
        if (arg == "--") {  // everything after -- is a temperature
            command_line.temperatures.insert(command_line.temperatures.end(),
                                             args.begin() + i + 1, args.end());
            break;
        }
        if (arg.size() < 2 || arg[0] != '-') {  // not an option; "-" alone isn't either
            command_line.temperatures.push_back(arg);
            continue;
        }
        // Split --to=f into --to and f, and -tf into -t and f.
        std::string name = arg;
        std::optional<std::string> value;
        if (arg.starts_with("--")) {
            if (auto equals = arg.find('='); equals != std::string::npos) {
                name = arg.substr(0, equals);
                value = arg.substr(equals + 1);
            }
        } else if (arg.size() > 2) {
            name = arg.substr(0, 2);
            value = arg.substr(2);
        }
        if (name == "-h" || name == "--help") {
            std::cout << usage << '\n' << help << '\n';
            std::exit(0);
        }
        if (name != "-t" && name != "--to" && name != "-d" && name != "--digits") {
            usage_error("unknown option " + arg);
        }
        if (!value) {  // the value is the next argument
            if (i + 1 == args.size()) usage_error("option " + name + " needs a value");
            value = args[++i];
        }
        if (name == "-t" || name == "--to") {
            if (!is_unit(*value)) usage_error("unknown unit: " + *value);
            command_line.to = (*value)[0];
        } else {
            if (value->size() != 1 || (*value)[0] < '0' || (*value)[0] > '9') {
                usage_error("digits must be 0 to 9, not " + *value);
            }
            command_line.digits = (*value)[0] - '0';
        }
    }
    if (command_line.temperatures.empty()) usage_error("no temperatures");
    return command_line;
}

// A number followed by c, f, or k, or nothing.
std::optional<Temperature> parse_temperature(const std::string& text) {
    if (text.size() < 2 || !is_unit(text.substr(text.size() - 1))) return std::nullopt;
    const char* first = text.data();
    const char* last = text.data() + text.size() - 1;  // the unit
    double value;
    // from_chars reads the whole number or fails; it skips no spaces and allows no +.
    auto [end, error] = std::from_chars(first, last, value);
    if (error != std::errc{} || end != last || !std::isfinite(value)) return std::nullopt;
    return Temperature{text, value, text.back()};
}

int main(int argc, char** argv) {
    CommandLine command_line = parse_command_line(std::vector<std::string>(argv + 1, argv + argc));

    // Check every temperature before printing any, so that a mistake prints nothing.
    std::vector<Temperature> temperatures;
    for (const std::string& text : command_line.temperatures) {
        std::optional<Temperature> temperature = parse_temperature(text);
        if (!temperature) {
            std::cerr << "temp: not a temperature: " << text << '\n';
            return 2;
        }
        temperatures.push_back(*temperature);
    }

    for (const Temperature& t : temperatures) {
        double converted = from_celsius(to_celsius(t.value, t.unit), command_line.to);
        std::cout << std::format("{} = {:.{}f}{}\n", t.text, converted, command_line.digits,
                                 command_line.to);
    }
}
