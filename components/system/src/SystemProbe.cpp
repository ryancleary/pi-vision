#include <pivision/system/SystemProbe.h>

#include <stdexcept>

#include "Utils.h"

namespace pivision::system {

CpuUsage::CpuUsage()
    : lastWall_(std::chrono::steady_clock::now())
    , lastCpu_(processCpuTime())
{
}

double CpuUsage::sample()
{
    const auto wall = std::chrono::steady_clock::now();
    const auto cpu = processCpuTime();
    // CPU time over wall time: 1.0 means one core was busy the whole interval.
    const std::chrono::duration<double> wallElapsed = wall - lastWall_;
    const std::chrono::duration<double> cpuElapsed = cpu - lastCpu_;
    lastWall_ = wall;
    lastCpu_ = cpu;
    lastInterval_ = wallElapsed;
    if (wallElapsed.count() <= 0.0)
        return 0.0;
    return 100.0 * cpuElapsed.count() / wallElapsed.count();
}

std::optional<double> readTemperatureCelsius(const std::filesystem::path &sysfs)
{
    // Reported in thousandths of a degree.
    const std::string text = readFirstLine(sysfs / "class/thermal/thermal_zone0/temp");
    try {
        return std::stod(text) / 1000.0;
    } catch (const std::logic_error &) { // invalid_argument or out_of_range
        return std::nullopt;
    }
}

std::optional<std::uint32_t> readThrottleFlags(const std::filesystem::path &sysfs)
{
    // Written by the Raspberry Pi firmware driver as hex, with or without "0x".
    const std::string text = readFirstLine(sysfs / "devices/platform/soc/soc:firmware/get_throttled");
    try {
        return static_cast<std::uint32_t>(std::stoul(text, nullptr, 16));
    } catch (const std::logic_error &) {
        return std::nullopt;
    }
}

std::vector<std::string> activeThrottleConditions(std::uint32_t flags)
{
    return conditionsAt(flags, 0);
}

std::vector<std::string> pastThrottleConditions(std::uint32_t flags)
{
    return conditionsAt(flags, kSinceBootShift);
}

} // namespace pivision::system
