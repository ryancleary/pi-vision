pivision_add_tests(display
    SOURCES
        UiConfigGTest.cpp
)

# The real ui.json, so a broken file fails the tests rather than the app.
target_compile_definitions(pivision_display_tests PRIVATE
    PIVISION_UI_CONFIG="${PROJECT_SOURCE_DIR}/app/config/ui.json"
)
