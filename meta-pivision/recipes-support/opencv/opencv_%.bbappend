# Only what pi-vision uses: core, imgproc, imgcodecs, videoio.
# Video input is V4L only (USB webcams); no GStreamer or FFmpeg, so no video
# file playback on the device. dnn is added when detection lands in Phase 4.
# Built at -O2 (see pivision.conf), since this is where the CPU time goes.
PACKAGECONFIG = "v4l jpeg png"
EXTRA_OECMAKE:append = " -DBUILD_LIST=core,imgproc,imgcodecs,videoio"

