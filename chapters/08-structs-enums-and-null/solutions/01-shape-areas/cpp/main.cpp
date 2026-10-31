#include <charconv>
#include <format>
#include <iostream>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

struct Circle {
    double radius;
};
struct Rectangle {
    double width, height;
};
struct Triangle {
    double base, height;
};

using Shape = std::variant<Circle, Rectangle, Triangle>;

template <class... Fs> struct overloaded : Fs... {
    using Fs::operator()...;
};

double area(const Shape &shape) {
    return std::visit(overloaded{
                          [](const Circle &c) { return std::numbers::pi * c.radius * c.radius; },
                          [](const Rectangle &r) { return r.width * r.height; },
                          [](const Triangle &t) { return t.base * t.height / 2; },
                      },
                      shape);
}

double parse_size(const std::string &text) {
    double value = 0;
    const char *end = text.data() + text.size();
    auto [stop, error] = std::from_chars(text.data(), end, value);
    if (error != std::errc{} || stop != end) {
        throw std::invalid_argument(std::format("\"{}\" is not a number", text));
    }
    if (value < 0) {
        throw std::invalid_argument(std::format("\"{}\" is negative", text));
    }
    return value;
}

// Turns a shape name and its sizes into a Shape, or throws std::invalid_argument.
Shape parse(const std::string &name, const std::vector<std::string> &texts) {
    std::size_t needed = 0;
    if (name == "circle") {
        needed = 1;
    } else if (name == "rectangle" || name == "triangle") {
        needed = 2;
    } else {
        throw std::invalid_argument(std::format("unknown shape \"{}\"", name));
    }
    if (texts.size() != needed) {
        throw std::invalid_argument(std::format("{} needs {} size{}, got {}", name, needed,
                                                needed == 1 ? "" : "s", texts.size()));
    }
    std::vector<double> sizes;
    for (const auto &text : texts) {
        sizes.push_back(parse_size(text));
    }
    if (name == "circle") {
        return Circle{sizes[0]};
    }
    if (name == "rectangle") {
        return Rectangle{sizes[0], sizes[1]};
    }
    return Triangle{sizes[0], sizes[1]};
}

int main() {
    double total = 0;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream words(line);
        std::string name;
        if (!(words >> name)) {
            continue; // a blank line
        }
        std::vector<std::string> texts;
        for (std::string word; words >> word;) {
            texts.push_back(word);
        }
        try {
            Shape shape = parse(name, texts);
            total += area(shape);
            std::cout << std::format("{}: {:.2f}\n", name, area(shape));
        } catch (const std::invalid_argument &error) {
            std::cout << "error: " << error.what() << '\n';
        }
    }
    std::cout << std::format("total: {:.2f}\n", total);
}
