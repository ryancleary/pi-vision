cmake_minimum_required(VERSION 3.21)
project(PiVision VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PIVISION_BUILD_TESTS "Build unit tests" ON)
# Desktop builds set this (CMakePresets.json) so each run's log lands in the repo.
# Device builds leave it empty: the app logs to the journal only.
set(PIVISION_DEV_LOG_FILE "" CACHE STRING "Default log file for development builds")
# Same idea for captures. Desktop: a folder in the repo, all kept. Device: the
# crash partition, newest 10 kept (see app/src/main.cpp).
set(PIVISION_DEV_CAPTURE_DIR "" CACHE STRING "Default capture folder for development builds")

# Pinned to Qt 6.8 to match the meta-qt6 branch used for the Pi image.
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Quick
    HINTS "$ENV{QT_ROOT_DIR}") # take the pinned install over the system one (Arch)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.9)
    message(FATAL_ERROR
        "Found Qt ${Qt6_VERSION}, but this project is pinned to 6.8.\n"
        "Set QT_ROOT_DIR to your Qt 6.8 install and reconfigure.")
endif()
message(STATUS "Using Qt ${Qt6_VERSION} from ${Qt6_DIR}")

# Pinned to 4.9 to match Scarthgap. Never highgui: it may pull in a second Qt.
find_package(OpenCV 4.9 REQUIRED COMPONENTS core imgproc videoio
    HINTS "$ENV{OPENCV_ROOT_DIR}") # take the pinned install over the system one (Arch)
if(OpenCV_VERSION VERSION_GREATER_EQUAL 5)
    message(FATAL_ERROR
        "Found OpenCV ${OpenCV_VERSION}, but this project is pinned to 4.9.\n"
        "Set OPENCV_ROOT_DIR to your OpenCV 4.9 install and reconfigure.")
endif()
message(STATUS "Using OpenCV ${OpenCV_VERSION} from ${OpenCV_DIR}")

qt_standard_project_setup(REQUIRES 6.8)

# Put every QML module in one directory so the QML compiler and Qt Creator
# can find modules defined in other subdirectories.
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)
# Qt Creator's QML code model finds our modules (PiVision, PiVision.Display)
# through this cache variable. Without it, it can't resolve them and reports
# "package import requires a version number".
set(QML_IMPORT_PATH ${QT_QML_OUTPUT_DIRECTORY} CACHE STRING "QML import path for Qt Creator" FORCE)

if(PIVISION_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    include(GoogleTest)
    list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    include(PiVisionTesting)
endif()

add_subdirectory(components)
add_subdirectory(app)

