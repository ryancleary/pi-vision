#ifndef PIVISION_CAPTURE_VIDEOSOURCE_H
#define PIVISION_CAPTURE_VIDEOSOURCE_H

#include <cstdint>
#include <string>

#include <opencv2/videoio.hpp>

#include <pivision/capture/FrameSource.h>

namespace pivision::capture {

// A video file or stream URL. Files loop; streams end when the stream does.
class VideoSource final : public FrameSource {
public:
    explicit VideoSource(std::string location);

    bool open() override;
    bool read(Frame &out) override;
    void close() override;
    std::string name() const override;
    double nominalFps() const override { return fps_; }

private:
    std::string location_;
    cv::VideoCapture capture_;
    double fps_ = 30.0;
    bool isFile_ = false;
    std::uint64_t index_ = 0;
};

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_VIDEOSOURCE_H
