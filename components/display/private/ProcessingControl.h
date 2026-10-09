#ifndef PIVISION_DISPLAY_PROCESSINGCONTROL_H
#define PIVISION_DISPLAY_PROCESSINGCONTROL_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include <pivision/pipeline/Pipeline.h>
#include <pivision/pipeline/StageConfig.h>

class QQmlEngine;
class QJSEngine;

namespace pivision::display {

// Backs the processing panel. Exposed to QML as the singleton ProcessingControl
// through exposeProcessingControl().
class ProcessingControl : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    // One entry per stage, in processing order, with current settings:
    //   { id, label, enabled, parameters: [ { name, label, value, min, max, step } ] }
    Q_PROPERTY(QVariantList stages READ stages CONSTANT)

public:
    ProcessingControl(pipeline::Pipeline *pipeline, const QList<pipeline::StageConfig> &stages,
        QObject *parent = nullptr);

    static ProcessingControl *create(QQmlEngine *, QJSEngine *engine);
    inline static ProcessingControl *instance_ = nullptr;

    QVariantList stages() const;
    // Current settings, including changes made in the UI.
    const QList<pipeline::StageConfig> &stageConfigs() const { return stages_; }

    Q_INVOKABLE void setEnabled(const QString &id, bool enabled);
    Q_INVOKABLE void setParameter(const QString &id, const QString &name, double value);

private:
    pipeline::StageConfig *find(const QString &id);

    QPointer<pipeline::Pipeline> pipeline_;
    QList<pipeline::StageConfig> stages_;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_PROCESSINGCONTROL_H
