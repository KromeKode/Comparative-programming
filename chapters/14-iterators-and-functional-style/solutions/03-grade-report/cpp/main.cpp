#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <ranges>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

struct Grade {
    std::string name, subject;
    int score;
};

using Groups = std::map<std::string, std::vector<Grade>>;

// The standard library has no group-by, so this fills a map of vectors. The
// key can be a member pointer such as &Grade::subject, since std::invoke can
// call those too. A std::map keeps its keys sorted.
Groups group_by(const std::vector<Grade>& grades, auto key) {
    Groups groups;
    for (const Grade& grade : grades) groups[std::invoke(key, grade)].push_back(grade);
    return groups;
}

int total(const std::vector<Grade>& grades) {
    return std::accumulate(grades.begin(), grades.end(), 0,
                           [](int sum, const Grade& g) { return sum + g.score; });
}

std::string join(const std::vector<std::string>& items, const std::string& sep) {
    std::string text;
    for (const std::string& item : items) text += (text.empty() ? "" : sep) + item;
    return text;
}

std::vector<std::string> report(const std::vector<Grade>& grades) {
    std::vector<std::string> lines{"subjects:"};
    for (const auto& [subject, group] : group_by(grades, &Grade::subject)) {
        int best = std::ranges::max(group, {}, &Grade::score).score;
        auto is_best = [best](const Grade& g) { return g.score == best; };
        auto best_names = group | std::views::filter(is_best) | std::views::transform(&Grade::name);
        // A view is lazy, so it is copied into a vector to sort it. C++23 adds
        // std::ranges::to for this step.
        std::vector<std::string> names(best_names.begin(), best_names.end());
        std::ranges::sort(names);
        int average = total(group) / static_cast<int>(group.size());
        lines.push_back("  " + subject + ": average " + std::to_string(average) + ", best " +
                        std::to_string(best) + " (" + join(names, ", ") + ")");
    }

    struct Entry {
        std::string name;
        int total;
    };
    Groups students = group_by(grades, &Grade::name);
    std::vector<Entry> ranked;
    for (const auto& [name, group] : students) ranked.push_back({name, total(group)});
    // std::tie compares field by field. Swapping a and b for the total puts the
    // highest first, and ties go by name.
    std::ranges::sort(ranked, [](const Entry& a, const Entry& b) {
        return std::tie(b.total, a.name) < std::tie(a.total, b.name);
    });
    lines.push_back("ranking:");
    for (std::size_t i = 0; i < ranked.size(); i++) {
        lines.push_back("  " + std::to_string(i + 1) + ". " + ranked[i].name + " " +
                        std::to_string(ranked[i].total));
    }

    std::vector<std::string> passed;
    for (const auto& [name, group] : students) {
        if (std::ranges::all_of(group, [](const Grade& g) { return g.score >= 50; })) {
            passed.push_back(name);
        }
    }
    lines.push_back("passed everything: " + (passed.empty() ? "nobody" : join(passed, ", ")));
    return lines;
}

int main() {
    std::vector<Grade> grades;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        Grade grade;
        if (in >> grade.name >> grade.subject >> grade.score) grades.push_back(grade);
    }
    for (const std::string& line : report(grades)) std::cout << line << "\n";
}
