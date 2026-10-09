#ifndef PIVISION_PROCESSING_UTILITIES_H
#define PIVISION_PROCESSING_UTILITIES_H

namespace pivision::processing {

// OpenCV's Gaussian blur needs a positive, odd kernel size (the kernel has a
// center pixel). Rounds to the nearest whole size, keeps it within 1..31,
// and moves an even size up to the next odd one.
int validGaussianKernelSize(double requested);

} // namespace pivision::processing

#endif // PIVISION_PROCESSING_UTILITIES_H
