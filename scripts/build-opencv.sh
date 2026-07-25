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
    -DBUILD_LIST=core,imgproc,imgcodecs,videoio,

