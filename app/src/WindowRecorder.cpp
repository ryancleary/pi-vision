#include "WindowRecorder.h"

#include <chrono>
#include <utility>

#include <QDir>
#include <QImage>
#include <QQuickWindow>
#include <QTimer>

#include <pivision/logging/Logging.h>

namespace pivision::app {

WindowRecorder::WindowRecorder(QQuickWindow *window, QString directory, QObject *parent)
    : QObject(parent)
    , window_(window)
    , directory_(std::move(directory))
    , timer_(new QTimer(this))
{
    if (!QDir().mkpath(directory_)) {
        qCWarning(logging::lcApp) << "Cannot create" << directory_ << "- not recording";
        return;
    }
    qCInfo(logging::lcApp) << "Recording the window to" << directory_ << "at" << kFps << "fps";
    connect(timer_, &QTimer::timeout, this, &WindowRecorder::grab);
    timer_->start(std::chrono::milliseconds(std::chrono::seconds(1)) / kFps);
}

void WindowRecorder::grab()
{
    if (!window_)
        return;
    const QImage image = window_->grabWindow();
    ++frame_;
    const QString name = QStringLiteral("frame-%1.jpg").arg(frame_, 5, 10, QLatin1Char('0'));
    if (!image.save(QDir(directory_).filePath(name), "JPG", kJpegQuality))
        qCWarning(logging::lcApp) << "Could not save" << name;
}

} // namespace pivision::app
