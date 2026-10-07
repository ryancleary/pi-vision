# pi-vision

Real-time computer vision pipeline in C++/Qt6/QML with OpenCV, running on desktop Linux and on a Raspberry Pi 3 via a custom Yocto image.

> **Status:** work in progress. See [docs/PLAN.md](docs/PLAN.md).

## Planned features
- Pluggable input: video file (default, bundled), USB webcam, image folder
- Pluggable OpenCV processing stages, toggled live from the UI
- Object detection (OpenCV DNN, ONNX model) decoupled from display rate
- QML overlay with detections, FPS and per-stage latency
- Headless mode for CI and automated tests
- Yocto image that boots straight into the app (eglfs, no desktop)

## Try it
| You have | Do this |
|---|---|
| Linux desktop | Build and run the app (instructions coming) |
| Raspberry Pi 3 | Flash the prebuilt image from Releases (coming) |
| Time and disk | Build the image yourself with kas (coming) |

## License
MIT. See [LICENSE](LICENSE).
