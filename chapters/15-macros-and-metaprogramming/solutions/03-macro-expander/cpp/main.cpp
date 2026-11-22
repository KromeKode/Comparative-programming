#include <algorithm>
#include <cctype>
#include <cstddef>
#include <format>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct Macro {
    std::optional<std::vector<std::string>> params;  // none for a macro with no parentheses
    std::string body;
};

struct ExpandError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

// std::less<> lets find and contains take a string_view, without making a
// string from it first.
using Names = std::set<std::string, std::less<>>;
std::map<std::string, Macro, std::less<>> macros;

constexpr auto npos = std::string_view::npos;

bool is_word_char(char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; }

std::size_t word_end(std::string_view text, std::size_t start) {
    while (start < text.size() && is_word_char(text[start])) ++start;
    return start;
}

bool is_name(std::string_view word) {
    return !word.empty() && !std::isdigit(static_cast<unsigned char>(word[0])) &&
           word_end(word, 0) == word.size();
}

// The index just past the string that starts at text[start], a quote.
std::size_t string_end(std::string_view text, std::size_t start) {
    std::size_t close = text.find('"', start + 1);
    return close == npos ? text.size() : close + 1;
}

std::string_view trim(std::string_view text) {
    constexpr std::string_view spaces = " \t\r\n\f\v";
    std::size_t start = text.find_first_not_of(spaces);
    if (start == npos) return {};
    return text.substr(start, text.find_last_not_of(spaces) - start + 1);
}

// Splits the arguments of a call at its top-level commas, and trims them.
// Returns them with the index just past the ")".
std::pair<std::vector<std::string_view>, std::size_t> split_args(std::string_view text,
                                                                 std::size_t open,
                                                                 std::string_view name) {
    std::vector<std::string_view> args;
    int depth = 0;
    std::size_t start = open + 1, i = open + 1;
    while (i < text.size()) {
        char c = text[i];
        if (c == '"') {
            i = string_end(text, i);
            continue;
        }
        if (c == '(') {
            ++depth;
        } else if (c == ')' && depth > 0) {
            --depth;
        } else if (c == ')' || (c == ',' && depth == 0)) {
            args.push_back(trim(text.substr(start, i - start)));
            start = i + 1;
            if (c == ')') return {args, i + 1};
        }
        ++i;
    }
    throw ExpandError(std::format("unterminated call to {}", name));
}

std::string expand(std::string_view text, const Names& disabled);

// The macro's body, with each parameter replaced by its argument.
std::string substitute(const Macro& macro, const std::vector<std::string_view>& args,
                       const Names& disabled) {
    std::map<std::string_view, std::string_view> raw;
    for (std::size_t p = 0; p < args.size(); ++p) raw[(*macro.params)[p]] = args[p];
    std::map<std::string_view, std::string> expanded;  // an argument is expanded only if used
    std::string out;
    std::string_view body = macro.body;
    std::size_t i = 0;
    while (i < body.size()) {
        std::size_t end = i + 1;
        if (body[i] == '"') {
            end = string_end(body, i);
        } else if (body[i] == '#') {
            // # and a parameter make a string of the argument as written.
            std::size_t start = std::min(body.find_first_not_of(' ', i + 1), body.size());
            std::size_t stop = word_end(body, start);
            if (auto arg = raw.find(body.substr(start, stop - start)); arg != raw.end()) {
                out += std::format("\"{}\"", arg->second);
                i = stop;
                continue;
            }
        } else if (is_word_char(body[i])) {
            end = word_end(body, i);
            std::string_view word = body.substr(i, end - i);
            if (auto arg = raw.find(word); arg != raw.end()) {
                auto [cached, added] = expanded.try_emplace(word);
                if (added) cached->second = expand(arg->second, disabled);
                out += cached->second;
                i = end;
                continue;
            }
        }
        out += body.substr(i, end - i);
        i = end;
    }
    return out;
}

// Expands every macro in text, except those in disabled.
std::string expand(std::string_view text, const Names& disabled) {
    std::string out;
    std::size_t i = 0;
    while (i < text.size()) {
        std::size_t end = i + 1;
        if (text[i] == '"') {
            end = string_end(text, i);
        } else if (is_word_char(text[i])) {
            end = word_end(text, i);
            std::string_view word = text.substr(i, end - i);
            auto found = macros.find(word);
            if (found != macros.end() && !disabled.contains(word)) {
                const Macro& macro = found->second;
                // Inside its own expansion, a macro's name is left alone.
                Names inner = disabled;
                inner.emplace(word);
                if (!macro.params) {
                    out += expand(macro.body, inner);
                    i = end;
                    continue;
                }
                std::size_t paren = text.find_first_not_of(' ', end);
                if (paren != npos && text[paren] == '(') {
                    auto [args, after] = split_args(text, paren, word);
                    if (macro.params->empty() && args.size() == 1 && args[0].empty()) args.clear();
                    if (args.size() != macro.params->size()) {
                        std::size_t count = macro.params->size();
                        throw ExpandError(std::format("{} takes {} argument{}, got {}", word, count,
                                                      count == 1 ? "" : "s", args.size()));
                    }
                    out += expand(substitute(macro, args, disabled), inner);
                    i = after;
                    continue;
                }
            }
        }
        out += text.substr(i, end - i);
        i = end;
    }
    return out;
}

void define(std::string_view rest) {
    std::size_t end = word_end(rest, 0);
    std::string name(rest.substr(0, end));
    if (!is_name(name)) throw ExpandError("bad #define");
    if (end == rest.size() || rest[end] != '(') {
        macros[name] = Macro{std::nullopt, std::string(trim(rest.substr(end)))};
        return;
    }
    std::size_t close = rest.find(')', end);
    if (close == npos) throw ExpandError("bad #define");
    std::string_view inside = rest.substr(end + 1, close - end - 1);
    std::vector<std::string> params;
    for (std::size_t start = 0; !trim(inside).empty() && start <= inside.size();) {
        std::size_t comma = std::min(inside.find(',', start), inside.size());
        params.emplace_back(trim(inside.substr(start, comma - start)));
        start = comma + 1;
    }
    Names unique(params.begin(), params.end());
    if (!std::ranges::all_of(params, is_name) || unique.size() != params.size()) {
        throw ExpandError("bad #define");
    }
    macros[name] = Macro{params, std::string(trim(rest.substr(close + 1)))};
}

// Returns the line to print, or nothing for a directive.
std::optional<std::string> run(std::string_view line) {
    std::string_view whole = trim(line);
    std::size_t space = std::min(whole.find(' '), whole.size());
    std::string_view directive = whole.substr(0, space);
    std::string_view rest = trim(whole.substr(space));
    if (directive == "#define") {
        define(rest);
        return std::nullopt;
    }
    if (directive == "#undef") {
        if (!is_name(rest)) throw ExpandError("bad #undef");
        if (auto found = macros.find(rest); found != macros.end()) macros.erase(found);
        return std::nullopt;
    }
    return expand(line, {});
}

int main() {
    for (std::string line; std::getline(std::cin, line);) {
        try {
            if (auto result = run(line)) std::cout << *result << '\n';
        } catch (const ExpandError& error) {
            std::cout << "error: " << error.what() << '\n';
        }
    }
}
