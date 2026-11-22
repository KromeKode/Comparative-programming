// Versions and ranges as npm writes them: 1.2.3-beta.1, ^1.2, ~1.2.3, >=1 <2, 1.x || 3.
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace semver {

struct Version {
    long long major = 0, minor = 0, patch = 0;
    std::vector<std::string> pre;  // the pre-release's identifiers, if it has one

    std::string str() const;
};

// The version that text is, or nothing if it isn't one.
std::optional<Version> parse_version(std::string_view text);
// Negative if a comes before b, 0 if they are equal, and positive if after.
int compare(const Version& a, const Version& b);

class Range {
public:
    // The range that text is. Throws std::invalid_argument, saying why, if it
    // isn't one.
    explicit Range(std::string_view text);

    bool allows(const Version& version) const;
    // The highest of versions that the range allows, or nothing if it allows none.
    std::optional<Version> highest(const std::vector<Version>& versions) const;

private:
    enum class Op { eq, lt, le, gt, ge };
    struct Simple {
        Op op;
        Version bound;
    };
    // Sets of simple comparators, joined by ||. A version is in the range if it
    // passes every comparator of some set.
    std::vector<std::vector<Simple>> sets_;

    static std::optional<std::vector<Simple>> expand(std::string_view token);
    static bool holds(const Version& version, const Simple& simple);
};

}  // namespace semver
