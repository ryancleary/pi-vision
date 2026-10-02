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
    // Opens the same device again or, if it came back under another node
    // (a re-plugged webcam can move from /dev/video0 to /dev/video2), the
    // first camera with the same driver name.
    bool reopen() override;
    bool read(Frame &out) override;
    void close() override;
    // The driver's name once open, e.g. "HD Webcam C270 (/dev/video0)".
    std::string name() const override { return label_.empty() ? device_ : label_; }
    // What the camera reported after open, or the requested rate before.
    double nominalFps() const override { return fps_ > 0.0 ? fps_ : config_.fps; }

private:
    std::string device_;
    std::string label_;      // "HD Webcam C270 (/dev/video0)" once opened
    std::string driverName_; // "HD Webcam C270" once opened
    CameraConfig config_;
    cv::VideoCapture capture_;
    double fps_ = 0.0;
    std::uint64_t index_ = 0;
};

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_CAMERASOURCE_H
