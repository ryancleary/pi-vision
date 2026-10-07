#pragma once

#include <chrono>
#include <cstdint>

#include <opencv2/core/mat.hpp>

namespace pivision::capture {

// A captured image plus the metadata later stages need.
struct Frame {
    cv::Mat image;                                   // 8-bit BGR
    std::uint64_t index = 0;                         // increments on every read, never resets
    std::chrono::steady_clock::time_point captured;  // when the source produced the frame
};

} // namespace pivision::capture

