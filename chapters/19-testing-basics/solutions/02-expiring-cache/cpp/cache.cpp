#include "cache.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

namespace cache {

long long SystemClock::now() const {
    auto since_1970 = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(since_1970).count();
}

void Cache::put(const std::string& key, const std::string& value) {
    const long long now = clock_.now();
    std::erase_if(entries_, [now](const auto& item) { return !item.second.is_live(now); });
    entries_[key] = Entry{value, now + ttl_};
}

std::optional<std::string> Cache::get(const std::string& key) const {
    auto found = entries_.find(key);
    if (found == entries_.end() || !found->second.is_live(clock_.now())) return std::nullopt;
    return found->second.value;
}

std::size_t Cache::size() const {
    const long long now = clock_.now();
    return std::ranges::count_if(entries_,
                                 [now](const auto& item) { return item.second.is_live(now); });
}

}  // namespace cache
