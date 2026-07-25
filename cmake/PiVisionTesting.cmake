# pivision_add_tests(<component> SOURCES <files...> [LIBRARIES <targets...>])
#
# Builds pivision_<component>_tests from SOURCES, links it with pivision::<component>,
# GTest and any extra LIBRARIES, and registers each test with CTest.
function(pivision_add_tests component)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "SOURCES;LIBRARIES")
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "pivision_add_tests(${component}): SOURCES is required")
    endif()

    set(target pivision_${component}_tests)
    add_executable(${target} ${ARG_SOURCES})
    target_link_libraries(${target} PRIVATE
        pivision::${component}
        GTest::gtest_main
        ${ARG_LIBRARIES}
    )

    # Discover tests when CTest runs, not at build time, so cross-compiled
    # test binaries don't have to execute on the build machine.
    gtest_discover_tests(${target}
        DISCOVERY_MODE PRE_TEST
        PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endfunction()

