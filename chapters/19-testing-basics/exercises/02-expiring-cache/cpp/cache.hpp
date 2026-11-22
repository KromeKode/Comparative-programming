#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

namespace cache {

// Something that tells the time, in whole seconds.
class Clock {
public:
    virtual ~Clock() = default;
    virtual long long now() const = 0;
};

// The computer's clock, which counts the seconds since 1970.
class SystemClock : public Clock {
public:
    long long now() const override;
};

// Keeps each value for ttl seconds after it was put, and then forgets it. The cache
// keeps a reference to its clock, so the clock must outlive it.
class Cache {
public:
    Cache(long long ttl, const Clock& clock) : ttl_(ttl), clock_(clock) {}

    // Stores value under key for the next ttl seconds, replacing any value there.
    void put(const std::string& key, const std::string& value);

    // The value under key, or nothing if there is none or it has expired.
    std::optional<std::string> get(const std::string& key) const;

    // How many entries haven't expired.
    std::size_t size() const;

private:
    struct Entry {
        std::string value;
        long long expires;  // the time the entry stops being live

        bool is_live(long long now) const { return now < expires; }
    };

    long long ttl_;
    const Clock& clock_;
    std::unordered_map<std::string, Entry> entries_;
};

}  // namespace cache
