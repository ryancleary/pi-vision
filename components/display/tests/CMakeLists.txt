pivision_add_tests(display
    SOURCES
        UiConfigGTest.cpp
        UtilsGTest.cpp
)

# UtilsGTest tests the private helpers directly.
target_include_directories(pivision_display_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../private)

# The real ui.json and display profiles, so a broken file fails the tests rather than the app.
target_compile_definitions(pivision_display_tests PRIVATE
    PIVISION_UI_CONFIG="${PROJECT_SOURCE_DIR}/app/config/ui.json"
    # The image's display profiles; each may override ui.json for its screen.
    PIVISION_DISPLAY_PROFILES="${PROJECT_SOURCE_DIR}/meta-pivision/recipes-bsp/pivision-display/files"
)
