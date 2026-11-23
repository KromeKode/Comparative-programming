#include "roman.hpp"

#include <stdexcept>
#include <utility>

namespace roman {

namespace {

const std::pair<int, const char*> NUMERALS[] = {
    {1000, "M"},
    {900, "CM"},
    {500, "D"},
    {400, "CD"},
    {100, "C"},
    {90, "XC"},
    {50, "L"},
    {40, "XL"},
    {10, "X"},
    {9, "IX"},
    {5, "V"},
    {4, "IV"},
    {1, "I"},
};

}  // namespace

std::string to_roman(int n) {
    if (n < 1 || n > 3999) {
        throw std::out_of_range(std::to_string(n) + " is out of range");
    }
    std::string numeral;
    for (const auto& [value, letters] : NUMERALS) {
        for (; n >= value; n -= value) numeral += letters;
    }
    return numeral;
}

}  // namespace roman
