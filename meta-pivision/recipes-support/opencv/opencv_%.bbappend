# Only what pi-vision uses: core, imgproc, imgcodecs, videoio, and dnn for
# object detection (it runs the ONNX model; protobuf comes from meta-oe).
# Video input is V4L only (USB webcams); no GStreamer or FFmpeg, so no video
# file playback on the device.
# Built at -O2 (see the distro config), since this is where the CPU time goes.
PACKAGECONFIG = "v4l jpeg png dnn"
EXTRA_OECMAKE:append = " -DBUILD_LIST=core,imgproc,imgcodecs,videoio,dnn"
