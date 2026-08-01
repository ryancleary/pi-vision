# Pi 3 build only (class-target); the host build just provides tools.
#
# Graphics (eglfs, kms, gbm, gles2, linuxfb) is already selected by meta-qt6
# from our DISTRO_FEATURES (opengl, no x11), so only the general defaults are
# trimmed here. Dropped from meta-qt6's defaults: dbus, glib, icu, openssl, widgets.
# The VNC platform plugin is built whenever gui and network are on.
PACKAGECONFIG_DEFAULT:class-target = "\
    accessibility \
    fontconfig \
    gui \
    harfbuzz \
    jpeg \
    libinput \
    ltcg \
    optimize-size \
    png \
    udev \
    xkbcommon \
    zlib \
    zstd \
"

