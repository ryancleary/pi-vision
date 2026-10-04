# System tests: the whole app, run offscreen, playing a scenario (see
# components/display/include/pivision/display/Scenario.h) against OpenCV's
# vtest.avi clip. ctest -L system runs only these. On a device: scripts/pv systest.
set(PIVISION_SYSTEM_CLIP ${PROJECT_SOURCE_DIR}/assets/media/vtest-640x480-mjpeg.avi)
if(NOT EXISTS ${PIVISION_SYSTEM_CLIP})
    message(STATUS "System tests skipped: no test clip; run scripts/fetch-assets.sh (needs ffmpeg)")
    return()
endif()

add_test(NAME system.smoke
    COMMAND pivision
        --scenario ${CMAKE_CURRENT_SOURCE_DIR}/smoke.json
        --results ${CMAKE_CURRENT_BINARY_DIR}/smoke/results.json
        --capture-dir ${CMAKE_CURRENT_BINARY_DIR}/smoke/captures --capture-keep 0
        --model ${PROJECT_SOURCE_DIR}/assets/models/nanodet-plus-m-1.5x-416.json
        --log-file ${CMAKE_CURRENT_BINARY_DIR}/smoke/pivision.log
        ${PIVISION_SYSTEM_CLIP}
)
set_tests_properties(system.smoke PROPERTIES
    ENVIRONMENT QT_QPA_PLATFORM=offscreen
    LABELS system
    # The model loads on the first detection; slow CI machines need the margin.
    TIMEOUT 180
)
