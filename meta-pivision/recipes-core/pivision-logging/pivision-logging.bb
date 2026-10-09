SUMMARY = "pi-vision logging policy: in-memory journal, crash dumps and captures on the SD card"
DESCRIPTION = "Keeps the systemd journal in RAM, installs crash-dump, which \
pi-vision services run after stopping to save the journal when they crash, and \
creates the folder on the crash partition where the app saves captures."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://journald-pivision.conf file://crash-dump file://tmpfiles-pivision.conf"
S = "${WORKDIR}"

inherit allarch

do_install() {
    install -d ${D}${sysconfdir}/systemd/journald.conf.d
    install -m 0644 ${WORKDIR}/journald-pivision.conf \
        ${D}${sysconfdir}/systemd/journald.conf.d/10-pivision.conf

    install -d ${D}${sysconfdir}/tmpfiles.d
    install -m 0644 ${WORKDIR}/tmpfiles-pivision.conf ${D}${sysconfdir}/tmpfiles.d/pivision.conf

    install -d ${D}${libexecdir}/pivision
    install -m 0755 ${WORKDIR}/crash-dump ${D}${libexecdir}/pivision/crash-dump
}

FILES:${PN} = "${sysconfdir}/systemd/journald.conf.d ${sysconfdir}/tmpfiles.d ${libexecdir}/pivision"
# journalctl comes with systemd.
RDEPENDS:${PN} = "systemd"
