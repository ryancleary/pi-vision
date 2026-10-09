#ifndef PIVISION_PIPELINE_STAGECONFIG_H
#define PIVISION_PIPELINE_STAGECONFIG_H

#include <optional>

#include <QByteArray>
#include <QList>
#include <QString>

namespace pivision::pipeline {

struct StageParameterConfig {
    QString name;  // as the stage knows it, e.g. "size"
    QString label; // shown next to the slider
    double value = 0.0;
    double minimum = 0.0;
    double maximum = 1.0;
    double step = 1.0;
};

struct StageConfig {
    QString id; // a built-in stage, e.g. "blur"
    QString label;
    bool enabled = false;
    QList<StageParameterConfig> parameters;
};

// Reads the processing stages from JSON (app/config/stages.json):
//
//   { "stages": [
//       { "id": "blur", "label": "Blur", "enabled": false,
//         "parameters": [ { "name": "size", "label": "Kernel size",
//                           "value": 5, "min": 3, "max": 15, "step": 2 } ] },
//       ... ] }
//
// Stages run in the order listed. The file ships inside the app, so anything
// wrong with it is an error: invalid JSON, a missing field, an unknown stage
// or parameter, a duplicate stage. Returns nothing and sets `error` if so.
std::optional<QList<StageConfig>> parseStageConfig(const QByteArray &json, QString *error = nullptr);

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_STAGECONFIG_H
