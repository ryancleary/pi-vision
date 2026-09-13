#include "UsbControl.h"

#include <chrono>
#include <utility>

#include <QDir>
#include <QJSEngine>
#include <QPointer>
#include <QThreadPool>

#include <unistd.h>

#include <pivision/logging/Logging.h>
#include <pivision/snapshot/Export.h>

namespace pivision::display {

UsbControl::UsbControl(
    QString mountRoot, QString captureDirectory, QString crashDirectory, QObject *parent)
    : QObject(parent)
    , mountRoot_(std::move(mountRoot))
    , captureDirectory_(std::move(captureDirectory))
    , crashDirectory_(std::move(crashDirectory))
{
    pollTimer_.setInterval(kPollInterval);
    connect(&pollTimer_, &QTimer::timeout, this, &UsbControl::checkMounts);
    pollTimer_.start();
    checkMounts();
}

UsbControl *UsbControl::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(instance_);
    Q_ASSERT(engine->thread() == instance_->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
    return instance_;
}

void UsbControl::checkMounts()
{
    const QStringList sticks =
        snapshot::mountPointsUnder(mountRoot_, snapshot::currentMountPoints());
    // With more than one stick in, use the first; it's rare enough not to need a choice.
    const QString mountPoint = sticks.isEmpty() ? QString() : sticks.first();
    if (mountPoint == mountPoint_)
        return;
    mountPoint_ = mountPoint;
    qCInfo(logging::lcDisplay) << (present() ? "USB stick at" : "USB stick removed") << mountPoint_;
    emit presentChanged();
}

void UsbControl::copyAll()
{
    if (busy_ || !present())
        return;
    setBusy(true);

    const QDir target(QDir(mountPoint_).filePath(QStringLiteral("pivision")));
    // Copying runs on a pool thread so the UI keeps running; the result
    // comes back to this thread through a queued call.
    QPointer<UsbControl> self(this);
    QThreadPool::globalInstance()->start([self, target, captures = captureDirectory_,
                                             crashes = crashDirectory_] {
        const snapshot::ExportResult captureResult =
            snapshot::exportNumberedFolders(captures, target.filePath(QStringLiteral("captures")));
        snapshot::ExportResult crashResult;
        if (captureResult.ok() && !crashes.isEmpty())
            crashResult = snapshot::exportNumberedFolders(crashes, target.filePath(QStringLiteral("crashes")));
        // The stick is mounted with "sync" on the Pi; this covers desktops, which aren't.
        ::sync();

        const QString error = !captureResult.ok() ? captureResult.error : crashResult.error;
        const QString copiedText = crashes.isEmpty()
            ? QStringLiteral("%1 capture(s)").arg(captureResult.copied)
            : QStringLiteral("%1 capture(s) and %2 crash dump(s)")
                  .arg(captureResult.copied)
                  .arg(crashResult.copied);
        const QString summary = QStringLiteral("Copied %1; safe to remove").arg(copiedText);
        QMetaObject::invokeMethod(
            self.data(),
            [self, error, summary] {
                if (!self)
                    return;
                self->setBusy(false);
                if (error.isEmpty()) {
                    qCInfo(logging::lcDisplay) << summary;
                    emit self->copied(summary);
                } else {
                    qCWarning(logging::lcDisplay) << "Copy to USB failed:" << error;
                    emit self->failed(error);
                }
            },
            Qt::QueuedConnection);
    });
}

void UsbControl::setBusy(bool busy)
{
    if (busy == busy_)
        return;
    busy_ = busy;
    emit busyChanged();
}

} // namespace pivision::display
