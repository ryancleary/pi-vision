#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include <pivision/capture/CameraDiscovery.h>

using pivision::capture::findCameras;

namespace fs = std::filesystem;

TEST(CameraDiscovery, MissingDirectoryGivesNoCameras)
{
    EXPECT_TRUE(findCameras("/nonexistent").empty());
}

TEST(CameraDiscovery, IgnoresNodesThatAreNotCaptureDevices)
{
    // Plain files named like device nodes: they open, but the V4L2 query fails.
    const fs::path dir = fs::temp_directory_path() / "pivision-fake-dev";
    fs::remove_all(dir);
    fs::create_directories(dir);
    for (const char *name : { "video0", "video1", "video10", "videoX", "media0" })
        std::ofstream(dir / name) << "not a device";

    EXPECT_TRUE(findCameras(dir.string()).empty());
    fs::remove_all(dir);
}

TEST(CameraDiscovery, DescribeRejectsMissingAndNonDeviceFiles)
{
    EXPECT_FALSE(pivision::capture::describeCamera("/nonexistent/video0").has_value());
    EXPECT_FALSE(pivision::capture::describeCamera("/etc/hostname").has_value());
}
