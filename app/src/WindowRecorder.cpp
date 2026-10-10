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
    connect(timer_, &QTimer::timeout, this, &WindowRecorder::grab);
}

void WindowRecorder::start()
{
    if (window_)
        connect(window_, &QQuickWindow::frameSwapped, this, &WindowRecorder::begin, Qt::SingleShotConnection);
}

void WindowRecorder::begin()
{
    if (timer_->isActive())
        return;
    times_.setFileName(QDir(directory_).filePath(QStringLiteral("times.txt")));
    if (!QDir().mkpath(directory_) || !times_.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(logging::lcApp) << "Cannot write to" << directory_ << "- not recording";
        return;
    }
    timesOut_.setDevice(&times_);
    qCInfo(logging::lcApp) << "Recording the window to" << directory_ << "at up to" << kFps << "fps";
    clock_.start();
    timer_->start(std::chrono::milliseconds(std::chrono::seconds(1)) / kFps);
}

void WindowRecorder::grab()
{
    if (!window_)
        return;
    const qint64 elapsedMs = clock_.elapsed();
    const QImage image = window_->grabWindow();
    if (!firstGrabDone_) {
        firstGrabDone_ = true;
        return;
    }
    ++frame_;
    timesOut_ << elapsedMs << '\n';
    timesOut_.flush();
    const QString name = QStringLiteral("frame-%1.jpg").arg(frame_, 5, 10, QLatin1Char('0'));
    if (!image.save(QDir(directory_).filePath(name), "JPG", kJpegQuality))
        qCWarning(logging::lcApp) << "Could not save" << name;
}

} // namespace pivision::app
