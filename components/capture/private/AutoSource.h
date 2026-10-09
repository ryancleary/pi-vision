#ifndef PIVISION_CAPTURE_AUTOSOURCE_H
#define PIVISION_CAPTURE_AUTOSOURCE_H

#include <memory>
#include <string>
#include <vector>

#include <pivision/capture/FrameSource.h>
#include <pivision/capture/SourceFactory.h>

namespace pivision::capture {

// Tries each camera in turn and falls back to the test pattern. Once open,
// every call goes to whichever source was chosen.
class AutoSource final : public FrameSource {
public:
    AutoSource(std::vector<std::string> devices, const CameraConfig &config,
        const TestPatternConfig &fallback);

    bool open() override;
    bool read(Frame &out) override;
    void close() override;
    std::string name() const override;
    double nominalFps() const override;

private:
    std::vector<std::string> m_devices;
    CameraConfig m_config;
    TestPatternConfig m_fallback;
    std::unique_ptr<FrameSource> m_active;
};

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_AUTOSOURCE_H
