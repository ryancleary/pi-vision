#ifndef PIVISION_PIPELINE_UTILITIES_H
#define PIVISION_PIPELINE_UTILITIES_H

#include <chrono>
#include <optional>

#include <QJsonObject>
#include <QSize>
#include <QString>

#include <pivision/capture/FrameSource.h>

namespace pivision::pipeline {

// The source's name as a QString, for logs and the UI.
QString nameOf(const capture::FrameSource &source);

// The size a frame is processed at: unchanged if it already fits inside
// `limit`, otherwise shrunk to fit with its aspect ratio kept (QSize::scaled
// with Qt::KeepAspectRatio does the math). An empty `limit` means no limit.
QSize processingSizeFor(const QSize &frame, const QSize &limit);

// Events per second between two counts taken `elapsed` apart. Zero if no
// time passed, so a single sample doesn't divide by zero.
double ratePerSecond(double countDelta, std::chrono::steady_clock::duration elapsed);

// For parsers: stores `message` in `*error` (if given) and returns nullopt,
// so a failure is one line: `return failWith(error, "...");`
std::nullopt_t failWith(QString *error, const QString &message);

// The number at `key` in `object`, or nothing if it's missing or not a number.
std::optional<double> jsonNumber(const QJsonObject &object, const char *key);

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_UTILITIES_H
