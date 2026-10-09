# Replace psplash's built-in color header with ours.
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI += "file://psplash-colors.h"

do_configure:prepend() {
    cp ${WORKDIR}/psplash-colors.h ${S}/psplash-colors.h
}

