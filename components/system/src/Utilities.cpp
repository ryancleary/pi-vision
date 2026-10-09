#include "Utilities.h"

#include <bitset>
#include <fstream>

#include <sys/resource.h>

namespace pivision::system {

std::vector<std::string> conditionsAt(std::uint32_t flags, int shift)
{
    const std::bitset<32> bits(flags);
    std::vector<std::string> labels;
    for (const ThrottleCondition &condition : kThrottleConditions) {
        if (bits.test(static_cast<std::size_t>(condition.bit + shift)))
            labels.emplace_back(condition.label);
    }
    return labels;
}

std::string readFirstLine(const std::filesystem::path &file)
{
    std::ifstream in(file);
    std::string line;
    std::getline(in, line);
    return line;
}

std::chrono::microseconds processCpuTime()
{
    rusage usage {};
    getrusage(RUSAGE_SELF, &usage);
    const auto toDuration = [](const timeval &t) {
        return std::chrono::seconds(t.tv_sec) + std::chrono::microseconds(t.tv_usec);
    };
    return toDuration(usage.ru_utime) + toDuration(usage.ru_stime);
}

} // namespace pivision::system
