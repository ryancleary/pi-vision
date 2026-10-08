SUMMARY = "pi-vision image for Raspberry Pi 3"
LICENSE = "MIT"

inherit core-image

# Start from the same base as core-image-minimal: just enough to boot.
IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_LINGUAS = ""

# Development access: SSH with an empty root password.
# Removed when the release image is split out.
IMAGE_FEATURES += "ssh-server-dropbear allow-empty-password empty-root-password allow-root-login"

IMAGE_INSTALL += " \
    kernel-modules \
    mesa-megadriver \
    qtbase \
    qtbase-plugins \
    qtdeclarative \
    qtdeclarative-qmlplugins \
    qtdeclarative-tools \
    ttf-dejavu-sans \
"

