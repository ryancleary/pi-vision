#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <optional>
#include <thread>

#include <pivision/pipeline/LatestFrameBuffer.h>

namespace pivision::pipeline {

class LatestFrameBufferTest : public ::testing::Test {
protected:
    static DisplayFrame frameWithIndex(std::uint64_t index)
    {
        DisplayFrame frame;
        frame.index = index;
        return frame;
    }
};

TEST_F(LatestFrameBufferTest, StartsEmpty)
{
    LatestFrameBuffer buffer;
    EXPECT_FALSE(buffer.take().has_value());
    EXPECT_EQ(buffer.droppedCount(), 0u);
}

TEST_F(LatestFrameBufferTest, PutIntoEmptyBufferRequestsNotification)
{
    LatestFrameBuffer buffer;
    EXPECT_TRUE(buffer.put(frameWithIndex(0)));
}

TEST_F(LatestFrameBufferTest, PutIntoFullBufferReplacesFrameAndCountsDrop)
{
    LatestFrameBuffer buffer;
    buffer.put(frameWithIndex(0));
    EXPECT_FALSE(buffer.put(frameWithIndex(1)));
    EXPECT_EQ(buffer.droppedCount(), 1u);

    auto frame = buffer.take();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->index, 1u);
}

TEST_F(LatestFrameBufferTest, TakeEmptiesBuffer)
{
    LatestFrameBuffer buffer;
    buffer.put(frameWithIndex(0));
    ASSERT_TRUE(buffer.take().has_value());

    EXPECT_FALSE(buffer.take().has_value());
    EXPECT_TRUE(buffer.put(frameWithIndex(1))); // empty again, so it notifies again
}

TEST_F(LatestFrameBufferTest, ConcurrentProducerAndConsumerKeepOrderAndAccountForEveryFrame)
{
    LatestFrameBuffer buffer;
    constexpr std::uint64_t frameCount = 100000;
    std::atomic<bool> producerDone { false };

    std::thread producer([&] {
        for (std::uint64_t i = 0; i < frameCount; ++i)
            buffer.put(frameWithIndex(i));
        producerDone = true;
    });

    std::uint64_t taken = 0;
    std::uint64_t outOfOrder = 0;
    std::optional<std::uint64_t> last;
    auto consume = [&] {
        if (auto frame = buffer.take()) {
            if (last && frame->index <= *last)
                ++outOfOrder;
            last = frame->index;
            ++taken;
        }
    };

    while (!producerDone)
        consume();
    producer.join();
    consume(); // the final frame, if the consumer hadn't taken it yet

    EXPECT_EQ(outOfOrder, 0u);
    // Every frame was either taken or replaced; none disappeared.
    EXPECT_EQ(taken + buffer.droppedCount(), frameCount);
}

} // namespace pivision::pipeline
