#include "stock.hpp"

namespace stock {

void Stock::add(const std::string& item, long long amount) { counts_[item] += amount; }

bool Stock::take(const std::string& item, long long amount) {
    auto found = counts_.find(item);
    if (found == counts_.end() || found->second < amount) return false;
    found->second -= amount;
    return true;
}

long long Stock::count(const std::string& item) const {
    auto found = counts_.find(item);
    return found == counts_.end() ? 0 : found->second;
}

std::vector<std::pair<std::string, long long>> Stock::items() const {
    std::vector<std::pair<std::string, long long>> result;
    // A std::map keeps its keys in order, so the items come out sorted.
    for (const auto& [item, count] : counts_) {
        if (count > 0) result.emplace_back(item, count);
    }
    return result;
}

}  // namespace stock
