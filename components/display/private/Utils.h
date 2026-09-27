#ifndef PIVISION_DISPLAY_UTILS_H
#define PIVISION_DISPLAY_UTILS_H

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QSize>
#include <QString>
#include <QVariantMap>

#include <pivision/pipeline/DetectionResult.h>
#include <pivision/pipeline/StageConfig.h>

namespace pivision::display {

// [width, height], as sizes are written in capture info.json files.
QJsonArray sizeToJson(const QSize &size);

// Each stage's id, label, on/off and parameter values, for capture info.json.
QJsonArray stagesToJson(const QList<pipeline::StageConfig> &stages);

// The class name for `classId` from the result's model, or the number itself
// if the model has no name for it.
QString classLabel(const pipeline::DetectionResult &result, int classId);

// The detector's newest result, for capture info.json: which image it looked
// at, which frame that was, and each box in that image's pixels.
QJsonObject detectionToJson(const pipeline::DetectionResult &result);

// One entry of the source dropdown: { label, spec }.
QVariantMap sourceEntry(const QString &label, const QString &spec);

} // namespace pivision::display

#endif // PIVISION_DISPLAY_UTILS_H
