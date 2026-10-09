#include "SourceSelector.h"

#include <QDir>
#include <QJSEngine>
#include <QVariantMap>

#include <pivision/capture/CameraDiscovery.h>
#include <pivision/logging/Logging.h>

namespace pivision::display {

using logging::lcDisplay;

namespace {

    const QString kTestPatternSpec = QStringLiteral("test-pattern");

    QVariantMap entry(const QString &label, const QString &spec)
    {
        return { { QStringLiteral("label"), label }, { QStringLiteral("spec"), spec } };
    }

} // namespace

SourceSelector::SourceSelector(pipeline::Pipeline *pipeline, const capture::CameraConfig &camera,
    const capture::TestPatternConfig &pattern, QObject *parent)
    : QObject(parent)
    , m_pipeline(pipeline)
    , m_camera(camera)
    , m_pattern(pattern)
{
    refresh();
}

SourceSelector *SourceSelector::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(s_instance);
    Q_ASSERT(engine->thread() == s_instance->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

QVariantList SourceSelector::sources() const { return m_sources; }

void SourceSelector::refresh()
{
    QVariantList sources;
    for (const capture::CameraInfo &camera : capture::findCameras()) {
        const QString device = QString::fromStdString(camera.device);
        sources.append(entry(
            QStringLiteral("%1 (%2)").arg(QString::fromStdString(camera.name), device), device));
    }
    sources.append(entry(QStringLiteral("Test pattern"), kTestPatternSpec));

    qCDebug(lcDisplay) << "Found" << sources.size() - 1 << "camera(s)";
    if (sources != m_sources) {
        m_sources = sources;
        emit sourcesChanged();
    }
}

void SourceSelector::select(const QString &spec)
{
    QString trimmed = spec.trimmed();
    if (!m_pipeline || trimmed.isEmpty())
        return;
    // Typed paths often start with ~; nothing else expands it here.
    if (trimmed.startsWith(QStringLiteral("~/")))
        trimmed.replace(0, 1, QDir::homePath());

    qCInfo(lcDisplay) << "Source selected:" << trimmed;
    if (trimmed == kTestPatternSpec)
        m_pipeline->setSource(capture::makeTestPatternSource(m_pattern));
    else
        m_pipeline->setSource(capture::makeSourceFromSpec(trimmed.toStdString(), m_camera));
}

} // namespace pivision::display
