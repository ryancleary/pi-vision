#ifndef PIVISION_DISPLAY_USBCONTROL_H
#define PIVISION_DISPLAY_USBCONTROL_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

class QQmlEngine;
class QJSEngine;

namespace pivision::display {

// Backs the "Copy to USB" button: notices a USB stick mounted under the mount
// root and copies captures and crash dumps onto it. The system does the
// mounting (on the Pi, a udev rule with systemd-mount); this only looks for
// the result. Exposed to QML as the singleton Usb through exposeUsb().
class UsbControl : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Usb)
    QML_SINGLETON
    // True while a stick is mounted.
    Q_PROPERTY(bool present READ present NOTIFY presentChanged)
    // Where it's mounted, e.g. /run/media/sda1; empty when there's none.
    Q_PROPERTY(QString mountPoint READ mountPoint NOTIFY presentChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    // `crashDirectory` may be empty (desktop builds have no crash dumps).
    UsbControl(QString mountRoot, QString captureDirectory, QString crashDirectory,
        QObject *parent = nullptr);

    static UsbControl *create(QQmlEngine *, QJSEngine *engine);
    inline static UsbControl *instance_ = nullptr;

    bool present() const { return !mountPoint_.isEmpty(); }
    QString mountPoint() const { return mountPoint_; }
    bool busy() const { return busy_; }

    // Copies whatever the stick doesn't have yet into <stick>/pivision/.
    // The result arrives as copied() or failed().
    Q_INVOKABLE void copyAll();

signals:
    void presentChanged();
    void busyChanged();
    void copied(const QString &summary);
    void failed(const QString &message);

private:
    void checkMounts();
    void setBusy(bool busy);

    QString mountRoot_;
    QString captureDirectory_;
    QString crashDirectory_;
    QString mountPoint_;
    QTimer pollTimer_;
    bool busy_ = false;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_USBCONTROL_H
