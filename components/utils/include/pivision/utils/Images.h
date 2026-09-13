#ifndef PIVISION_UTILS_IMAGES_H
#define PIVISION_UTILS_IMAGES_H

#include <QImage>

#include <opencv2/core/mat.hpp>

namespace pivision::utils {

// Copies an 8-bit BGR or gray cv::Mat into a QImage that owns its pixels, so
// it can outlive the Mat and cross threads.
QImage toQImage(const cv::Mat &mat);

} // namespace pivision::utils

#endif // PIVISION_UTILS_IMAGES_H
