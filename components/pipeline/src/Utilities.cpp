#include "Utilities.h"

#include <QJsonValue>

namespace pivision::pipeline {

QString nameOf(const capture::FrameSource &source)
{
    return QString::fromStdString(source.name());
}

QSize processingSizeFor(const QSize &frame, const QSize &limit)
{
    const bool fits = frame.width() <= limit.width() && frame.height() <= limit.height();
    if (limit.isEmpty() || fits)
        return frame;
    return frame.scaled(limit, Qt::KeepAspectRatio);
}

double ratePerSecond(double countDelta, std::chrono::steady_clock::duration elapsed)
{
    const double seconds = std::chrono::duration<double>(elapsed).count();
    return seconds > 0.0 ? countDelta / seconds : 0.0;
}

std::nullopt_t failWith(QString *error, const QString &message)
{
    if (error)
        *error = message;
    return std::nullopt;
}

std::optional<double> jsonNumber(const QJsonObject &object, const char *key)
{
    const QJsonValue value = object.value(QLatin1String(key));
    if (!value.isDouble())
        return std::nullopt;
    return value.toDouble();
}

} // namespace pivision::pipeline
