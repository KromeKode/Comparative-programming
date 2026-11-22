#pragma once

#include <string>
#include <string_view>

namespace duration {

// The number of seconds that text says, such as 5400 for "1h30m". Throws
// std::invalid_argument if text isn't a duration, and std::out_of_range if it is too
// long to count.
long long parse_duration(std::string_view text);

// seconds, which is at least 0, as a duration such as "1h30m" for 5400, leaving out
// the units that are 0.
std::string format_duration(long long seconds);

}  // namespace duration
