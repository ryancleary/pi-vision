#include "BuiltinStages.h"

#include "Utilities.h"

#include <algorithm>
#include <vector>

namespace pivision::processing {

// --- ClaheStage ---

// CLAHE equalizes the image in an 8x8 grid of tiles (OpenCV's default), so
// each region's contrast is stretched on its own.
ClaheStage::ClaheStage()
    : clahe_(cv::createCLAHE(clipLimit_, cv::Size(8, 8)))
{
}

void ClaheStage::process(const cv::Mat &in, cv::Mat &out)
{
    if (in.channels() == 1) {
        clahe_->apply(in, out);
        return;
    }
    // Equalize lightness only (channel 0 of Lab), so colors don't shift.
    cv::cvtColor(in, lab_, cv::COLOR_BGR2Lab);
    cv::extractChannel(lab_, lightness_, 0);
    clahe_->apply(lightness_, lightness_);
    cv::insertChannel(lightness_, lab_, 0);
    cv::cvtColor(lab_, out, cv::COLOR_Lab2BGR);
}

bool ClaheStage::setParameter(const std::string &name, double value)
{
    if (name != "clipLimit")
        return false;
    // How strongly CLAHE may boost contrast in each tile. OpenCV's own default
    // is 40; values near 0 effectively turn equalization off.
    constexpr double kMinClipLimit = 0.1;
    constexpr double kMaxClipLimit = 40.0;
    clipLimit_ = std::clamp(value, kMinClipLimit, kMaxClipLimit);
    clahe_->setClipLimit(clipLimit_);
    return true;
}

std::optional<double> ClaheStage::parameter(const std::string &name) const
{
    if (name == "clipLimit")
        return clipLimit_;
    return std::nullopt;
}

// --- GrayscaleStage ---

void GrayscaleStage::process(const cv::Mat &in, cv::Mat &out)
{
    if (in.channels() == 1)
        in.copyTo(out);
    else
        cv::cvtColor(in, out, cv::COLOR_BGR2GRAY);
}

bool GrayscaleStage::setParameter(const std::string &, double) { return false; }

std::optional<double> GrayscaleStage::parameter(const std::string &) const { return std::nullopt; }

// --- BlurStage ---

void BlurStage::process(const cv::Mat &in, cv::Mat &out)
{
    cv::GaussianBlur(in, out, cv::Size(size_, size_), 0.0);
}

bool BlurStage::setParameter(const std::string &name, double value)
{
    if (name != "size")
        return false;
    size_ = validGaussianKernelSize(value);
    return true;
}

std::optional<double> BlurStage::parameter(const std::string &name) const
{
    if (name == "size")
        return size_;
    return std::nullopt;
}

// --- EdgesStage ---

void EdgesStage::process(const cv::Mat &in, cv::Mat &out)
{
    if (in.channels() == 1) {
        cv::Canny(in, out, low_, high_);
        return;
    }
    cv::cvtColor(in, gray_, cv::COLOR_BGR2GRAY);
    cv::Canny(gray_, out, low_, high_);
}

bool EdgesStage::setParameter(const std::string &name, double value)
{
    // Canny compares gradient strength against these thresholds. Keeping them
    // within 0..255 matches the 8-bit range the sliders work in.
    constexpr double kMinThreshold = 0.0;
    constexpr double kMaxThreshold = 255.0;
    const double clamped = std::clamp(value, kMinThreshold, kMaxThreshold);
    if (name == "low")
        low_ = clamped;
    else if (name == "high")
        high_ = clamped;
    else
        return false;
    return true;
}

std::optional<double> EdgesStage::parameter(const std::string &name) const
{
    if (name == "low")
        return low_;
    if (name == "high")
        return high_;
    return std::nullopt;
}

} // namespace pivision::processing
