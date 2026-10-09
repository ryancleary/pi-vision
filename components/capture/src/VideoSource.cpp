#include "VideoSource.h"

#include <chrono>
#include <filesystem>
#include <utility>

namespace pivision::capture {

VideoSource::VideoSource(std::string location)
    : m_location(std::move(location))
{
}

bool VideoSource::open()
{
    if (!m_capture.open(m_location))
        return false;

    const double reported = m_capture.get(cv::CAP_PROP_FPS);
    m_fps = reported > 0.0 ? reported : 30.0;
    m_isFile = m_location.find("://") == std::string::npos;
    m_index = 0;
    return true;
}

bool VideoSource::read(Frame &out)
{
    if (!m_capture.isOpened())
        return false;

    bool ok = m_capture.read(out.image) && !out.image.empty();
    if (!ok && m_isFile) {
        // End of file: loop by reopening. Seeking back to frame 0 isn't reliable
        // across OpenCV backends (GStreamer can't always report its position),
        // but reopening works with all of them.
        ok = m_capture.open(m_location) && m_capture.read(out.image) && !out.image.empty();
    }
    if (!ok)
        return false;

    out.index = m_index++;
    out.captured = std::chrono::steady_clock::now();
    return true;
}

void VideoSource::close()
{
    m_capture.release();
}

std::string VideoSource::name() const
{
    // Show a file by its name; show a URL in full.
    if (m_location.find("://") != std::string::npos)
        return m_location;
    return std::filesystem::path(m_location).filename().string();
}

double VideoSource::nominalFps() const
{
    return m_fps;
}

} // namespace pivision::capture
