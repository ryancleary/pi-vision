SUMMARY = "pi-vision USB sticks: mount FAT/exFAT sticks for copying captures"
DESCRIPTION = "A udev rule and helper that mount USB sticks at /run/media/<name>, \
writable by the pivision user and synchronous, so the app can copy captures and \
crash dumps onto them and the stick can be pulled as soon as it's done."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://99-pivision-usb.rules file://usb-mount"
S = "${WORKDIR}"

inherit allarch

do_install() {
    install -d ${D}${sysconfdir}/udev/rules.d
    install -m 0644 ${WORKDIR}/99-pivision-usb.rules ${D}${sysconfdir}/udev/rules.d/

    install -d ${D}${libexecdir}/pivision
    install -m 0755 ${WORKDIR}/usb-mount ${D}${libexecdir}/pivision/usb-mount
}

FILES:${PN} = "${sysconfdir}/udev/rules.d ${libexecdir}/pivision/usb-mount"
# systemd-mount and the udev rules this one builds on come with systemd.
RDEPENDS:${PN} = "systemd"
