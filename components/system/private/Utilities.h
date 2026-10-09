#ifndef PIVISION_SYSTEM_UTILITIES_H
#define PIVISION_SYSTEM_UTILITIES_H

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace pivision::system {

// One throttle condition: its bit in the get_throttled flags and its label.
struct ThrottleCondition {
    int bit;
    const char *label;
};

// Throttle flag bits, from the Raspberry Pi documentation for
// `vcgencmd get_throttled`. The "since boot" copy of each condition sits
// kSinceBootShift bits higher.
constexpr int kSinceBootShift = 16;
constexpr std::array<ThrottleCondition, 4> kThrottleConditions { {
    { 0, "under-voltage" },
    { 1, "frequency capped" },
    { 2, "throttled" },
    { 3, "soft temperature limit" },
} };

// Labels of the conditions whose bit, moved up by `shift`, is set in `flags`.
std::vector<std::string> conditionsAt(std::uint32_t flags, int shift);

// First line of a sysfs file; empty if it can't be read.
std::string readFirstLine(const std::filesystem::path &file);

// User plus system CPU time this process has used so far.
std::chrono::microseconds processCpuTime();

} // namespace pivision::system

#endif // PIVISION_SYSTEM_UTILITIES_H
