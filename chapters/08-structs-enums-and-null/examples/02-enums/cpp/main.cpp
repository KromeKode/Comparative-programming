#include <array>
#include <iostream>
#include <optional>
#include <string_view>

// An enum class is scoped (Light::Red) and does not turn into an int by itself.
enum class Light { Red, Green, Yellow };

// C++ cannot list an enum's values or names for you, so the program keeps its own.
constexpr std::array ALL_LIGHTS = {Light::Red, Light::Green, Light::Yellow};

std::string_view name(Light light) {
    switch (light) {
    case Light::Red:
        return "red";
    case Light::Green:
        return "green";
    case Light::Yellow:
        return "yellow";
    }
    return "?"; // an enum class can still hold any value of its underlying int
}

Light next(Light light) {
    auto number = (static_cast<int>(light) + 1) % static_cast<int>(ALL_LIGHTS.size());
    return static_cast<Light>(number);
}

std::string_view meaning(Light light) {
    switch (light) {
    case Light::Red:
        return "stop";
    case Light::Green:
        return "go";
    case Light::Yellow:
        return "slow down";
    }
    return "?";
}

int seconds(Light light) {
    switch (light) {
    case Light::Red:
        return 30;
    case Light::Green:
        return 25;
    case Light::Yellow:
        return 5;
    }
    return 0;
}

std::optional<Light> parse_light(std::string_view text) {
    for (Light light : ALL_LIGHTS) {
        if (name(light) == text) {
            return light;
        }
    }
    return std::nullopt;
}

void show_parse(std::string_view text) {
    auto light = parse_light(text);
    std::cout << "parse \"" << text << "\": " << (light ? name(*light) : "not a light")
              << '\n';
}

int main() {
    std::cout << "lights:";
    for (Light light : ALL_LIGHTS) {
        std::cout << ' ' << name(light);
    }
    std::cout << '\n';
    std::cout << "yellow is number " << static_cast<int>(Light::Yellow) << '\n';

    Light light = Light::Red;
    for (std::size_t i = 0; i < ALL_LIGHTS.size(); i++) {
        std::cout << "after " << name(light) << ": " << name(next(light)) << '\n';
        light = next(light);
    }
    std::cout << "red means " << meaning(Light::Red) << ", green means "
              << meaning(Light::Green) << ", yellow means " << meaning(Light::Yellow)
              << '\n';

    int total = 0;
    for (Light each : ALL_LIGHTS) {
        total += seconds(each);
    }
    std::cout << "one cycle: " << total << " seconds\n";

    show_parse("green");
    show_parse("blue");
}
