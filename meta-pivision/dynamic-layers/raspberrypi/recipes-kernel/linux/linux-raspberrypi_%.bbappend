# Same fragment as the QEMU kernel, shared from recipes-kernel/linux/files.
FILESEXTRAPATHS:prepend := "${THISDIR}/../../../../recipes-kernel/linux/files:"
SRC_URI += "file://quiet-display.cfg"

