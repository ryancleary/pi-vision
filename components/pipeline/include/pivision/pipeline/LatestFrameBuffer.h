#ifndef PIVISION_PIPELINE_LATESTFRAMEBUFFER_H
#define PIVISION_PIPELINE_LATESTFRAMEBUFFER_H

#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include <QImage>

#include <pivision/processing/StageChain.h>

namespace pivision::pipeline {

// A frame ready for display: the camera image and what processing made of it,
// always from the same capture.
struct DisplayFrame {
    QImage raw;      // as captured
    QImage processed; // at the processing size; gray if a stage made it gray
    std::uint64_t index = 0;
    std::chrono::steady_clock::time_point captured;
    // Time each processing step took on this frame.
    std::vector<processing::StageTiming> timings;
    // Counts source switches; frames from a replaced source are dropped.
    std::uint64_t generation = 0;
};

// Holds at most one item. A new item replaces an unread one, so the reader
// always gets the newest and never falls behind. Thread-safe.
template <typename T>
class LatestBuffer {
public:
    // Returns true if the buffer was empty, meaning the reader needs a notification.
    bool put(T item)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const bool wasEmpty = !item_.has_value();
        if (!wasEmpty)
            ++dropped_;
        item_ = std::move(item);
        return wasEmpty;
    }

    // Removes and returns the item, if there is one.
    std::optional<T> take()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::optional<T> item = std::move(item_);
        item_.reset();
        return item;
    }

    // Items that were replaced before anyone read them.
    std::uint64_t droppedCount() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return dropped_;
    }

private:
    mutable std::mutex mutex_;
    std::optional<T> item_;
    std::uint64_t dropped_ = 0;
};

using LatestFrameBuffer = LatestBuffer<DisplayFrame>;

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_LATESTFRAMEBUFFER_H
