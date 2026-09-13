#ifndef PIVISION_CAPTURE_UTILITIES_H
#define PIVISION_CAPTURE_UTILITIES_H

#include <optional>
#include <string>

namespace pivision::capture {

// The N in a device name "videoN", or nothing if the name has another form.
std::optional<int> deviceNumber(const std::string &filename);

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_UTILITIES_H
