#include <gtest/gtest.h>

#include <pivision/capture/SourceFactory.h>

namespace pivision::capture {

// Real capture needs hardware, so CI only covers what happens without a camera.

TEST(CameraSource, OpenFailsForMissingDevice)
{
    auto source = makeCameraSource("/nonexistent/video0");
    EXPECT_FALSE(source->open());

    Frame frame;
    EXPECT_FALSE(source->read(frame));
}

TEST(CameraSource, ReportsDeviceAndRequestedRate)
{
    CameraConfig config;
    config.fps = 15.0;
    auto source = makeCameraSource("/dev/video3", config);
    EXPECT_EQ(source->name(), "/dev/video3");
    EXPECT_DOUBLE_EQ(source->nominalFps(), 15.0);
}

} // namespace pivision::capture
