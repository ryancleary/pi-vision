cmake_minimum_required(VERSION 3.21)
project(PiVision VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PIVISION_BUILD_TESTS "Build unit tests" ON)

# Pinned to Qt 6.8 to match the meta-qt6 branch used for the Pi image.
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Quick)
if(Qt6_VERSION VERSION_GREATER_EQUAL 6.9)
    message(FATAL_ERROR
        "Found Qt ${Qt6_VERSION}, but this project is pinned to 6.8.\n"
        "Set QT_ROOT_DIR to your Qt 6.8 install and reconfigure.")
endif()
message(STATUS "Using Qt ${Qt6_VERSION} from ${Qt6_DIR}")

# Pinned to 4.9 to match Scarthgap. Never highgui: it may pull in a second Qt.
find_package(OpenCV 4.9 REQUIRED COMPONENTS core imgproc videoio)
if(OpenCV_VERSION VERSION_GREATER_EQUAL 5)
    message(FATAL_ERROR
        "Found OpenCV ${OpenCV_VERSION}, but this project is pinned to 4.9.\n"
        "Set OPENCV_ROOT_DIR to your OpenCV 4.9 install and reconfigure.")
endif()
message(STATUS "Using OpenCV ${OpenCV_VERSION} from ${OpenCV_DIR}")

qt_standard_project_setup(REQUIRES 6.8)

if(PIVISION_BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)
endif()

add_subdirectory(app)
add_subdirectory(components)


