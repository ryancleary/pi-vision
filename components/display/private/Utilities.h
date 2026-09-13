#ifndef PIVISION_DISPLAY_UTILITIES_H
#define PIVISION_DISPLAY_UTILITIES_H

#include <QJsonArray>
#include <QList>
#include <QSize>
#include <QString>
#include <QVariantMap>

#include <pivision/pipeline/StageConfig.h>

namespace pivision::display {

// [width, height], as sizes are written in capture info.json files.
QJsonArray sizeToJson(const QSize &size);

// Each stage's id, label, on/off and parameter values, for capture info.json.
QJsonArray stagesToJson(const QList<pipeline::StageConfig> &stages);

// One entry of the source dropdown: { label, spec }.
QVariantMap sourceEntry(const QString &label, const QString &spec);

} // namespace pivision::display

#endif // PIVISION_DISPLAY_UTILITIES_H
