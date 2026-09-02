#include "FrameProcessor.h"

#include <utility>

#include <QImage>

#include <opencv2/imgproc.hpp>

#include <pivision/logging/Logging.h>
#include <pivision/processing/StageFactory.h>

namespace pivision::pipeline {

namespace {

    // Copies an 8-bit BGR or gray image into a QImage that owns its pixels, so
    // it can outlive the cv::Mat and cross to the GUI thread.
    QImage toQImage(const cv::Mat &mat)
    {
        const auto format = mat.channels() == 1 ? QImage::Format_Grayscale8 : QImage::Format_BGR888;
        const QImage view(mat.data, mat.cols, mat.rows, static_cast<qsizetype>(mat.step), format);
        return view.copy();
    }

    // The size a frame is processed at: unchanged if it already fits inside
    // `limit`, otherwise shrunk to fit with its aspect ratio kept (QSize::scaled
    // with Qt::KeepAspectRatio does the math). An empty `limit` means no limit.
    QSize processingSizeFor(const QSize &frame, const QSize &limit)
    {
        const bool fits = frame.width() <= limit.width() && frame.height() <= limit.height();
        if (limit.isEmpty() || fits)
            return frame;
        return frame.scaled(limit, Qt::KeepAspectRatio);
    }

} // namespace

FrameProcessor::FrameProcessor(RawFrameBuffer &input, LatestFrameBuffer &output)
    : input_(input)
    , output_(output)
{
}

void FrameProcessor::configure(const QList<StageConfig> &stages)
{
    chain_.clear();
    for (const StageConfig &config : stages) {
        auto stage = processing::makeStage(config.id.toStdString());
        if (!stage)
            continue; // parseStageConfig already rejects unknown stages
        for (const StageParameterConfig &parameter : config.parameters)
            stage->setParameter(parameter.name.toStdString(), parameter.value);
        chain_.add(std::move(stage), config.enabled);
    }
    qCDebug(logging::lcPipeline) << "Processing stages configured:" << stages.size();
}

void FrameProcessor::setStageEnabled(const std::string &id, bool enabled)
{
    if (!chain_.setEnabled(id, enabled))
        qCWarning(logging::lcPipeline, "No processing stage \"%s\"", id.c_str());
}

void FrameProcessor::setStageParameter(const std::string &id, const std::string &name, double value)
{
    if (!chain_.setParameter(id, name, value))
        qCWarning(logging::lcPipeline, "Stage \"%s\" has no parameter \"%s\"", id.c_str(), name.c_str());
}

void FrameProcessor::processLatest()
{
    auto raw = input_.take();
    if (!raw || raw->frame.image.empty())
        return;
    const cv::Mat &image = raw->frame.image;

    DisplayFrame frame;
    frame.raw = toQImage(image);
    frame.index = raw->frame.index;
    frame.captured = raw->frame.captured;
    frame.generation = raw->generation;

    // Shrink to the processing size if the frame is larger; never enlarge it.
    const cv::Mat *input = &image;
    const QSize target = processingSizeFor(QSize(image.cols, image.rows), processSize_);
    if (target != QSize(image.cols, image.rows)) {
        // INTER_AREA averages the pixels being merged, the recommended way to shrink.
        cv::resize(image, scaled_, cv::Size(target.width(), target.height()), 0.0, 0.0,
            cv::INTER_AREA);
        input = &scaled_;
    }

    if (chain_.anyEnabled()) {
        chain_.run(*input, result_, &frame.timings);
        frame.processed = toQImage(result_);
    } else {
        // Nothing to do: share the raw image instead of copying it.
        frame.processed = input == &image ? frame.raw : toQImage(*input);
    }

    if (output_.put(std::move(frame)))
        emit frameAvailable();
}

} // namespace pivision::pipeline
