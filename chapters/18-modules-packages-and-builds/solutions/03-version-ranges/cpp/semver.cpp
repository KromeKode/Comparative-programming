#include "semver.hpp"

#include <algorithm>
#include <stdexcept>
#include <tuple>

namespace semver {

// An unnamed namespace makes its names private to this file, as static does in C.
namespace {

bool all_digits(std::string_view text) {
    return std::all_of(text.begin(), text.end(), [](char c) { return c >= '0' && c <= '9'; });
}

// A number in a version: 0, or up to 15 digits that don't start with 0.
std::optional<long long> parse_number(std::string_view text) {
    if (text.empty() || text.size() > 15 || !all_digits(text)) return std::nullopt;
    if (text.size() > 1 && text[0] == '0') return std::nullopt;
    return std::stoll(std::string(text));
}

std::vector<std::string_view> split(std::string_view text, char separator) {
    std::vector<std::string_view> parts;
    while (true) {
        std::size_t at = text.find(separator);
        parts.push_back(text.substr(0, at));
        if (at == std::string_view::npos) return parts;
        text.remove_prefix(at + 1);
    }
}

// A pre-release is identifiers separated by dots. Each is digits without a 0 in
// front, or letters, digits, and hyphens with at least one that isn't a digit.
std::optional<std::vector<std::string>> parse_pre(std::string_view text) {
    std::vector<std::string> identifiers;
    for (std::string_view identifier : split(text, '.')) {
        bool ok = !identifier.empty() &&
                  std::all_of(identifier.begin(), identifier.end(), [](char c) {
                      return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
                             (c >= 'A' && c <= 'Z') || c == '-';
                  });
        if (!ok || (identifier.size() > 1 && identifier[0] == '0' && all_digits(identifier))) {
            return std::nullopt;
        }
        identifiers.emplace_back(identifier);
    }
    return identifiers;
}

// Numbers compare as numbers, and come before other identifiers, which compare
// character by character. Numbers have no 0 in front, so a longer one is larger.
int compare_identifiers(const std::string& a, const std::string& b) {
    bool a_number = all_digits(a), b_number = all_digits(b);
    if (a_number != b_number) return a_number ? -1 : 1;
    if (a_number && a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    return a.compare(b) < 0 ? -1 : a.compare(b) > 0 ? 1 : 0;
}

bool same_core(const Version& a, const Version& b) {
    return a.major == b.major && a.minor == b.minor && a.patch == b.patch;
}

bool is_wild(std::string_view part) { return part == "x" || part == "X" || part == "*"; }

}  // namespace

std::string Version::str() const {
    std::string text =
        std::to_string(major) + '.' + std::to_string(minor) + '.' + std::to_string(patch);
    for (std::size_t i = 0; i < pre.size(); i++) text += (i == 0 ? "-" : ".") + pre[i];
    return text;
}

std::optional<Version> parse_version(std::string_view text) {
    // The core is everything before the first '-', and the pre-release after it.
    std::size_t dash = text.find('-');
    std::vector<std::string_view> parts = split(text.substr(0, dash), '.');
    if (parts.size() != 3) return std::nullopt;
    Version version;
    long long* numbers[] = {&version.major, &version.minor, &version.patch};
    for (int i = 0; i < 3; i++) {
        std::optional<long long> number = parse_number(parts[i]);
        if (!number) return std::nullopt;
        *numbers[i] = *number;
    }
    if (dash != std::string_view::npos) {
        std::optional<std::vector<std::string>> pre = parse_pre(text.substr(dash + 1));
        if (!pre) return std::nullopt;
        version.pre = *pre;
    }
    return version;
}

int compare(const Version& a, const Version& b) {
    if (!same_core(a, b)) {
        return std::tie(a.major, a.minor, a.patch) < std::tie(b.major, b.minor, b.patch) ? -1
                                                                                         : 1;
    }
    // A version without a pre-release comes after the same version with one.
    if (a.pre.empty() || b.pre.empty()) return a.pre.empty() - b.pre.empty();
    for (std::size_t i = 0; i < a.pre.size() && i < b.pre.size(); i++) {
        if (int order = compare_identifiers(a.pre[i], b.pre[i]); order != 0) return order;
    }
    return (a.pre.size() > b.pre.size()) - (a.pre.size() < b.pre.size());
}

std::optional<std::vector<Range::Simple>> Range::expand(std::string_view token) {
    char kind = ' ';
    for (std::string_view prefix : {"<=", ">=", "<", ">", "=", "^", "~"}) {
        if (token.starts_with(prefix)) {
            kind = prefix == "<=" ? 'l' : prefix == ">=" ? 'g' : prefix[0];
            token.remove_prefix(prefix.size());
            break;
        }
    }
    std::size_t dash = token.find('-');
    std::vector<std::string_view> parts = split(token.substr(0, dash), '.');
    if (parts.size() > 3) return std::nullopt;
    // known counts the numbers, which must come before any wildcards.
    long long nums[3] = {0, 0, 0};
    std::size_t known = 0;
    for (std::size_t i = 0; i < parts.size(); i++) {
        if (is_wild(parts[i])) continue;
        std::optional<long long> number = parse_number(parts[i]);
        if (!number || known != i) return std::nullopt;
        nums[known++] = *number;
    }
    Version low{nums[0], nums[1], nums[2], {}};
    if (dash != std::string_view::npos) {
        // A pre-release can follow only three numbers.
        std::optional<std::vector<std::string>> pre = parse_pre(token.substr(dash + 1));
        if (known < 3 || !pre) return std::nullopt;
        low.pre = *pre;
    }
    // The first version after all those whose parts up to level match: the -0
    // pre-release of the next major, minor, or patch.
    auto after = [&](std::size_t level) {
        if (level == 0) return Version{nums[0] + 1, 0, 0, {"0"}};
        if (level == 1) return Version{nums[0], nums[1] + 1, 0, {"0"}};
        return Version{nums[0], nums[1], nums[2] + 1, {"0"}};
    };
    if (known == 0) {
        // Any version: nothing is outside it, and nothing is above or below it.
        if (kind == '<' || kind == '>') return std::vector<Simple>{{Op::lt, {0, 0, 0, {"0"}}}};
        return std::vector<Simple>{};
    }
    Version next = after(known - 1);
    switch (kind) {
        case ' ':
        case '=':
            if (known == 3) return std::vector<Simple>{{Op::eq, low}};
            return std::vector<Simple>{{Op::ge, low}, {Op::lt, next}};
        case '>':
            if (known == 3) return std::vector<Simple>{{Op::gt, low}};
            next.pre.clear();
            return std::vector<Simple>{{Op::ge, next}};
        case 'g':
            return std::vector<Simple>{{Op::ge, low}};
        case '<':
            if (known < 3) low.pre = {"0"};
            return std::vector<Simple>{{Op::lt, low}};
        case 'l':
            if (known == 3) return std::vector<Simple>{{Op::le, low}};
            return std::vector<Simple>{{Op::lt, next}};
        case '~':
            return std::vector<Simple>{{Op::ge, low}, {Op::lt, after(known == 1 ? 0 : 1)}};
        default: {
            // ^ allows changes that don't change the first part that isn't 0.
            std::size_t level = known - 1;
            for (std::size_t i = known; i-- > 0;) {
                if (nums[i] != 0) level = i;
            }
            return std::vector<Simple>{{Op::ge, low}, {Op::lt, after(level)}};
        }
    }
}

Range::Range(std::string_view text) {
    while (true) {
        std::size_t at = text.find("||");
        std::string_view side = text.substr(0, at);
        std::vector<Simple>& simples = sets_.emplace_back();
        bool any = false;
        while (true) {
            std::size_t start = side.find_first_not_of(" \t\n\v\f\r");
            if (start == std::string_view::npos) break;
            side.remove_prefix(start);
            std::size_t end = std::min(side.find_first_of(" \t\n\v\f\r"), side.size());
            std::string_view token = side.substr(0, end);
            std::optional<std::vector<Simple>> expanded = expand(token);
            if (!expanded) throw std::invalid_argument(std::string(token) + " isn't a comparator");
            simples.insert(simples.end(), expanded->begin(), expanded->end());
            any = true;
            side.remove_prefix(end);
        }
        if (!any) throw std::invalid_argument("each side of || needs a comparator");
        if (at == std::string_view::npos) return;
        text.remove_prefix(at + 2);
    }
}

bool Range::holds(const Version& version, const Simple& simple) {
    int order = compare(version, simple.bound);
    switch (simple.op) {
        case Op::eq: return order == 0;
        case Op::lt: return order < 0;
        case Op::le: return order <= 0;
        case Op::gt: return order > 0;
        case Op::ge: return order >= 0;
    }
    return false;
}

bool Range::allows(const Version& version) const {
    for (const std::vector<Simple>& simples : sets_) {
        bool passes = std::all_of(simples.begin(), simples.end(),
                                  [&](const Simple& simple) { return holds(version, simple); });
        // A pre-release is in only if a comparator of the set names a pre-release of
        // the same major, minor, and patch.
        bool named = std::any_of(simples.begin(), simples.end(), [&](const Simple& simple) {
            return !simple.bound.pre.empty() && same_core(simple.bound, version);
        });
        if (passes && (version.pre.empty() || named)) return true;
    }
    return false;
}

std::optional<Version> Range::highest(const std::vector<Version>& versions) const {
    std::optional<Version> best;
    for (const Version& version : versions) {
        if (allows(version) && (!best || compare(version, *best) > 0)) best = version;
    }
    return best;
}

}  // namespace semver
