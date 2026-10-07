#include <pivision/capture/SourceFactory.h>

#include "TestPatternSource.h"

namespace pivision::capture {

std::unique_ptr<FrameSource> makeTestPatternSource(const TestPatternConfig &config)
{
    return std::make_unique<TestPatternSource>(config);
}

} // namespace pivision::capture

