# Pi 3 build only (class-target); the host build just provides tools.
#
# Graphics (eglfs, kms, gbm, gles2, linuxfb) is already selected by meta-qt6
# from our DISTRO_FEATURES (opengl, no x11), so only the general defaults are
# trimmed here. Dropped from meta-qt6's defaults: dbus, glib, icu, openssl, widgets.
# The VNC platform plugin (built by default whenever gui and network are on)
# is switched off: Qt's VNC server crashed with current TigerVNC viewers, and
# the image renders on its own display.
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

EXTRA_OECMAKE:append:class-target = " -DFEATURE_vnc=OFF"
