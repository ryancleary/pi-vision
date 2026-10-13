#!/bin/sh
# Downloads the files pi-vision needs but doesn't keep in git (the model, a test
# photo and the system-test clip), and checks each against a pinned SHA-256 so
# a changed or corrupted download is caught instead of used. Files that are
# already present and correct are skipped, so it's safe to run any time.
#
# Usage: scripts/fetch-assets.sh
set -eu

REPO=$(cd "$(dirname "$0")/.." && pwd)

# fetch <path in repo> <url> <sha256>
fetch() {
    dest="$REPO/$1"
    url=$2
    sha=$3

    if [ -f "$dest" ] && echo "$sha  $dest" | sha256sum -c --status; then
        echo "ok       $1"
        return
    fi

    mkdir -p "$(dirname "$dest")"
    tmp="$dest.download"
    echo "fetching $1"
    curl -fsSL --retry 3 -o "$tmp" "$url"
    if ! echo "$sha  $tmp" | sha256sum -c --status; then
        rm -f "$tmp"
        echo "error: $1 does not match its pinned SHA-256" >&2
        exit 1
    fi
    mv "$tmp" "$dest"
}

# NanoDet-Plus-m 1.5x 416, 80 COCO classes, Apache-2.0. From the OpenCV Model
# Zoo at its 4.9.0 tag, to match the pinned OpenCV.
fetch assets/models/nanodet-plus-m-1.5x-416.onnx \
    https://media.githubusercontent.com/media/opencv/opencv_zoo/4.9.0/models/object_detection_nanodet/object_detection_nanodet_2022nov.onnx \
    4b82da9944b88577175ee23a459dce2e26e6e4be573def65b1055dc2d9720186

# A photo for tests: a football player with a ball (OpenCV's sample data,
# Apache-2.0), at the 4.9.0 tag.
fetch assets/media/messi5.jpg \
    https://raw.githubusercontent.com/opencv/opencv/4.9.0/samples/data/messi5.jpg \
    1d570e49654e84c7a943918537bd9e5e1ef82920152e147c834006e235be97c9

# Pedestrians walking (OpenCV's sample data, Apache-2.0), at the 4.9.0 tag:
# the input for the system tests in tests/system. Stored as DivX, which the
# Pi can't decode (no FFmpeg or GStreamer in the image), so it's converted
# with ffmpeg to Motion-JPEG AVI, which OpenCV reads with its own built-in
# decoder: 640x480 like the camera, 30 fps (frames repeated) so the pipeline
# sees a camera's load, first 30 s. The app loops a file at its end. Only the
# converted clip is kept; it isn't checksummed, as ffmpeg versions encode
# differently.
clip=assets/media/vtest-640x480-mjpeg.avi
original=assets/media/vtest.avi
if [ -f "$REPO/$clip" ]; then
    echo "ok       $clip"
elif ! command -v ffmpeg >/dev/null; then
    echo "skipped  $clip: needs ffmpeg (system tests won't run without it)" >&2
else
    fetch "$original" \
        https://raw.githubusercontent.com/opencv/opencv/4.9.0/samples/data/vtest.avi \
        45cddc9490be69345cbdab64ca583be65987e864ca408038e648db99e10516cf
    echo "making   $clip"
    ffmpeg -v error -y -i "$REPO/$original" -t 30 -vf "scale=640:480,fps=30" \
        -c:v mjpeg -q:v 8 -an -f avi "$REPO/$clip.tmp"
    mv "$REPO/$clip.tmp" "$REPO/$clip"
    rm "$REPO/$original"
fi
