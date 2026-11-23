// Copies stdin to stdout, with each date in it written as YYYY-MM-DD.
#include <array>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <regex>
#include <string>

static const std::array<std::string, 12> months = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December",
};

static int days_in(int month, int year) {
    if (month == 2) return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) ? 29 : 28;
    return month == 4 || month == 6 || month == 9 || month == 11 ? 30 : 31;
}

static int month_of(const std::string& name) {
    for (std::size_t i = 0; i < months.size(); i++) {
        if (months[i].starts_with(name)) return static_cast<int>(i) + 1;
    }
    return 0;
}

// The date as YYYY-MM-DD, or the text as it was if it isn't a real date.
static std::string rewrite(const std::smatch& g) {
    // Only one of the three forms matched; matched says which groups took part.
    int day, month, year;
    if (g[1].matched) {
        day = std::stoi(g[1]), month = std::stoi(g[2]), year = std::stoi(g[3]);
    } else if (g[4].matched) {
        day = std::stoi(g[5]), month = month_of(g[4]), year = std::stoi(g[6]);
    } else {
        day = std::stoi(g[7]), month = month_of(g[8]), year = std::stoi(g[9]);
    }
    if (month < 1 || month > 12 || day < 1 || day > days_in(month, year)) {
        return g.str();  // not a real date, so left as it was
    }
    char out[11];
    std::snprintf(out, sizeof out, "%04d-%02d-%02d", year, month, day);
    return out;
}

int main() {
    std::string month;
    for (const std::string& name : months) {
        // Each month's name, or its first three letters: "Jan(?:uary)?", and so on.
        if (!month.empty()) month += "|";
        month += name.size() > 3 ? name.substr(0, 3) + "(?:" + name.substr(3) + ")?" : name;
    }
    const std::string d = "([0-9]{1,2})", m = d, y = "([0-9]{4})";
    // std::regex reads bytes. Its \b looks at the letters, digits, and _ of the C locale,
    // which are ASCII's, so the bytes of "é" aren't letters to it.
    const std::regex date("\\b(?:" + d + "/" + m + "/" + y + "|(" + month + ") " + d + ", " +
                          y + "|" + d + " (" + month + ") " + y + ")\\b");
    std::string text{std::istreambuf_iterator<char>(std::cin), {}};
    // regex_replace can only put in a fixed text, with $1 and the like, so this copies the
    // text between the matches, and each match rewritten.
    std::string out;
    auto last = text.cbegin();
    for (std::sregex_iterator it(text.begin(), text.end(), date), end; it != end; ++it) {
        out.append(last, (*it)[0].first);
        out += rewrite(*it);
        last = (*it)[0].second;
    }
    out.append(last, text.cend());
    std::cout << out;
}
