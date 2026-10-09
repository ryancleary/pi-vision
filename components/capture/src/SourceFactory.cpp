#include <pivision/capture/SourceFactory.h>

#include <utility>

#include <pivision/capture/CameraDiscovery.h>

#include "AutoSource.h"
#include "CameraSource.h"
#include "TestPatternSource.h"
#include "VideoSource.h"

namespace pivision::capture {

std::unique_ptr<FrameSource> makeTestPatternSource(const TestPatternConfig &config)
{
    return std::make_unique<TestPatternSource>(config);
}

std::unique_ptr<FrameSource> makeCameraSource(const std::string &device,
    const CameraConfig &config)
{
    return std::make_unique<CameraSource>(device, config);
}

std::unique_ptr<FrameSource> makeVideoSource(const std::string &location)
{
    return std::make_unique<VideoSource>(location);
}

std::unique_ptr<FrameSource> makeSourceFromSpec(const std::string &spec,
    const CameraConfig &config)
{
    if (spec.rfind("/dev/video", 0) == 0)
        return makeCameraSource(spec, config);
    return makeVideoSource(spec);
}

std::unique_ptr<FrameSource> makeAutoSource(std::vector<std::string> devices,
    const CameraConfig &config, const TestPatternConfig &fallback)
{
    return std::make_unique<AutoSource>(std::move(devices), config, fallback);
}

std::unique_ptr<FrameSource> makeAutoSource(const CameraConfig &config,
    const TestPatternConfig &fallback)
{
    std::vector<std::string> devices;
    for (const CameraInfo &camera : findCameras())
        devices.push_back(camera.device);
    return makeAutoSource(std::move(devices), config, fallback);
}

} // namespace pivision::capture
