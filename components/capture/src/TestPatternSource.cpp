#include "TestPatternSource.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>

#include <opencv2/imgproc.hpp>

namespace pivision::capture {

TestPatternSource::TestPatternSource(const TestPatternConfig &config)
    : config_(config)
{
}

bool TestPatternSource::open()
{
    if (config_.width <= 0 || config_.height <= 0 || config_.fps <= 0.0)
        return false;

    // A dark blue-to-brighter-blue horizontal gradient, built once and copied
    // into every frame. cv::resize stretches a two-pixel image across the
    // width, interpolating the colors in between.
    const cv::Mat ends = (cv::Mat_<cv::Vec3b>(1, 2) << cv::Vec3b(90, 40, 30), cv::Vec3b(150, 80, 30));
    cv::resize(ends, background_, cv::Size(config_.width, config_.height), 0.0, 0.0,
        cv::INTER_LINEAR);
    index_ = 0;
    return true;
}

bool TestPatternSource::read(Frame &out)
{
    if (background_.empty())
        return false;

    background_.copyTo(out.image); // reuses out.image's buffer when the size matches

    // A square a quarter of the frame height tall, vertically centered, sliding
    // right by kStepPixels each frame and wrapping back to the left edge. The
    // motion makes dropped or repeated frames easy to spot by eye.
    constexpr std::uint64_t kStepPixels = 4;
    const int side = std::max(1, config_.height / 4);
    const std::uint64_t travel = static_cast<std::uint64_t>(std::max(1, config_.width - side));
    const int x = static_cast<int>((index_ * kStepPixels) % travel);
    const int centeredY = (config_.height - side) / 2;
    cv::rectangle(out.image, cv::Rect(x, centeredY, side, side), cv::Scalar(60, 200, 255), cv::FILLED);
    cv::putText(out.image, std::to_string(index_), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
        0.8, cv::Scalar(255, 255, 255), 2);

    out.index = index_++;
    out.captured = std::chrono::steady_clock::now();
    return true;
}

void TestPatternSource::close() { background_.release(); }

} // namespace pivision::capture
