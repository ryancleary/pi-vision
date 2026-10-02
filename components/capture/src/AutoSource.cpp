#include "AutoSource.h"

#include <utility>

namespace pivision::capture {

AutoSource::AutoSource(std::vector<std::string> devices, const CameraConfig &config,
    const TestPatternConfig &fallback)
    : devices_(std::move(devices))
    , config_(config)
    , fallback_(fallback)
{
}

bool AutoSource::open()
{
    close();
    active_.reset();
    for (const std::string &device : devices_) {
        auto camera = makeCameraSource(device, config_);
        if (camera->open()) {
            active_ = std::move(camera);
            return true;
        }
    }

    auto pattern = makeTestPatternSource(fallback_);
    if (!pattern->open())
        return false;
    active_ = std::move(pattern);
    return true;
}

bool AutoSource::reopen()
{
    // Keep the chosen source (and its name) and ask it to come back.
    return active_ ? active_->reopen() : open();
}

bool AutoSource::read(Frame &out)
{
    return active_ && active_->read(out);
}

void AutoSource::close()
{
    // The choice is kept so reopen() can return to it; open() chooses again.
    if (active_)
        active_->close();
}

} // namespace pivision::capture
