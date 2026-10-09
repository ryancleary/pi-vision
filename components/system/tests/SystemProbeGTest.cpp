#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include <pivision/system/SystemProbe.h>

namespace pivision::system {

namespace {

    // A throwaway sysfs tree for one test.
    class FakeSysfs {
    public:
        explicit FakeSysfs(const std::string &name)
            : root_(std::filesystem::path(::testing::TempDir()) / ("pivision-sysfs-" + name))
        {
            std::filesystem::remove_all(root_);
        }
        ~FakeSysfs() { std::filesystem::remove_all(root_); }
        FakeSysfs(const FakeSysfs &) = delete;
        FakeSysfs &operator=(const FakeSysfs &) = delete;

        void write(const std::string &relative, const std::string &content) const
        {
            const std::filesystem::path file = root_ / relative;
            std::filesystem::create_directories(file.parent_path());
            std::ofstream(file) << content << '\n';
        }

        const std::filesystem::path &root() const { return root_; }

    private:
        std::filesystem::path root_;
    };

} // namespace

TEST(SystemProbeGTest, TemperatureIsInDegrees)
{
    const FakeSysfs sysfs("temp");
    sysfs.write("class/thermal/thermal_zone0/temp", "48312");
    const auto celsius = readTemperatureCelsius(sysfs.root());
    ASSERT_TRUE(celsius.has_value());
    EXPECT_NEAR(*celsius, 48.312, 1e-9);
}

TEST(SystemProbeGTest, NoThermalZoneMeansNoTemperature)
{
    const FakeSysfs sysfs("nothermal");
    EXPECT_FALSE(readTemperatureCelsius(sysfs.root()).has_value());
}

TEST(SystemProbeGTest, ThrottleFlagsParseWithOrWithoutPrefix)
{
    const FakeSysfs sysfs("throttle");
    sysfs.write("devices/platform/soc/soc:firmware/get_throttled", "50005");
    EXPECT_EQ(readThrottleFlags(sysfs.root()), 0x50005U);
    sysfs.write("devices/platform/soc/soc:firmware/get_throttled", "0x4");
    EXPECT_EQ(readThrottleFlags(sysfs.root()), 0x4U);
}

TEST(SystemProbeGTest, NotAPiMeansNoThrottleFlags)
{
    const FakeSysfs sysfs("notpi");
    EXPECT_FALSE(readThrottleFlags(sysfs.root()).has_value());
}

TEST(SystemProbeGTest, ThrottleFlagsDecode)
{
    // 0x50005: under-voltage and throttled now; both have also occurred since boot.
    const std::vector<std::string> active = activeThrottleConditions(0x50005);
    const std::vector<std::string> past = pastThrottleConditions(0x50005);
    EXPECT_EQ(active, (std::vector<std::string> { "under-voltage", "throttled" }));
    EXPECT_EQ(past, (std::vector<std::string> { "under-voltage", "throttled" }));
    EXPECT_TRUE(activeThrottleConditions(0).empty());
}

TEST(SystemProbeGTest, CpuUsageIsNonNegative)
{
    CpuUsage usage;
    // Burn a little CPU so the interval isn't empty.
    volatile double sink = 0.0;
    for (int i = 0; i < 1000000; ++i)
        sink = sink + i;
    EXPECT_GE(usage.sample(), 0.0);
}

} // namespace pivision::system
