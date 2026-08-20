#include "AutoSource.h"

#include <utility>

namespace pivision::capture {

AutoSource::AutoSource(std::vector<std::string> devices, const CameraConfig &config,
    const TestPatternConfig &fallback)
    : m_devices(std::move(devices))
    , m_config(config)
    , m_fallback(fallback)
{
}

bool AutoSource::open()
{
    close();
    for (const std::string &device : m_devices) {
        auto camera = makeCameraSource(device, m_config);
        if (camera->open()) {
            m_active = std::move(camera);
            return true;
        }
    }

    auto pattern = makeTestPatternSource(m_fallback);
    if (!pattern->open())
        return false;
    m_active = std::move(pattern);
    return true;
}

bool AutoSource::read(Frame &out)
{
    return m_active && m_active->read(out);
}

void AutoSource::close()
{
    if (m_active) {
        m_active->close();
        m_active.reset();
    }
}

std::string AutoSource::name() const
{
    return m_active ? m_active->name() : "Auto";
}

double AutoSource::nominalFps() const
{
    return m_active ? m_active->nominalFps() : m_config.fps;
}

} // namespace pivision::capture
