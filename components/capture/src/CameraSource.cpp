#include "CameraSource.h"

#include <pivision/capture/CameraDiscovery.h>

#include <chrono>
#include <utility>

namespace pivision::capture {

CameraSource::CameraSource(std::string device, const CameraConfig &config)
    : device_(std::move(device))
    , config_(config)
{
}

bool CameraSource::open()
{
    // V4L2 explicitly, so OpenCV never hands a device path to another backend.
    if (!capture_.open(device_, cv::CAP_V4L2))
        return false;

    // Format first, then size: the driver picks a size from the format's modes.
    if (config_.mjpeg)
        capture_.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    capture_.set(cv::CAP_PROP_FRAME_WIDTH, config_.width);
    capture_.set(cv::CAP_PROP_FRAME_HEIGHT, config_.height);
    capture_.set(cv::CAP_PROP_FPS, config_.fps);

    const double reported = capture_.get(cv::CAP_PROP_FPS);
    fps_ = reported > 0.0 ? reported : config_.fps;
    if (auto info = describeCamera(device_))
        label_ = info->name + " (" + device_ + ")";
    index_ = 0;
    return true;
}

bool CameraSource::read(Frame &out)
{
    if (!capture_.isOpened() || !capture_.read(out.image) || out.image.empty())
        return false;

    out.index = index_++;
    out.captured = std::chrono::steady_clock::now();
    return true;
}

void CameraSource::close()
{
    capture_.release();
}

} // namespace pivision::capture
