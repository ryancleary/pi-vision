# Pi 3: OpenGL ES 2 through Mesa's VC4 driver, drawn to the display by eglfs on KMS.
PACKAGECONFIG_GL = "gles2"
PACKAGECONFIG:append = " eglfs kms gbm fontconfig"

