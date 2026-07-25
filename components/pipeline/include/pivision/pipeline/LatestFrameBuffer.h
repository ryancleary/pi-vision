#ifndef PIVISION_PIPELINE_LATESTFRAMEBUFFER_H
#define PIVISION_PIPELINE_LATESTFRAMEBUFFER_H

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>

#include <QImage>

namespace pivision::pipeline {

// A frame ready for display.
struct DisplayFrame {
    QImage image;
    std::uint64_t index = 0;
    std::chrono::steady_clock::time_point captured;
};

// Holds at most one frame. A new frame replaces an unread one, so the reader
// always gets the newest frame and never falls behind. Thread-safe.
class LatestFrameBuffer {
public:
    // Returns true if the buffer was empty, meaning the reader needs a notification.
    bool put(DisplayFrame frame);

    // Removes and returns the frame, if there is one.
    std::optional<DisplayFrame> take();

    // Frames that were replaced before anyone read them.
    std::uint64_t droppedCount() const;

private:
    mutable std::mutex m_mutex;
    std::optional<DisplayFrame> m_frame;
    std::uint64_t m_dropped = 0;
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_LATESTFRAMEBUFFER_H
