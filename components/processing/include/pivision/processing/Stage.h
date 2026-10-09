#ifndef PIVISION_PROCESSING_STAGE_H
#define PIVISION_PROCESSING_STAGE_H

#include <optional>
#include <string>

#include <opencv2/core/mat.hpp>

namespace pivision::processing {

// One image-processing step. Takes an 8-bit image with 1 (gray) or 3 (BGR)
// channels and writes its result to `out`, which never aliases `in`.
// Not thread-safe: one thread owns a stage and calls it.
class Stage {
public:
    virtual ~Stage();

    Stage(const Stage &) = delete;
    Stage &operator=(const Stage &) = delete;

    // Matches the "id" used in stages.json.
    virtual std::string id() const = 0;

    virtual void process(const cv::Mat &in, cv::Mat &out) = 0;

    // Returns false for a name this stage doesn't have. Values outside what the
    // stage supports are clamped, e.g. blur sizes are made odd.
    virtual bool setParameter(const std::string &name, double value) = 0;
    virtual std::optional<double> parameter(const std::string &name) const = 0;

protected:
    Stage() = default;
};

} // namespace pivision::processing

#endif // PIVISION_PROCESSING_STAGE_H
