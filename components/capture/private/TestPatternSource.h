#ifndef PIVISION_CAPTURE_TESTPATTERNSOURCE_H
#define PIVISION_CAPTURE_TESTPATTERNSOURCE_H

#include <cstdint>
#include <string>

#include <opencv2/core/mat.hpp>

#include <pivision/capture/FrameSource.h>
#include <pivision/capture/SourceFactory.h>

namespace pivision::capture {

class TestPatternSource final : public FrameSource {
public:
    explicit TestPatternSource(const TestPatternConfig &config);

    bool open() override;
    bool read(Frame &out) override;
    void close() override;
    std::string name() const override { return "Test pattern"; }
    double nominalFps() const override { return config_.fps; }

private:
    TestPatternConfig config_;
    cv::Mat background_;
    std::uint64_t index_ = 0;
};

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_TESTPATTERNSOURCE_H
