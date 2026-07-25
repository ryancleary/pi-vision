#include <pivision/testsupport/QtEventLoopFixture.h>

#include <QCoreApplication>

namespace pivision::testsupport {

void QtEventLoopFixture::SetUpTestSuite()
{
    if (QCoreApplication::instance())
        return;

    // QCoreApplication keeps references to argc and argv, so they must outlive it.
    static int argc = 1;
    static char name[] = "pivision-tests";
    static char *argv[] = { name, nullptr };
    static QCoreApplication app(argc, argv);
}

} // namespace pivision::testsupport

