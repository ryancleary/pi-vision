#include "CameraSource.h"

#include <chrono>
#include <utility>

namespace pivision::capture {

CameraSource::CameraSource(std::string device, const CameraConfig &config)
    : m_device(std::move(device))
    , m_config(config)
{
}

bool CameraSource::open()
{
    // V4L2 explicitly, so OpenCV never hands a device path to another backend.
    if (!m_capture.open(m_device, cv::CAP_V4L2))
        return false;

    // Format first, then size: the driver picks a size from the format's modes.
    if (m_config.mjpeg)
        m_capture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    m_capture.set(cv::CAP_PROP_FRAME_WIDTH, m_config.width);
    m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, m_config.height);
    m_capture.set(cv::CAP_PROP_FPS, m_config.fps);

    const double reported = m_capture.get(cv::CAP_PROP_FPS);
    m_fps = reported > 0.0 ? reported : m_config.fps;
    m_index = 0;
    return true;
}

bool CameraSource::read(Frame &out)
{
    if (!m_capture.isOpened() || !m_capture.read(out.image) || out.image.empty())
        return false;

    out.index = m_index++;
    out.captured = std::chrono::steady_clock::now();
    return true;
}

void CameraSource::close()
{
    m_capture.release();
}

std::string CameraSource::name() const
{
    return m_device;
}

double CameraSource::nominalFps() const
{
    return m_fps > 0.0 ? m_fps : m_config.fps;
}

} // namespace pivision::capture
