#include "duration.hpp"

#include <charconv>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace duration {

namespace {

constexpr std::string_view UNITS = "dhms";  // largest first, which is the order they come in
constexpr long long SECONDS[] = {86400, 3600, 60, 1};  // in each of the units

std::invalid_argument not_a_duration(std::string_view text, const char* why) {
    return std::invalid_argument("\"" + std::string(text) + "\" " + why);
}

}  // namespace

long long parse_duration(std::string_view text) {
    if (text.empty()) throw not_a_duration(text, "is empty");
    long long total = 0;
    std::size_t next_unit = 0;  // each part's unit comes after the one before it in UNITS
    std::size_t i = 0;
    while (i < text.size()) {
        std::size_t start = i;
        while (i < text.size() && '0' <= text[i] && text[i] <= '9') i++;
        if (i == start) throw not_a_duration(text, "has a part with no number");
        if (i == text.size()) throw not_a_duration(text, "ends with a number with no unit");
        long long number = 0;
        if (std::from_chars(text.data() + start, text.data() + i, number).ec != std::errc()) {
            throw std::out_of_range("too long to count");
        }
        std::size_t unit = UNITS.find(text[i]);
        if (unit == std::string_view::npos) throw not_a_duration(text, "has a letter not a unit");
        if (unit < next_unit) throw not_a_duration(text, "has a unit out of order, or twice");
        if (number > (std::numeric_limits<long long>::max() - total) / SECONDS[unit]) {
            throw std::out_of_range("too long to count");
        }
        total += number * SECONDS[unit];
        next_unit = unit + 1;
        i++;
    }
    return total;
}

std::string format_duration(long long seconds) {
    if (seconds == 0) return "0s";
    std::string text;
    for (std::size_t unit = 0; unit < UNITS.size(); unit++) {
        long long amount = seconds / SECONDS[unit];
        seconds %= SECONDS[unit];
        if (amount > 0) text += std::to_string(amount) + UNITS[unit];
    }
    return text;
}

}  // namespace duration
