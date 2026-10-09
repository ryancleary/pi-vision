#include <pivision/pipeline/StageConfig.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>

#include <pivision/processing/StageFactory.h>

namespace pivision::pipeline {

namespace {

    std::nullopt_t fail(QString *error, const QString &message)
    {
        if (error)
            *error = message;
        return std::nullopt;
    }

    std::optional<double> number(const QJsonObject &object, const char *key)
    {
        const QJsonValue value = object.value(QLatin1String(key));
        if (!value.isDouble())
            return std::nullopt;
        return value.toDouble();
    }

} // namespace

std::optional<QList<StageConfig>> parseStageConfig(const QByteArray &json, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return fail(error,
            QStringLiteral("%1 at offset %2").arg(parseError.errorString()).arg(parseError.offset));

    const QJsonValue stagesValue = document.object().value(QStringLiteral("stages"));
    if (!stagesValue.isArray())
        return fail(error, QStringLiteral("expected an object with a \"stages\" array"));

    QList<StageConfig> stages;
    QSet<QString> seen;
    const QJsonArray array = stagesValue.toArray();
    for (qsizetype i = 0; i < array.size(); ++i) {
        const QJsonObject object = array.at(i).toObject();
        const QString where = QStringLiteral("stages[%1]").arg(i);

        StageConfig stage;
        stage.id = object.value(QStringLiteral("id")).toString();
        const auto probe = processing::makeStage(stage.id.toStdString());
        if (!probe)
            return fail(error, QStringLiteral("%1: unknown stage \"%2\"").arg(where, stage.id));
        if (seen.contains(stage.id))
            return fail(error, QStringLiteral("%1: stage \"%2\" listed twice").arg(where, stage.id));
        seen.insert(stage.id);

        stage.label = object.value(QStringLiteral("label")).toString(stage.id);
        stage.enabled = object.value(QStringLiteral("enabled")).toBool(false);

        const QJsonArray parameters = object.value(QStringLiteral("parameters")).toArray();
        for (qsizetype j = 0; j < parameters.size(); ++j) {
            const QJsonObject p = parameters.at(j).toObject();
            const QString pwhere = QStringLiteral("%1.parameters[%2]").arg(where).arg(j);

            StageParameterConfig parameter;
            parameter.name = p.value(QStringLiteral("name")).toString();
            parameter.label = p.value(QStringLiteral("label")).toString(parameter.name);
            const auto value = number(p, "value");
            const auto minimum = number(p, "min");
            const auto maximum = number(p, "max");
            if (!value || !minimum || !maximum)
                return fail(error, QStringLiteral("%1: needs numeric value, min and max").arg(pwhere));
            if (*minimum > *maximum || *value < *minimum || *value > *maximum)
                return fail(error, QStringLiteral("%1: value must lie within min..max").arg(pwhere));
            parameter.value = *value;
            parameter.minimum = *minimum;
            parameter.maximum = *maximum;
            parameter.step = number(p, "step").value_or(1.0);

            if (!probe->setParameter(parameter.name.toStdString(), parameter.value))
                return fail(error, QStringLiteral("%1: stage \"%2\" has no parameter \"%3\"")
                                       .arg(pwhere, stage.id, parameter.name));
            stage.parameters.append(parameter);
        }
        stages.append(stage);
    }
    return stages;
}

} // namespace pivision::pipeline
