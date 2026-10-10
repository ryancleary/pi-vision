SUMMARY = "NanoDet-Plus object detection model for pi-vision"
DESCRIPTION = "The ONNX model pi-vision's Detection mode runs. Its description \
(nanodet-plus-m-1.5x-416.json) comes with the pivision package; both end up in \
/usr/share/pivision/models, where the app looks by default."
HOMEPAGE = "https://github.com/opencv/opencv_zoo/tree/4.9.0/models/object_detection_nanodet"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

# The same file and checksum as scripts/fetch-assets.sh, from the OpenCV Model
# Zoo at its 4.9.0 tag. Stored in git LFS, hence media.githubusercontent.com.
SRC_URI = "https://media.githubusercontent.com/media/opencv/opencv_zoo/4.9.0/models/object_detection_nanodet/object_detection_nanodet_2022nov.onnx;downloadfilename=nanodet-plus-m-1.5x-416.onnx"
SRC_URI[sha256sum] = "4b82da9944b88577175ee23a459dce2e26e6e4be573def65b1055dc2d9720186"

S = "${WORKDIR}"

inherit allarch

do_install() {
    install -d ${D}${datadir}/pivision/models
    install -m 0644 ${WORKDIR}/nanodet-plus-m-1.5x-416.onnx ${D}${datadir}/pivision/models/
}

FILES:${PN} = "${datadir}/pivision/models"
