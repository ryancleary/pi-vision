#ifndef PIVISION_DETECTION_DETECTION_H
#define PIVISION_DETECTION_DETECTION_H

#include <opencv2/core/types.hpp>

namespace pivision::detection {

// One object found in an image.
struct Detection {
    cv::Rect box;      // in the pixels of the image that was searched
    int classId = -1;  // index into the model's class list (COCO for NanoDet)
    float score = 0.0F; // confidence, 0..1
};

} // namespace pivision::detection

#endif // PIVISION_DETECTION_DETECTION_H

