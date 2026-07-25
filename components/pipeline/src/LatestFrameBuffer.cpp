#include <pivision/pipeline/LatestFrameBuffer.h>

#include <utility>

namespace pivision::pipeline {

bool LatestFrameBuffer::put(DisplayFrame frame)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    const bool wasEmpty = !m_frame.has_value();
    if (!wasEmpty)
        ++m_dropped;
    m_frame = std::move(frame);
    return wasEmpty;
}

std::optional<DisplayFrame> LatestFrameBuffer::take()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::optional<DisplayFrame> frame = std::move(m_frame);
    m_frame.reset();
    return frame;
}

std::uint64_t LatestFrameBuffer::droppedCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_dropped;
}

} // namespace pivision::pipeline
