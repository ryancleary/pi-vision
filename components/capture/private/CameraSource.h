#ifndef PIVISION_CAPTURE_CAMERASOURCE_H
#define PIVISION_CAPTURE_CAMERASOURCE_H

#include <cstdint>
#include <string>

#include <opencv2/videoio.hpp>

#include <pivision/capture/FrameSource.h>
#include <pivision/capture/SourceFactory.h>

namespace pivision::capture {

class CameraSource final : public FrameSource {
public:
    CameraSource(std::string device, const CameraConfig &config);

    bool open() override;
    bool read(Frame &out) override;
    void close() override;
    std::string name() const override;
    double nominalFps() const override;

private:
    std::string m_device;
    CameraConfig m_config;
    cv::VideoCapture m_capture;
    double m_fps = 0.0;
    std::uint64_t m_index = 0;
};

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_CAMERASOURCE_H
