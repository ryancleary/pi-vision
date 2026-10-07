# pi-vision: plan and decisions

Single source of truth for the project. Update it when a decision changes.

## Goal
Portfolio project showing C++/Qt6/QML + computer vision + embedded Linux,
aimed at medical imaging, robotics, drone, and image-processing roles.
Anyone can build and run it without special hardware.

## How people test it
1. **Desktop Linux:** build and run the app; a sample video is the default input.
2. **Raspberry Pi 3:** flash the prebuilt `.wic` image from GitHub Releases.
3. **Full build:** one `kas` command builds the image (hours, ~100 GB disk).

## App architecture
- **Inputs (pluggable):** video file (default), USB webcam via V4L2, image folder.
- **Stages (pluggable, toggled from QML):** grayscale, blur, edges, DNN detector.
- **Threading:** capture → processing → display. Detection runs on its own
  thread on the latest frame; display never waits on inference. Results are
  tagged with the frame they came from.
- **Rendering:** custom `QQuickItem`, texture upload in `updatePaintNode()`.
  No Qt Multimedia. Boxes drawn as a QML overlay, not burned into frames.
- **Metrics panel:** FPS and per-stage latency (desktop vs Pi is part of the demo).
- **Headless mode:** CLI flag processes a file and writes JSON. Used by CI/GTest.
- **Processing resolution:** 320×240 on the Pi.
- **Language:** C++20 (GCC 13 in Scarthgap supports it).

## Target hardware
Raspberry Pi 3 Model B v1.2: 4× Cortex-A53 @ 1.2 GHz, 1 GB RAM,
VideoCore IV (OpenGL ES 2.0 only). Expect ~1–3 FPS detection.

## Yocto
- Release: Scarthgap (5.0 LTS).
- Layers: poky, meta-raspberrypi, meta-openembedded, meta-qt6, meta-pivision.
- `MACHINE = "raspberrypi3-64"`, VC4 KMS via Mesa.
- Qt on eglfs (KMS), no X11/Wayland. App launched by systemd at boot.
- OpenCV with DNN enabled.
- `populate_sdk` for a cross-compile SDK.
- Build with `kas-container` (host is Arch, which Yocto doesn't support).
- CI builds the desktop app only; image built locally and attached to Releases.

## Guardrails
- **Qt version pin:** the minimum Qt version is the one the pinned meta-qt6
  branch provides. CI builds against that exact version so newer APIs from the
  desktop install can't slip in.
- **Model license:** no AGPL models (e.g. Ultralytics YOLO). Permissive only.
- **Large assets:** not committed. `scripts/fetch-assets.sh` downloads them and
  verifies SHA256; the Yocto recipe fetches with checksums too.
- **Bring-up order:** prove the image boots a stock Qt demo on eglfs before
  adding the app, so graphics problems are isolated.

## Phases
### 0. Repo
- [ ] Create empty GitHub repo, push scaffold
- [ ] Description, topics, pin on profile

### 1. Desktop skeleton
- [ ] Dev environment (Qt version pin, OpenCV, CMake, Qt Creator kit)
- [ ] CMake project, QML window, video file → frames on screen
- [ ] GTest wired up
- [ ] GitHub Actions: build + tests

### 2. Yocto bring-up
- [ ] kas config, layers pinned
- [ ] Image boots on Pi 3, stock Qt demo on eglfs

### 3. App in the image
- [ ] App recipe + systemd unit
- [ ] Skeleton app autostarts on the Pi

### 4. Pipeline and detection
- [ ] Stage interface + classical stages, with tests
- [ ] DNN detector on its own thread, QML overlay
- [ ] Metrics panel, headless mode
- [ ] Rebuild image, measure on Pi

### 5. Release
- [ ] Flashable image on GitHub Releases
- [ ] Demo GIF, architecture diagram, quickstart, design decisions in README

## Open decisions
- Final repo name (currently `pi-vision`)
- Exact meta-qt6 branch / Qt version
- Detection model (permissive license, small enough for the Pi 3)
- Sample video source (CC0 / public domain)
