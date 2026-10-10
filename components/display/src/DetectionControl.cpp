#include "DetectionControl.h"

#include <QJSEngine>
#include <QVariantMap>

#include <pivision/logging/Logging.h>

namespace pivision::display {

DetectionControl::DetectionControl(pipeline::Pipeline *pipeline, QObject *parent)
    : QObject(parent)
    , pipeline_(pipeline)
{
    connect(pipeline, &pipeline::Pipeline::detectionAvailable, this,
        &DetectionControl::onDetectionAvailable);
    connect(pipeline, &pipeline::Pipeline::detectorStateChanged, this, &DetectionControl::stateChanged);
}

DetectionControl *DetectionControl::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(instance_);
    Q_ASSERT(engine->thread() == instance_->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
    return instance_;
}

void DetectionControl::setEnabled(bool enabled)
{
    if (!pipeline_ || enabled == pipeline_->detectionEnabled())
        return;
    pipeline_->setDetectionEnabled(enabled);
    emit enabledChanged();
}

DetectionControl::Input DetectionControl::input() const
{
    return pipeline_ && pipeline_->detectionInput() == pipeline::DetectionInput::Processed ? Processed : Raw;
}

void DetectionControl::setInput(Input input)
{
    if (!pipeline_ || input == this->input())
        return;
    pipeline_->setDetectionInput(input == Processed ? pipeline::DetectionInput::Processed
                                                    : pipeline::DetectionInput::Raw);
    emit inputChanged();
}

DetectionControl::State DetectionControl::state() const
{
    if (!pipeline_)
        return Off;
    switch (pipeline_->detectorState()) {
    case pipeline::DetectorState::Off:
        return Off;
    case pipeline::DetectorState::Loading:
        return Loading;
    case pipeline::DetectorState::Ready:
        return Ready;
    case pipeline::DetectorState::Failed:
        return Failed;
    }
    return Off;
}

void DetectionControl::onDetectionAvailable()
{
    boxes_.clear();
    modelAspect_ = 0.0;
    const auto result = pipeline_ ? pipeline_->latestDetection() : std::nullopt;
    if (result && !result->image.isNull()) {
        // Fractions of the image, so QML can place them on any view of it.
        const double width = result->image.width();
        const double height = result->image.height();
        for (const detection::Detection &detection : result->detections) {
            const auto classIndex = static_cast<size_t>(detection.classId);
            const QString label = result->classNames && classIndex < result->classNames->size()
                ? QString::fromStdString(result->classNames->at(classIndex))
                : QString::number(detection.classId);
            boxes_.append(QVariantMap {
                { QStringLiteral("x"), detection.box.x / width },
                { QStringLiteral("y"), detection.box.y / height },
                { QStringLiteral("width"), detection.box.width / width },
                { QStringLiteral("height"), detection.box.height / height },
                { QStringLiteral("label"), label },
                { QStringLiteral("score"), detection.score },
            });
        }
        if (!result->modelInputSize.isEmpty()) {
            modelAspect_ = static_cast<double>(result->modelInputSize.width())
                / result->modelInputSize.height();
        }
    }
    emit resultChanged();
}

} // namespace pivision::display
