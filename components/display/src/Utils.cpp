#include "Utils.h"


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

QString classLabel(const pipeline::DetectionResult &result, int classId)
{
    const auto index = static_cast<size_t>(classId);
    if (classId >= 0 && result.classNames && index < result.classNames->size())
        return QString::fromStdString(result.classNames->at(index));
    return QString::number(classId);
}

QJsonObject detectionToJson(const pipeline::DetectionResult &result)
{
    QJsonArray boxes;
    for (const detection::Detection &detection : result.detections) {
        boxes.append(QJsonObject {
            { QStringLiteral("label"), classLabel(result, detection.classId) },
            { QStringLiteral("classId"), detection.classId },
            { QStringLiteral("score"), detection.score },
            // [x, y, width, height] in the pixels of imageSize.
            { QStringLiteral("box"),
                QJsonArray { detection.box.x, detection.box.y, detection.box.width,
                    detection.box.height } },
        });
    }
    return QJsonObject {
        { QStringLiteral("input"),
            result.input == pipeline::DetectionInput::Processed ? QStringLiteral("processed")
                                                                : QStringLiteral("raw") },
        // Detection runs behind the camera, so this is usually an earlier
        // frame than the one in raw.png.
        { QStringLiteral("frameIndex"), static_cast<qint64>(result.frameIndex) },
        { QStringLiteral("imageSize"), sizeToJson(result.image.size()) },
        { QStringLiteral("modelInputSize"), sizeToJson(result.modelInputSize) },
        { QStringLiteral("ms"), result.milliseconds },
        { QStringLiteral("detections"), boxes },
    };
}

QVariantMap sourceEntry(const QString &label, const QString &spec)
{
    return { { QStringLiteral("label"), label }, { QStringLiteral("spec"), spec } };
}

} // namespace pivision::display
