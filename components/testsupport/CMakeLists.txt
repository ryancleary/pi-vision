add_library(pivision_testsupport STATIC
    include/pivision/testsupport/QtEventLoopFixture.h
    src/QtEventLoopFixture.cpp
)
add_library(pivision::testsupport ALIAS pivision_testsupport)

target_include_directories(pivision_testsupport
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pivision_testsupport
    PUBLIC GTest::gtest Qt6::Core
)

