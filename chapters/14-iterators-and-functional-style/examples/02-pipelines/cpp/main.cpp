#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <string>
#include <vector>

struct Score {
    std::string name;
    int points;
};

// C++20 has no join for strings, so this writes one.
std::string join(std::ranges::input_range auto&& items, const std::string& sep) {
    std::string text;
    for (const auto& item : items) text += (text.empty() ? "" : sep) + item;
    return text;
}

const char* yes_no(bool value) { return value ? "yes" : "no"; }

int main() {
    const std::vector<Score> scores{{"ada", 91}, {"bob", 78}, {"cy", 85},
                                    {"dee", 62}, {"eve", 99}, {"finn", 54}};
    auto passed = [](const Score& score) { return score.points >= 70; };

    // Views are lazy: nothing is filtered until join loops over the result.
    std::cout << "passed: "
              << join(scores | std::views::filter(passed) | std::views::transform(&Score::name),
                      ", ")
              << "\n";

    auto passes = scores | std::views::filter(passed) | std::views::transform(&Score::points);
    std::cout << "total of the passes: " << std::accumulate(passes.begin(), passes.end(), 0)
              << "\n";

    // &Score::points is a projection: compare scores by their points.
    const Score& best = std::ranges::max(scores, {}, &Score::points);
    std::cout << "best: " << best.name << " with " << best.points << "\n";
    std::cout << "everyone passed: " << yes_no(std::ranges::all_of(scores, passed)) << "\n";
    std::cout << "anyone above 95: "
              << yes_no(std::ranges::any_of(scores, [](int p) { return p > 95; }, &Score::points))
              << "\n";

    std::vector<Score> ranked = scores;
    std::ranges::sort(ranked, std::greater{}, &Score::points);
    std::vector<std::string> top;
    for (const Score& score : ranked | std::views::take(3)) {
        top.push_back(std::to_string(top.size() + 1) + ". " + score.name + " " +
                      std::to_string(score.points));
    }
    std::cout << "top three: " << join(top, ", ") << "\n";

    std::set<char> letters;
    for (char letter : scores | std::views::transform(&Score::name) | std::views::join) {
        letters.insert(letter);
    }
    auto as_text = [](char letter) { return std::string(1, letter); };
    std::cout << "letters used: " << join(letters | std::views::transform(as_text), " ") << "\n";
}
