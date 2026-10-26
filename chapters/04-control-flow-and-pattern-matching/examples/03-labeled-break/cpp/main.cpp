#include <array>
#include <iostream>
#include <optional>
#include <utility>

constexpr std::array<std::array<int, 3>, 3> grid{{{3, 8, 1}, {9, 4, 7}, {2, 6, 5}}};

// C++ has no labeled break. Putting the loops in a function lets return leave both.
std::optional<std::pair<int, int>> find(int target, int& checked) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            ++checked;
            if (grid[r][c] == target) {
                return std::pair{r, c};
            }
        }
    }
    return std::nullopt;
}

int main() {
    for (int target : {7, 10}) {
        int checked = 0;
        if (auto found = find(target, checked)) {
            auto [r, c] = *found;
            std::cout << target << ": row " << r << ", column " << c << " (checked " << checked
                      << " cells)\n";
        } else {
            std::cout << target << ": not found (checked " << checked << " cells)\n";
        }
    }
}
