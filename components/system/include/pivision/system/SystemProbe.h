#ifndef PIVISION_SYSTEM_SYSTEMPROBE_H
#define PIVISION_SYSTEM_SYSTEMPROBE_H

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

// Readings about the machine the app runs on: its own CPU use, the SoC
// temperature, and (on a Raspberry Pi) whether the firmware is throttling.
// Plain C++, no Qt. Paths take a sysfs root so tests can use a fake tree.
namespace pivision::system {

// CPU time this process used between calls, as a percentage of one core:
// 100 means one core fully busy, so a 4-core Pi can show up to 400.
class CpuUsage {
public:
    CpuUsage();

    // Usage since the previous call (or since construction).
    double sample();

    // How long the last sample() covered.
    std::chrono::duration<double> lastInterval() const { return lastInterval_; }

private:
    std::chrono::steady_clock::time_point lastWall_;
    std::chrono::microseconds lastCpu_;
    std::chrono::duration<double> lastInterval_ {};
};

// SoC temperature from the first thermal zone; empty where there is none
// (e.g. QEMU).
std::optional<double> readTemperatureCelsius(const std::filesystem::path &sysfs = "/sys");

// The Raspberry Pi firmware's get_throttled flags (same as
// `vcgencmd get_throttled`); empty on anything that isn't a Pi.
std::optional<std::uint32_t> readThrottleFlags(const std::filesystem::path &sysfs = "/sys");

// Conditions active right now, as short labels; empty when all is well.
std::vector<std::string> activeThrottleConditions(std::uint32_t flags);

// Conditions that have occurred at any point since boot.
std::vector<std::string> pastThrottleConditions(std::uint32_t flags);

} // namespace pivision::system

#endif // PIVISION_SYSTEM_SYSTEMPROBE_H
