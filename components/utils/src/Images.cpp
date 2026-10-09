#include <pivision/utils/Images.h>

namespace pivision::utils {

QImage toQImage(const cv::Mat &mat)
{
    const auto format = mat.channels() == 1 ? QImage::Format_Grayscale8 : QImage::Format_BGR888;
    // A view onto the Mat's pixels (no copy), then copy() to own them.
    const QImage view(mat.data, mat.cols, mat.rows, static_cast<qsizetype>(mat.step), format);
    return view.copy();
}

} // namespace pivision::utils
