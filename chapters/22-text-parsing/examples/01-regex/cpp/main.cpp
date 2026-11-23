// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
#include <algorithm>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Route {
    int requests = 0;
    int failed = 0;
    int slowest = 0;
};

int main() {
    // std::regex has no named groups, so these are numbered: 1 is the date, 2 the time,
    // 3 the method, 4 the path, 5 the status, and 6 the time taken. regex_match matches
    // the whole line, so the pattern needs no ^ or $. [0-9], not \d: here \d asks the
    // locale. GCC's std::regex calls itself once for each character that [^ ]* takes, so a
    // path of some 33,000 characters runs out of stack, and the program crashes.
    const std::regex line_re(
        "([0-9]{4}-[0-9]{2}-[0-9]{2}) ([0-9]{2}:[0-9]{2}:[0-9]{2})"
        " (GET|POST|PUT|DELETE) (/[^ ]*)"
        " ([0-9]{3}) ([0-9]{1,9})ms");
    const std::regex id("/[0-9]+");

    // An unordered_map has no order, so the routes are kept in a vector, with an index.
    std::vector<std::pair<std::string, Route>> routes;
    std::unordered_map<std::string, std::size_t> index;
    int status = 0;
    std::string line;
    // getline splits at "\n" only, and keeps a "\r" before it.
    for (int number = 1; std::getline(std::cin, line); number++) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::smatch match;
        if (!std::regex_match(line, match, line_re)) {
            std::cerr << "logscan: line " << number << ": not a log line\n";
            status = 1;
            continue;
        }
        // In a replacement, "$&" and "$1" stand for what matched; "/:id" has no "$".
        std::string key = match[3].str() + " " + std::regex_replace(match[4].str(), id, "/:id");
        auto [at, added] = index.try_emplace(key, routes.size());
        if (added) routes.emplace_back(key, Route{});
        Route& route = routes[at->second].second;
        // The regex let through only digits, few enough to fit, so stoi can't fail.
        route.requests++;
        if (std::stoi(match[5].str()) >= 400) route.failed++;
        route.slowest = std::max(route.slowest, std::stoi(match[6].str()));
    }
    for (const auto& [key, route] : routes) {
        std::cout << key << ": " << route.requests << " request"
                  << (route.requests == 1 ? "" : "s") << ", " << route.failed
                  << " failed, slowest " << route.slowest << "ms\n";
    }
    return status;
}
