#include "Utils.h"

#include <QJsonObject>

namespace pivision::display {

QJsonArray sizeToJson(const QSize &size)
{
    return QJsonArray { size.width(), size.height() };
}

QJsonArray stagesToJson(const QList<pipeline::StageConfig> &stages)
{
    QJsonArray list;
    for (const pipeline::StageConfig &stage : stages) {
        QJsonObject parameters;
        for (const pipeline::StageParameterConfig &parameter : stage.parameters)
            parameters.insert(parameter.name, parameter.value);
        list.append(QJsonObject {
            { QStringLiteral("id"), stage.id },
            { QStringLiteral("label"), stage.label },
            { QStringLiteral("enabled"), stage.enabled },
            { QStringLiteral("parameters"), parameters },
        });
    }
    return list;
}

QVariantMap sourceEntry(const QString &label, const QString &spec)
{
    return { { QStringLiteral("label"), label }, { QStringLiteral("spec"), spec } };
}

} // namespace pivision::display
