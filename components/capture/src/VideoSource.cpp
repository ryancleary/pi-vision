#include "VideoSource.h"

#include <chrono>
#include <filesystem>
#include <utility>

namespace pivision::capture {

VideoSource::VideoSource(std::string location)
    : location_(std::move(location))
{
}

bool VideoSource::open()
{
    if (!capture_.open(location_))
        return false;

    const double reported = capture_.get(cv::CAP_PROP_FPS);
    fps_ = reported > 0.0 ? reported : 30.0;
    isFile_ = location_.find("://") == std::string::npos;
    index_ = 0;
    return true;
}

bool VideoSource::read(Frame &out)
{
    if (!capture_.isOpened())
        return false;

    bool ok = capture_.read(out.image) && !out.image.empty();
    if (!ok && isFile_) {
        // End of file: loop by reopening. Seeking back to frame 0 isn't reliable
        // across OpenCV backends (GStreamer can't always report its position),
        // but reopening works with all of them.
        ok = capture_.open(location_) && capture_.read(out.image) && !out.image.empty();
    }
    if (!ok)
        return false;

    out.index = index_++;
    out.captured = std::chrono::steady_clock::now();
    return true;
}

void VideoSource::close()
{
    capture_.release();
}

std::string VideoSource::name() const
{
    // Show a file by its name; show a URL in full.
    if (location_.find("://") != std::string::npos)
        return location_;
    return std::filesystem::path(location_).filename().string();
}

} // namespace pivision::capture
