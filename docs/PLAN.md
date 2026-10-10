# pi-vision plan

Working notes for the project. Keep this up to date as things change.

## Goal

Portfolio project covering C++/Qt6/QML, computer vision and embedded Linux.
It should be easy to try without special hardware.

Ways to run it:

1. Desktop Linux: build and run the app. A test pattern or sample video is the default input.
2. Raspberry Pi 3: flash the prebuilt image from GitHub Releases.
3. Build the image yourself with kas (takes hours and roughly 100 GB of disk).

## App design

- Inputs: video file, USB webcam (V4L2), image folder, test pattern.
- Processing stages can be turned on and off from the UI: grayscale, blur, edges, DNN detector.
- Capture, processing and display run on separate threads. Detection runs on its own
  thread against the latest frame so the display never waits on inference. Results
  carry the index of the frame they came from.
- Frames are drawn by a custom QQuickItem (texture upload in updatePaintNode).
  No Qt Multimedia. Bounding boxes are a QML overlay, not drawn into the frame.
- pipeline component (Qt Core/Gui, no QML): Pipeline owns a plain QThread and a
  FrameProducer (QObject worker, no QThread subclass) paced by a QTimer. Frames go
  through LatestFrameBuffer, which keeps only the newest frame and counts dropped ones.
  The producer signals only when the buffer goes from empty to full. Producer is
  stopped on its own thread (BlockingQueuedConnection) and owned by unique_ptr.
  capture stays Qt-free.
- display component (Qt Quick): QML module PiVision.Display. FrameView (private)
  pulls the newest frame on frameAvailable and uploads it in updatePaintNode, aspect-fit.
  Pipeline is exposed to QML as a singleton through a QML_FOREIGN registration;
  main.cpp owns it and calls exposePipeline() before loading QML.
- Layering: capture (no Qt) -> pipeline (Qt, no QML) -> display (Qt Quick) -> app.
  system (no Qt) reads CPU use, temperature and Pi throttle flags; display uses it.
- Compare UI: raw and processed frames shown side by side, as an overlay with an
  opacity slider, or picture-in-picture (processed full, raw inset). One mode enum
  drives QML states/transitions. Design adapted from an earlier QML project of mine,
  rebuilt on two FrameViews instead of Qt Multimedia. Starting mode and button
  order are in app/config/ui.json, built in like stages.json.
- QML colors/sizes live in one Theme.qml singleton. QML files in app/qml/ need
  `import PiVision` to see it: they sit in a subfolder of the module, and the
  implicit folder import doesn't pick up singletons.
- Metrics: a Metrics tab next to Stages in the side panel. Capture and display
  fps, capture-to-UI latency, per-stage times, dropped frames/s, process CPU,
  SoC temp, Pi throttle flags. Averaged over 1 s (FrameMetrics in pipeline),
  refreshed 4x/s, only while the tab is visible.
- Captures: a button saves metrics + settings as JSON plus raw/processed PNGs,
  in numbered folders (no RTC on the Pi, so the number gives the order).
  Desktop: logs/captures/, all kept. Pi: /var/crash/captures (crash partition),
  newest 10 kept; --capture-dir / --capture-keep override. Written to a
  .partial- folder and renamed, so a copy never sees half a capture.
  `pv captures TARGET` copies new ones into logs/TARGET/captures/ (--card for a
  mounted SD card).
- USB: a udev rule (pivision-usb) has systemd-mount put FAT/exFAT sticks at
  /run/media/<sdXN>, owned by pivision, mounted sync so a stick can be pulled
  as soon as a copy finishes (no eject needed). The app polls the mount table
  once a second; "Copy to USB" shows while a stick is in and copies new
  captures and crash dumps to <stick>/pivision/{captures,crashes}. Desktop
  builds look under /run/media/$USER (udisks). usb-storage, vfat, exfat and
  NLS are built into the Pi kernel (usb-storage.cfg).
- Crash partition budget (64 MB): 5 dumps x 8 MB journal max = 40 MB, plus 10
  captures at roughly 1 MB each.
- Headless mode: process a file from the command line and write JSON. Used for tests and CI.
- Process at 320x240 on the Pi.

## Code

- C++17.
- CMake only, no qmake. Presets: debug and release, Ninja, output in build/<preset>/.
- Qt pinned to 6.8, installed with aqtinstall under ~/Qt and found through QT_ROOT_DIR.
  CMake fails if it finds any other Qt version. CI uses the same version.
- OpenCV pinned to 4.9 to match Scarthgap. Built from source into ~/opt/opencv-4.9.0
  and found through OPENCV_ROOT_DIR. CMake fails on OpenCV 5.
  Build: core, imgproc, imgcodecs, videoio, dnn only. GStreamer for video, no FFmpeg
  (Arch's is too new for 4.9). No GTK, Qt, OpenCL or IPP so it behaves like the Pi build.
- OpenCV modules are listed explicitly in CMake. Never link highgui; it can pull in a second Qt.
- Component layout:

```
components/<name>/
    CMakeLists.txt
    include/pivision/<name>/   public headers
    private/                   private headers
    src/                       .cpp files
    tests/                     tests for this component
```

  Namespaces follow the path: include/pivision/capture/FrameSource.h declares
  pivision::capture::FrameSource. Each component has an alias target named the same
  way (pivision::capture) and other targets link the alias.
  Include guards are #ifndef/#define named after namespace and file
  (PIVISION_CAPTURE_FRAMESOURCE_H), no #pragma once.
  Public headers are included as <pivision/component/File.h>, private ones as "File.h".
  Concrete classes stay in private/ and are created through factory functions in the
  public headers. Tests use the public API only.
  Other targets only see include/. All headers, public and private, are listed
  in the target sources so AUTOMOC finds them.
- Tests: GTest, one executable per component (pivision_<component>_tests) built by
  pivision_add_tests() in cmake/PiVisionTesting.cmake. Files are named ClassNameGTest.cpp.
  Tests needing a Qt event loop derive from pivision::testsupport::QtEventLoopFixture
  (one QCoreApplication per executable). testsupport is only built with tests enabled.
  Tests are discovered at ctest time (PRE_TEST) so cross-compiled binaries never run
  on the build host.
- app/ holds main.cpp and QML only. Executable is pivision, QML module URI is PiVision,
  loaded with loadFromModule. UI uses Qt Quick Controls with the Basic style, imported
  directly (QtQuick.Controls.Basic) so the style is fixed at build time.

## Code conventions

Checked by .clang-tidy (clangd shows violations in the editor).

- Casts: static_cast and friends only. No C-style or functional casts (int(x)).
- Namespaces: no using-directives or using-declarations. Code inside
  pivision::X refers to other components relatively (logging::lcPipeline).
  Tests live inside the namespace of the component they test.
  Type aliases (using Name = Type) are fine.
- Class data members: camelCase with a trailing underscore (source_). Plain
  structs with public fields (Frame, DisplayFrame) use plain names.
- Trivial getters and setters are defined inline in the header. Anything that
  logs, emits a signal or needs a private type goes in the .cpp.
- Calculations: prefer a library call (QSize::scaled, cv::resize,
  std::chrono::round) or a small function whose name says what it does, with a
  comment on why. No unexplained arithmetic.
- No anonymous namespaces. Helpers live in pivision::<component>:
  private/Utils.h for ones only that component uses, the utils component
  (pivision::utils: Images.h, Strings.h, Files.h) for ones several use. The app
  has src/Utils.h in pivision::app.
- Constants that belong to a class are static constexpr members in its header.
- Test names follow Gherkin: Given<state>_When<action>_Then<outcome>, e.g.
  TEST_F(StageChainTest, GivenNoEnabledStages_WhenRunning_ThenOutputEqualsInput).
  GoogleTest advises against underscores in names because Suite_Test pairs can
  collide (https://google.github.io/googletest/faq.html); suite names here
  never contain one, so they can't.
- Test helpers are members of a fixture class (class FooTest : public
  ::testing::Test) used with TEST_F; helpers shared by several test files go
  in testsupport.

## Hardware

Raspberry Pi 3 Model B v1.2: 4x Cortex-A53 at 1.2 GHz, 1 GB RAM, VideoCore IV
(OpenGL ES 2.0). Expect roughly 1-3 FPS from the detector.

## Yocto

- Scarthgap (5.0 LTS).
- Layers: poky, meta-raspberrypi, meta-openembedded, meta-qt6 (6.8 branch), meta-pivision.
  No Boot2Qt.
- MACHINE = "raspberrypi3-64", VC4 KMS through Mesa.
- Qt runs on eglfs (KMS), no X11 or Wayland. systemd starts the app at boot.
- OpenCV built with DNN.
- populate_sdk for a cross-compile SDK.
- Build with kas-container (Docker) since Arch isn't a supported build host.
  scripts/kas.sh wraps it and sets KAS_WORK_DIR to yocto/, so fetched layers and all
  build output stay inside the repo but git-ignored; git clean -fdx removes everything.
- kas/base.yml holds everything shared; kas/<target>.yml adds the machine and any
  target-only layers. Layers are listed by branch; base.lock.yml and <target>.lock.yml
  (from `scripts/kas.sh lock`) pin exact commits. Update by re-running lock and committing.
- Targets: rpi3 (raspberrypi3-64) and qemux86-64 (Poky's QEMU machine, runs under KVM,
  for development without hardware). meta-pivision doesn't depend on meta-raspberrypi.
- meta-qt6 needs meta-python, so meta-openembedded provides meta-oe and meta-python.
- Distro pivision: Poky plus systemd, opengl, no x11/wayland/vulkan/ptest.
  -Os everywhere except OpenCV (-O2), since inference is where the CPU time goes.
- Image pivision-image: core-image-minimal base, dropbear SSH with empty root password
  (dev only, removed for release), Qt base/declarative, qml tool, one font, Mesa.
- Kernel modules: only what each target needs (PIVISION_KERNEL_MODULES with machine
  overrides), checked against the kernel .config. Pi 3: VC4, Ethernet and USB HID are
  built in; uvcvideo (generic USB webcams) and hid-multitouch are modules.
- qtbase (target only): eglfs/kms/gbm/gles2 come from the distro features. Trimmed
  defaults: no dbus, glib, icu, openssl, widgets. optimize-size and ltcg on. Qt's VNC
  platform plugin stays in for testing (QT_QPA_PLATFORM=vnc).
- TFT touchscreen support is deferred.
- Distro is named pivision-os (not pivision) because the distro name is added to
  OVERRIDES and would collide with the pivision recipe name.
- OpenCV in the image: core, imgproc, imgcodecs, videoio; V4L only (webcams, no video
  file playback on the device); jpeg/png. dnn added in Phase 4. The desktop build keeps
  GStreamer and dnn on purpose.
- App recipe pivision_git.bb: built from GitHub at a pinned SRCREV (push, then bump),
  qt6-cmake, tests off. Runs as system user pivision (groups video, render, input).
- Boot: psplash until the app starts; app QML shows "Starting..."; if the app keeps
  failing, OnFailure shows a fallback QML screen with the last log lines.
  No login prompt on the display; serial console keeps a login for debugging.
- pivision.service runs as pivision, restarts on failure; after 3 failures in 60 s,
  OnFailure starts pivision-failed.service (qml + failed.qml, shows journal lines).
- Flicker-free boot: CONFIG_FRAMEBUFFER_CONSOLE_DEFERRED_TAKEOVER=y and no CONFIG_LOGO
  (fragment quiet-display.cfg, both kernels). Disabling fbcon outright needs
  CONFIG_EXPERT because DRM fbdev emulation (used by psplash) forces it on.
  psplash colors match the app background (#1b1f24). Kernel args: quiet loglevel=3
  vt.global_cursor_default=0. Pi: DISABLE_SPLASH=1 (untested until flashed).
- Pi-only bbappends live in meta-pivision/dynamic-layers/raspberrypi via
  BBFILES_DYNAMIC, so the QEMU build doesn't need meta-raspberrypi.
- rm_work is on to keep disk use down (~35-50 GB for a full build).
- CI (GitHub Actions, ubuntu-24.04, GCC 13): installs Qt 6.8.3 with aqt and builds
  OpenCV with scripts/build-opencv.sh (POSIX sh, same script used locally). Both are
  cached; the OpenCV cache key is the script's hash. Runs the debug preset and ctest.
  No formatting check; clang-format is run by hand before milestones.
  Skipped for commits that only touch meta-pivision/, kas/, targets/, docs/, *.md,
  scripts/kas.sh or scripts/pv (paths-ignore).
- CI only builds the desktop app. The image is built locally and attached to Releases.
- Bring-up order: boot a stock Qt example on eglfs first, then add the app.

## Build tool

scripts/pv (Python, standard library only):
targets, build desktop [--preset], build image TARGET, build all, size TARGET,
run TARGET (QEMU: KVM when available, snapshot by default, SSH on 2222, VNC on 5900,
--display sdl|gtk|none, --memory, --persist, --dry-run).
Targets are defined in targets/<name>.json (kas file, machine, image, delivery).
build all runs desktop debug and release then every image, keeps going on failure,
and logs each step to build-logs/<timestamp>/ with a summary.
clean desktop [--preset], clean image TARGET [--recipe NAME] (bitbake -c cleansstate),
clean all [--deep [-y]]. Without --deep, Yocto keeps downloads and sstate so a
rebuild takes minutes; --deep removes all of yocto/ and the fetched assets. logs/
is never cleaned.

## Assets

The model and sample video are not committed. scripts/fetch-assets.sh downloads
them and checks SHA256. The Yocto recipe fetches them with checksums as well.
No AGPL models (rules out Ultralytics YOLO).

## Phases

### 0. Repo
- [x] Create GitHub repo and push the initial commit
- [x] Description and topics
- [ ] Pin on profile (after phase 1)

### 1. Desktop skeleton
- [x] Top-level CMake and presets
- [x] Empty QML window
- [x] Frame source interface
- [x] First source (test pattern)
- [x] Capture thread
- [x] Frame display item
- [x] Tests
- [x] CI

### 2. Yocto bring-up
- [x] kas config with pinned layers
- [x] Distro, image recipe, qtbase config
- [x] Per-target kas files, qemux86-64 target, size optimization
- [x] Build tool (scripts/pv)
- [x] Image boots in QEMU (eglfs and VNC tested)
- [ ] Image boots on the Pi and runs a QML test on eglfs

### 3. App in the image
- [x] App recipe (runs in QEMU)
- [x] systemd unit, psplash, failure screen
- [x] Flicker-free boot display
- [x] App starts at boot, no login prompt on the display (QEMU)
- [ ] Same on the Pi (needs flashing)

### 4. Pipeline and detection
- [x] Webcam and video file sources
- [x] Stage interface and basic stages, with tests; pipeline delivers raw + processed
- [x] Compare UI: side by side, overlay with opacity, picture-in-picture
- [x] Metrics panel
- [x] Capture button, storage on the crash partition, pv captures
- [x] Copy captures to USB from the app
- [ ] Detector thread and QML overlay
- [ ] Headless mode
- [ ] Rebuild the image and measure on the Pi

### 5. Release
- [ ] Image on GitHub Releases
- [ ] README: demo GIF, architecture diagram, quickstart, design notes

## Open questions

- Detection model (permissive license, small enough for the Pi 3).
- Sample video source (CC0 or public domain).
- Check on the Pi that /sys/devices/platform/soc/soc:firmware/get_throttled
  exists and is readable by the pivision user (metrics show n/a otherwise).
