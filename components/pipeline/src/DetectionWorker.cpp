#include "DetectionWorker.h"

#include <chrono>
#include <utility>

#include <opencv2/core.hpp>

#include <pivision/detection/ModelConfig.h>
#include <pivision/logging/Logging.h>

namespace pivision::pipeline {

DetectionWorker::DetectionWorker(DetectionJobBuffer &input, DetectionResultBuffer &output)
    : input_(input)
    , output_(output)
{
}

DetectionWorker::~DetectionWorker() = default;

bool DetectionWorker::loadModel()
{
    if (detector_)
        return true;
    if (loadFailed_)
        return false;

    emit stateChanged(DetectorState::Loading, {});
    std::string error;
    const auto config = detection::loadModelConfig(modelDescription_, &error);
    if (config) {
        try {
            detector_ = detection::makeDetector(*config);
        } catch (const cv::Exception &exception) {
            error = config->modelPath + ": " + exception.msg;
        }
    }
    if (!detector_) {
        loadFailed_ = true;
        qCWarning(logging::lcPipeline, "Could not load the detection model: %s", error.c_str());
        emit stateChanged(DetectorState::Failed, QString::fromStdString(error));
        return false;
    }
    qCInfo(logging::lcPipeline, "Loaded detection model %s", detector_->config().name.c_str());
    emit stateChanged(DetectorState::Ready, {});
    return true;
}

void DetectionWorker::detectLatest()
{
    auto job = input_.take();
    if (!job || !loadModel())
        return;

    DetectionResult result;
    const auto start = std::chrono::steady_clock::now();
    result.detections = detector_->detect(job->bgr);
    result.milliseconds
        = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    result.image = std::move(job->image);
    result.input = job->input;
    result.frameIndex = job->frameIndex;
    result.captured = job->captured;
    result.generation = job->generation;

    if (output_.put(std::move(result)))
        emit resultAvailable();
}

} // namespace pivision::pipeline
