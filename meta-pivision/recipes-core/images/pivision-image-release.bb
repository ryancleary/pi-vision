SUMMARY = "pi-vision release image for Raspberry Pi 3: no SSH, no root login"
LICENSE = "MIT"

# The dev image, without its remote access. Built with:
#   scripts/pv build image rpi3 --release
# Captures and crash logs come off the device on a USB stick instead.
require recipes-core/images/pivision-image.bb

IMAGE_FEATURES:remove = "ssh-server-dropbear allow-empty-password empty-root-password allow-root-login"
