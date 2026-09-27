#include "TerminationHandler.h"

#include <csignal>

#include <QCoreApplication>
#include <QSocketNotifier>

#include <sys/socket.h>
#include <unistd.h>

#include <pivision/logging/Logging.h>

namespace pivision::app {

TerminationHandler::TerminationHandler(QObject *parent)
    : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets_) != 0) {
        qCWarning(logging::lcApp) << "Cannot create the signal socket; SIGTERM will kill the app";
        return;
    }
    notifier_ = new QSocketNotifier(sockets_[1], QSocketNotifier::Read, this);
    connect(notifier_, &QSocketNotifier::activated, this, &TerminationHandler::onSocketReadable);

    struct sigaction action = {};
    action.sa_handler = &TerminationHandler::onSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    ::sigaction(SIGTERM, &action, nullptr);
    ::sigaction(SIGINT, &action, nullptr);
}

TerminationHandler::~TerminationHandler()
{
    std::signal(SIGTERM, SIG_DFL);
    std::signal(SIGINT, SIG_DFL);
    for (int &socket : sockets_) {
        if (socket >= 0)
            ::close(socket);
        socket = -1;
    }
}

void TerminationHandler::onSignal(int signal)
{
    const char byte = static_cast<char>(signal);
    // Nothing useful to do if this fails, and nothing else is safe to call here.
    [[maybe_unused]] const ssize_t written = ::write(sockets_[0], &byte, sizeof(byte));
}

void TerminationHandler::onSocketReadable()
{
    char byte = 0;
    if (::read(sockets_[1], &byte, sizeof(byte)) == sizeof(byte))
        qCInfo(logging::lcApp) << "Stopping on signal" << static_cast<int>(byte);
    QCoreApplication::quit();
}

} // namespace pivision::app
