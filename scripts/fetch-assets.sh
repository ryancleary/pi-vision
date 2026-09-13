#!/bin/sh
# Downloads the files pi-vision needs but doesn't keep in git (models), and
# checks each against a pinned SHA-256 so a changed or corrupted download is
# caught instead of used. Files that are already present and correct are
# skipped, so it's safe to run any time.
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

