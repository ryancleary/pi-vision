#ifndef PIVISION_TESTSUPPORT_QTEVENTLOOPFIXTURE_H
#define PIVISION_TESTSUPPORT_QTEVENTLOOPFIXTURE_H

#include <gtest/gtest.h>

namespace pivision::testsupport {

// Base fixture for tests that need a Qt event loop: queued signals across
// threads, timers, QSignalSpy::wait(). Creates a single QCoreApplication for
// the whole test executable the first time a test using this fixture runs.
class QtEventLoopFixture : public ::testing::Test {
protected:
    static void SetUpTestSuite();
};

} // namespace pivision::testsupport

#endif // PIVISION_TESTSUPPORT_QTEVENTLOOPFIXTURE_H
