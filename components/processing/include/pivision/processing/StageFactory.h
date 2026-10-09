#ifndef PIVISION_PROCESSING_STAGEFACTORY_H
#define PIVISION_PROCESSING_STAGEFACTORY_H

#include <memory>
#include <string>
#include <vector>

#include <pivision/processing/Stage.h>

namespace pivision::processing {

// Built-in stages:
//   "clahe"      contrast enhancement (CLAHE); parameter "clipLimit"
//   "grayscale"  BGR to gray
//   "blur"       Gaussian blur; parameter "size" (odd kernel size)
//   "edges"      Canny edge detection; parameters "low" and "high"
// Returns nullptr for an unknown id.
std::unique_ptr<Stage> makeStage(const std::string &id);

std::vector<std::string> knownStageIds();

} // namespace pivision::processing

#endif // PIVISION_PROCESSING_STAGEFACTORY_H
