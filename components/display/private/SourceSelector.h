#ifndef PIVISION_DISPLAY_SOURCESELECTOR_H
#define PIVISION_DISPLAY_SOURCESELECTOR_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include <pivision/capture/SourceFactory.h>
#include <pivision/pipeline/Pipeline.h>

class QQmlEngine;
class QJSEngine;

namespace pivision::display {

// Backs the source dropdown: lists cameras and the test pattern, and switches
// the pipeline to whatever the user picks. Exposed to QML as the singleton
// SourceSelector through exposeSourceSelector().
class SourceSelector : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    // Each entry is { label, spec }; pass spec to select().
    Q_PROPERTY(QVariantList sources READ sources NOTIFY sourcesChanged)

public:
    SourceSelector(pipeline::Pipeline *pipeline, const capture::CameraConfig &camera,
        const capture::TestPatternConfig &pattern, QObject *parent = nullptr);

    static SourceSelector *create(QQmlEngine *, QJSEngine *engine);
    inline static SourceSelector *instance_ = nullptr;

    QVariantList sources() const { return sources_; }

    // Rescans for cameras, so ones plugged in after startup appear.
    Q_INVOKABLE void refresh();
    // Switches to a camera device, file, stream URL, or the test pattern.
    Q_INVOKABLE void select(const QString &spec);

signals:
    void sourcesChanged();

private:
    // The spec select() takes for the built-in test pattern.
    static constexpr QLatin1String kTestPatternSpec { "test-pattern" };

    QPointer<pipeline::Pipeline> pipeline_;
    capture::CameraConfig camera_;
    capture::TestPatternConfig pattern_;
    QVariantList sources_;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_SOURCESELECTOR_H
