SUMMARY = "pi-vision: real-time vision pipeline in Qt Quick and OpenCV"
HOMEPAGE = "https://github.com/ryancleary/pi-vision"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=35189d49d63465911468cb1f5b244583"

# Built from GitHub at a pinned commit. Update SRCREV after pushing app changes.
SRC_URI = "git://github.com/ryancleary/pi-vision.git;protocol=https;branch=main"
SRCREV = "9e73c493bdf7f71e346c6bbfbe3140ddfe6f143b"
PV = "0.1+git"
S = "${WORKDIR}/git"

DEPENDS = "qtbase qtdeclarative qtdeclarative-native opencv"

inherit qt6-cmake useradd

# Unit tests run on the desktop and in CI, not in the image.
EXTRA_OECMAKE += "-DPIVISION_BUILD_TESTS=OFF"

# Runs as its own system user with access to the GPU (video, render),
# input devices (input) and webcams (video), and nothing else.
USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM:${PN} = "--system -f render; --system -f input"
USERADD_PARAM:${PN} = "--system --no-create-home --home-dir /nonexistent \
    --shell /sbin/nologin --user-group --groups video,render,input pivision"

# Boot straight into the app, with a failure screen if it can't start.
SRC_URI += "file://pivision.service file://pivision-failed.service file://failed.qml"

inherit systemd
SYSTEMD_SERVICE:${PN} = "pivision.service"

do_install:append() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/pivision.service ${WORKDIR}/pivision-failed.service \
        ${D}${systemd_system_unitdir}
    install -d ${D}${datadir}/pivision
    install -m 0644 ${WORKDIR}/failed.qml ${D}${datadir}/pivision
}

FILES:${PN} += "${datadir}/pivision ${systemd_system_unitdir}/pivision-failed.service"
# The failure screen runs through the qml tool.
RDEPENDS:${PN} += "qtdeclarative-tools pivision-logging"

RDEPENDS:${PN} += "pivision-model-nanodet"
