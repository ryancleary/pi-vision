#include "Utilities.h"

#include <algorithm>
#include <cmath>

namespace pivision::processing {

int validGaussianKernelSize(double requested)
{
    constexpr int kSmallest = 1;
    constexpr int kLargest = 31;
    const int size = std::clamp(static_cast<int>(std::lround(requested)), kSmallest, kLargest);
    return size % 2 == 1 ? size : size + 1;
}

} // namespace pivision::processing
