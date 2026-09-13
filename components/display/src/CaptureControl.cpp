#include "CaptureControl.h"

#include <utility>

#include <QJSEngine>
#include <QJsonArray>
#include <QJsonObject>
#include <QThreadPool>

#include <opencv2/core/version.hpp>

#include <pivision/logging/Logging.h>

#include "MetricsMonitor.h"
#include "ProcessingControl.h"
#include "Utils.h"

namespace pivision::display {

CaptureControl::CaptureControl(pipeline::Pipeline *pipeline, MetricsMonitor *metrics,
    ProcessingControl *processing, snapshot::SnapshotStore store, QString appVersion,
    QObject *parent)
    : QObject(parent)
    , pipeline_(pipeline)
    , metrics_(metrics)
    , processing_(processing)
    , store_(std::move(store))
    , appVersion_(std::move(appVersion))
{
}

CaptureControl *CaptureControl::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(instance_);
    Q_ASSERT(engine->thread() == instance_->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
    return instance_;
}

void CaptureControl::take()
{
    if (busy_)
        return;
    const auto frame = pipeline_ ? pipeline_->latestFrame() : std::nullopt;
    if (!frame) {
        emit failed(QStringLiteral("No frame to capture"));
        return;
    }

    // Gather everything now, on the GUI thread, so it all describes this moment.
    QJsonObject info = describe(*frame);
    setBusy(true);

    // Encoding and writing happen on a pool thread so the UI keeps running.
    // The images are implicitly shared copies, safe to read from there; the
    // result comes back to this thread through a queued call.
    QPointer<CaptureControl> self(this);
    QThreadPool::globalInstance()->start(
        [self, store = store_, raw = frame->raw, processed = frame->processed,
            info = std::move(info)] {
            const snapshot::SaveResult result = store.save(raw, processed, info);
            QMetaObject::invokeMethod(
                self.data(),
                [self, result] {
                    if (!self)
                        return;
                    self->setBusy(false);
                    if (result.ok()) {
                        emit self->saved(result.name);
                    } else {
                        qCWarning(logging::lcApp) << "Capture failed:" << result.error;
                        emit self->failed(result.error);
                    }
                },
                Qt::QueuedConnection);
        });
}

QJsonObject CaptureControl::describe(const pipeline::DisplayFrame &frame) const
{
    return QJsonObject {
        { QStringLiteral("app"),
            QJsonObject {
                { QStringLiteral("version"), appVersion_ },
                { QStringLiteral("qt"), QString::fromLatin1(qVersion()) },
                { QStringLiteral("opencv"), QStringLiteral(CV_VERSION) },
            } },
        { QStringLiteral("source"), pipeline_->sourceName() },
        { QStringLiteral("frame"),
            QJsonObject {
                { QStringLiteral("index"), static_cast<qint64>(frame.index) },
                { QStringLiteral("rawSize"), sizeToJson(frame.raw.size()) },
                { QStringLiteral("processedSize"), sizeToJson(frame.processed.size()) },
            } },
        { QStringLiteral("processSizeLimit"), sizeToJson(pipeline_->processSize()) },
        { QStringLiteral("stages"),
            processing_ ? stagesToJson(processing_->stageConfigs()) : QJsonArray() },
        { QStringLiteral("metrics"), metrics_ ? metrics_->toJson() : QJsonObject() },
    };
}

void CaptureControl::setBusy(bool busy)
{
    if (busy == busy_)
        return;
    busy_ = busy;
    emit busyChanged();
}

} // namespace pivision::display
