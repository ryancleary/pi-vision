#ifndef PIVISION_CAPTURE_UTILS_H
#define PIVISION_CAPTURE_UTILS_H

#include <optional>
#include <string>

namespace pivision::capture {

// The N in a device name "videoN", or nothing if the name has another form.
std::optional<int> deviceNumber(const std::string &filename);

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_UTILS_H
