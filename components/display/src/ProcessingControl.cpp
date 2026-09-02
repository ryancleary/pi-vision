#include "ProcessingControl.h"

#include <QJSEngine>
#include <QVariantMap>

#include <pivision/logging/Logging.h>

namespace pivision::display {

ProcessingControl::ProcessingControl(
    pipeline::Pipeline *pipeline, const QList<pipeline::StageConfig> &stages, QObject *parent)
    : QObject(parent)
    , pipeline_(pipeline)
    , stages_(stages)
{
}

ProcessingControl *ProcessingControl::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(instance_);
    Q_ASSERT(engine->thread() == instance_->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
    return instance_;
}

QVariantList ProcessingControl::stages() const
{
    QVariantList list;
    for (const pipeline::StageConfig &stage : stages_) {
        QVariantList parameters;
        for (const pipeline::StageParameterConfig &p : stage.parameters) {
            parameters.append(QVariantMap {
                { QStringLiteral("name"), p.name },
                { QStringLiteral("label"), p.label },
                { QStringLiteral("value"), p.value },
                { QStringLiteral("min"), p.minimum },
                { QStringLiteral("max"), p.maximum },
                { QStringLiteral("step"), p.step },
            });
        }
        list.append(QVariantMap {
            { QStringLiteral("id"), stage.id },
            { QStringLiteral("label"), stage.label },
            { QStringLiteral("enabled"), stage.enabled },
            { QStringLiteral("parameters"), parameters },
        });
    }
    return list;
}

void ProcessingControl::setEnabled(const QString &id, bool enabled)
{
    pipeline::StageConfig *stage = find(id);
    if (!stage || !pipeline_)
        return;
    stage->enabled = enabled;
    qCInfo(logging::lcDisplay) << "Stage" << id << (enabled ? "enabled" : "disabled");
    pipeline_->setStageEnabled(id, enabled);
}

void ProcessingControl::setParameter(const QString &id, const QString &name, double value)
{
    pipeline::StageConfig *stage = find(id);
    if (!stage || !pipeline_)
        return;
    for (pipeline::StageParameterConfig &parameter : stage->parameters) {
        if (parameter.name == name)
            parameter.value = value;
    }
    qCDebug(logging::lcDisplay) << "Stage" << id << name << "=" << value;
    pipeline_->setStageParameter(id, name, value);
}

pipeline::StageConfig *ProcessingControl::find(const QString &id)
{
    for (pipeline::StageConfig &stage : stages_) {
        if (stage.id == id)
            return &stage;
    }
    qCWarning(logging::lcDisplay) << "No processing stage" << id;
    return nullptr;
}

} // namespace pivision::display
