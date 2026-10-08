SUMMARY = "pi-vision image for Raspberry Pi 3"
LICENSE = "MIT"

inherit core-image

# Start from the same base as core-image-minimal: just enough to boot.
IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_LINGUAS = ""

# Development access: SSH with an empty root password.
# Removed when the release image is split out.
IMAGE_FEATURES += "ssh-server-dropbear allow-empty-password empty-root-password allow-root-login"

# Kernel modules each target needs. Most drivers are compiled into the kernel
# (checked against each kernel's .config), so the default is none. A target
# that needs modules adds them with a machine override, for example:
#   PIVISION_KERNEL_MODULES:qemux86-64 = "kernel-module-foo"
PIVISION_KERNEL_MODULES ?= ""

IMAGE_INSTALL += " \
    ${PIVISION_KERNEL_MODULES} \
    mesa-megadriver \
    qtbase \
    qtbase-plugins \
    qtdeclarative \
    qtdeclarative-qmlplugins \
    qtdeclarative-tools \
    ttf-dejavu-sans \
"

