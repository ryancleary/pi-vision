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
    std::string name() const override;
    double nominalFps() const override;

private:
    TestPatternConfig m_config;
    cv::Mat m_background;
    std::uint64_t m_index = 0;
};

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_TESTPATTERNSOURCE_H
