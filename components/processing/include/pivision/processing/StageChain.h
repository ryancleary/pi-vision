#ifndef PIVISION_PROCESSING_STAGECHAIN_H
#define PIVISION_PROCESSING_STAGECHAIN_H

#include <memory>
#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>

#include <pivision/processing/Stage.h>

namespace pivision::processing {

struct StageTiming {
    std::string id;
    double milliseconds = 0.0;
};

// Runs stages in the order they were added, skipping disabled ones.
// Not thread-safe: one thread owns the chain.
class StageChain {
public:
    void add(std::unique_ptr<Stage> stage, bool enabled);
    void clear() { entries_.clear(); }

    // Both return false if no stage has that id.
    bool setEnabled(const std::string &id, bool enabled);
    bool setParameter(const std::string &id, const std::string &name, double value);

    bool isEnabled(const std::string &id) const;
    bool anyEnabled() const;
    std::vector<std::string> order() const;

    // Runs every enabled stage on `in` and leaves the result in `out` (its own
    // copy, safe to keep). If `timings` is given, it receives one entry per
    // stage that ran.
    void run(const cv::Mat &in, cv::Mat &out, std::vector<StageTiming> *timings = nullptr);

private:
    struct Entry {
        std::unique_ptr<Stage> stage;
        bool enabled = false;
    };

    Entry *find(const std::string &id);
    const Entry *find(const std::string &id) const;

    std::vector<Entry> entries_;
    cv::Mat buffers_[2]; // reused between frames
};

} // namespace pivision::processing

#endif // PIVISION_PROCESSING_STAGECHAIN_H
