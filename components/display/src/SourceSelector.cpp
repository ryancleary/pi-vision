#include "SourceSelector.h"

#include <QDir>
#include <QJSEngine>
#include <QVariantMap>

#include <pivision/capture/CameraDiscovery.h>
#include <pivision/logging/Logging.h>

#include "Utils.h"

namespace pivision::display {

SourceSelector::SourceSelector(pipeline::Pipeline *pipeline, const capture::CameraConfig &camera,
    const capture::TestPatternConfig &pattern, QObject *parent)
    : QObject(parent)
    , pipeline_(pipeline)
    , camera_(camera)
    , pattern_(pattern)
{
    refresh();
}

SourceSelector *SourceSelector::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(instance_);
    Q_ASSERT(engine->thread() == instance_->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
    return instance_;
}

void SourceSelector::refresh()
{
    QVariantList sources;
    for (const capture::CameraInfo &camera : capture::findCameras()) {
        const QString device = QString::fromStdString(camera.device);
        sources.append(sourceEntry(
            QStringLiteral("%1 (%2)").arg(QString::fromStdString(camera.name), device), device));
    }
    sources.append(sourceEntry(QStringLiteral("Test pattern"), QString(kTestPatternSpec)));

    qCDebug(logging::lcDisplay) << "Found" << sources.size() - 1 << "camera(s)";
    if (sources != sources_) {
        sources_ = sources;
        emit sourcesChanged();
    }
}

void SourceSelector::select(const QString &spec)
{
    QString trimmed = spec.trimmed();
    if (!pipeline_ || trimmed.isEmpty())
        return;
    // Typed paths often start with ~; nothing else expands it here.
    if (trimmed.startsWith(QStringLiteral("~/")))
        trimmed.replace(0, 1, QDir::homePath());

    qCInfo(logging::lcDisplay) << "Source selected:" << trimmed;
    if (trimmed == kTestPatternSpec)
        pipeline_->setSource(capture::makeTestPatternSource(pattern_));
    else
        pipeline_->setSource(capture::makeSourceFromSpec(trimmed.toStdString(), camera_));
}

} // namespace pivision::display
