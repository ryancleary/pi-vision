#ifndef PIVISION_PROCESSING_BUILTINSTAGES_H
#define PIVISION_PROCESSING_BUILTINSTAGES_H

#include <optional>
#include <string>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#include <pivision/processing/Stage.h>

namespace pivision::processing {

// Contrast Limited Adaptive Histogram Equalization: boosts local contrast in
// dark or flat regions, as used on medical images. Color images are equalized
// on lightness only, so hues don't shift.
class ClaheStage final : public Stage {
public:
    ClaheStage();
    std::string id() const override { return "clahe"; }
    void process(const cv::Mat &in, cv::Mat &out) override;
    bool setParameter(const std::string &name, double value) override;
    std::optional<double> parameter(const std::string &name) const override;

private:
    double clipLimit_ = 2.0; // declared first: clahe_ is built from it
    cv::Ptr<cv::CLAHE> clahe_;
    cv::Mat lab_;
    cv::Mat lightness_;
};

class GrayscaleStage final : public Stage {
public:
    std::string id() const override { return "grayscale"; }
    void process(const cv::Mat &in, cv::Mat &out) override;
    bool setParameter(const std::string &name, double value) override;
    std::optional<double> parameter(const std::string &name) const override;
};

class BlurStage final : public Stage {
public:
    std::string id() const override { return "blur"; }
    void process(const cv::Mat &in, cv::Mat &out) override;
    bool setParameter(const std::string &name, double value) override;
    std::optional<double> parameter(const std::string &name) const override;

private:
    int size_ = 5;
};

class EdgesStage final : public Stage {
public:
    std::string id() const override { return "edges"; }
    void process(const cv::Mat &in, cv::Mat &out) override;
    bool setParameter(const std::string &name, double value) override;
    std::optional<double> parameter(const std::string &name) const override;

private:
    double low_ = 50.0;
    double high_ = 150.0;
    cv::Mat gray_;
};

} // namespace pivision::processing

#endif // PIVISION_PROCESSING_BUILTINSTAGES_H
