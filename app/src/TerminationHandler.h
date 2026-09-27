#ifndef PIVISION_APP_TERMINATIONHANDLER_H
#define PIVISION_APP_TERMINATIONHANDLER_H

#include <QObject>

class QSocketNotifier;

namespace pivision::app {

// Turns SIGTERM and SIGINT into QCoreApplication::quit(), so `systemctl stop`
// (SIGTERM) and Ctrl+C shut the app down normally and it exits with 0. By
// default both signals kill it, which systemd counts as a failure.
//
// A signal handler may only call async-signal-safe functions, so it writes a
// byte to a socket and Qt's event loop does the rest, as described in
// https://doc.qt.io/qt-6/unix-signals.html
class TerminationHandler : public QObject {
public:
    explicit TerminationHandler(QObject *parent = nullptr);
    ~TerminationHandler() override;

private:
    static void onSignal(int signal);
    void onSocketReadable();

    // [0] written by the signal handler, [1] read by the event loop.
    inline static int sockets_[2] = { -1, -1 };
    QSocketNotifier *notifier_ = nullptr;
};

} // namespace pivision::app

#endif // PIVISION_APP_TERMINATIONHANDLER_H
