#include <pivision/system/SystemProbe.h>

#include <array>
#include <bitset>
#include <fstream>
#include <stdexcept>

#include <sys/resource.h>

namespace pivision::system {

namespace {

    // Throttle flag bits, from the Raspberry Pi documentation for
    // `vcgencmd get_throttled`. The "since boot" copy of each condition sits
    // 16 bits higher.
    constexpr int kSinceBootShift = 16;
    struct Condition {
        int bit;
        const char *label;
    };
    constexpr std::array<Condition, 4> kConditions { {
        { 0, "under-voltage" },
        { 1, "frequency capped" },
        { 2, "throttled" },
        { 3, "soft temperature limit" },
    } };

    std::vector<std::string> conditionsAt(std::uint32_t flags, int shift)
    {
        const std::bitset<32> bits(flags);
        std::vector<std::string> labels;
        for (const Condition &condition : kConditions) {
            if (bits.test(static_cast<std::size_t>(condition.bit + shift)))
                labels.emplace_back(condition.label);
        }
        return labels;
    }

    // First line of a sysfs file; empty if it can't be read.
    std::string readLine(const std::filesystem::path &file)
    {
        std::ifstream in(file);
        std::string line;
        std::getline(in, line);
        return line;
    }

    // User plus system CPU time this process has used so far.
    std::chrono::microseconds processCpuTime()
    {
        rusage usage {};
        getrusage(RUSAGE_SELF, &usage);
        const auto toDuration = [](const timeval &t) {
            return std::chrono::seconds(t.tv_sec) + std::chrono::microseconds(t.tv_usec);
        };
        return toDuration(usage.ru_utime) + toDuration(usage.ru_stime);
    }

} // namespace

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
    if (wallElapsed.count() <= 0.0)
        return 0.0;
    return 100.0 * cpuElapsed.count() / wallElapsed.count();
}

std::optional<double> readTemperatureCelsius(const std::filesystem::path &sysfs)
{
    // Reported in thousandths of a degree.
    const std::string text = readLine(sysfs / "class/thermal/thermal_zone0/temp");
    try {
        return std::stod(text) / 1000.0;
    } catch (const std::logic_error &) { // invalid_argument or out_of_range
        return std::nullopt;
    }
}

std::optional<std::uint32_t> readThrottleFlags(const std::filesystem::path &sysfs)
{
    // Written by the Raspberry Pi firmware driver as hex, with or without "0x".
    const std::string text = readLine(sysfs / "devices/platform/soc/soc:firmware/get_throttled");
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
