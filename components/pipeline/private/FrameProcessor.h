#ifndef PIVISION_PIPELINE_FRAMEPROCESSOR_H
#define PIVISION_PIPELINE_FRAMEPROCESSOR_H

#include <string>
#include <vector>

#include <QList>
#include <QObject>
#include <QSize>

#include <opencv2/core/mat.hpp>

#include <pivision/pipeline/LatestFrameBuffer.h>
#include <pivision/pipeline/StageConfig.h>
#include <pivision/processing/StageChain.h>

#include "DetectionJob.h"
#include "RawFrame.h"

namespace pivision::pipeline {

// Lives on the processing thread. Takes the newest raw frame, scales it to the
// processing size, runs the enabled stages, and puts raw + processed images in
// the display buffer. Settings calls must run on the processing thread, or
// before it starts.
class FrameProcessor : public QObject {
    Q_OBJECT
public:
    FrameProcessor(RawFrameBuffer &input, LatestFrameBuffer &output, DetectionJobBuffer &detectionOutput);

    void configure(const QList<StageConfig> &stages);
    void setStageEnabled(const std::string &id, bool enabled);
    void setStageParameter(const std::string &id, const std::string &name, double value);
    // Frames larger than this are scaled down to fit, keeping their shape.
    // Smaller frames are processed as they are. An empty size means no scaling.
    void setProcessSize(const QSize &size) { processSize_ = size; }
    // While enabled, each frame (raw or processed) is also handed to the detector.
    void setDetectionFeed(bool enabled, DetectionInput input)
    {
        detectionEnabled_ = enabled;
        detectionInput_ = input;
    }

public slots:
    void processLatest();

signals:
    void frameAvailable();
    void detectionJobAvailable();

private:
    // `processed` is what the stages produced (or the scaled frame, with no stages).
    void feedDetection(const cv::Mat &raw, const cv::Mat &processed, const DisplayFrame &frame);

    RawFrameBuffer &input_;
    LatestFrameBuffer &output_;
    DetectionJobBuffer &detectionOutput_;
    bool detectionEnabled_ = false;
    DetectionInput detectionInput_ = DetectionInput::Raw;
    processing::StageChain chain_;
    QSize processSize_;
    cv::Mat scaled_;
    cv::Mat result_;
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_FRAMEPROCESSOR_H
