SUMMARY = "pi-vision display profile: GeekPi/52Pi EP-0084 7 inch touch screen"
DESCRIPTION = "Touch fixes (udev rule) and layout overrides (/etc/pivision/ui.json) \
for the EP-0084. The video side needs nothing: its EDID reports 1024x600. \
Added to the image by kas/displays/geekpi-ep0084.yml."
HOMEPAGE = "https://wiki-md.52pi.com/docs/EP-0084"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://geekpi-ep0084/ui.json file://geekpi-ep0084/99-geekpi-ep0084-touch.rules"
S = "${WORKDIR}/geekpi-ep0084"

inherit allarch

do_install() {
    install -d ${D}${sysconfdir}/pivision ${D}${nonarch_base_libdir}/udev/rules.d
    install -m 0644 ${S}/ui.json ${D}${sysconfdir}/pivision/ui.json
    install -m 0644 ${S}/99-geekpi-ep0084-touch.rules ${D}${nonarch_base_libdir}/udev/rules.d/
}

FILES:${PN} = "${sysconfdir}/pivision ${nonarch_base_libdir}/udev/rules.d"
