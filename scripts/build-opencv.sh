#!/bin/sh
# Builds the pinned OpenCV version with only the modules pi-vision uses,
# configured to match the Raspberry Pi image as closely as possible.
#
# Usage: scripts/build-opencv.sh [install-prefix]
# Default prefix: $HOME/opt/opencv-<version>
set -eu

OPENCV_VERSION=4.9.0
PREFIX=${1:-"$HOME/opt/opencv-$OPENCV_VERSION"}
SRC_DIR=${OPENCV_SRC_DIR:-"$HOME/src/opencv-$OPENCV_VERSION"}

if [ ! -d "$SRC_DIR" ]; then
    git clone --depth 1 --branch "$OPENCV_VERSION" \
        https://github.com/opencv/opencv.git "$SRC_DIR"
fi

# FFmpeg off: recent FFmpeg releases are too new for OpenCV 4.9.
# GTK/Qt off: no highgui, so OpenCV never links a second Qt.
# OpenCL/IPP off: the Pi has neither, so desktop numbers stay comparable.
cmake -S "$SRC_DIR" -B "$SRC_DIR/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DBUILD_LIST=core,imgproc,imgcodecs,videoio,dnn \
    -DWITH_FFMPEG=OFF -DWITH_GSTREAMER=ON -DWITH_V4L=ON \
    -DWITH_GTK=OFF -DWITH_QT=OFF -DWITH_OPENCL=OFF -DWITH_IPP=OFF \
    -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF \
    -DBUILD_opencv_apps=OFF -DBUILD_opencv_python3=OFF -DBUILD_JAVA=OFF

cmake --build "$SRC_DIR/build"
cmake --install "$SRC_DIR/build"

# Fail loudly if the install didn't produce what CMake needs to find OpenCV.
CONFIG="$PREFIX/lib/cmake/opencv4/OpenCVConfig.cmake"
if [ ! -f "$CONFIG" ]; then
    echo "error: $CONFIG not found after install" >&2
    exit 1
fi

echo "OpenCV $OPENCV_VERSION installed to $PREFIX"

