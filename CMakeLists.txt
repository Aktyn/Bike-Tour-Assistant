cmake_minimum_required(VERSION 3.0.0)
project(BikeTourAssistant VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(DIR_Config "./lib/Config")
set(DIR_FONTS "./lib/Fonts")
set(DIR_GUI "./lib/GUI")
set(DIR_EPD "./lib/LCD")

set(DIR_DISPLAY "./src/display")
set(DIR_BLUETOOTH "./src/bluetooth")
set(DIR_CAMERA "./src/camera")
set(DIR_CORE "./src/core")

set(C_INCLUDE_DIRECTORIES
    ${DIR_Config}
    ${DIR_FONTS}
    ${DIR_GUI}
    ${DIR_EPD}
    "./src"
)

# Source directories
file(GLOB DIR_Config_sources "${DIR_Config}/*.c")
file(GLOB DIR_FONTS_sources "${DIR_FONTS}/*.c")
file(GLOB DIR_GUI_sources "${DIR_GUI}/*.c")
file(GLOB DIR_EPD_sources "${DIR_EPD}/*.c")

file(GLOB common_sources "./src/*.cpp")
file(GLOB DIR_DISPLAY_sources "${DIR_DISPLAY}/*.cpp")
file(GLOB DIR_BLUETOOTH_sources "${DIR_BLUETOOTH}/*.cpp")
file(GLOB DIR_CAMERA_sources "${DIR_CAMERA}/*.cpp")
file(GLOB DIR_CORE_sources "${DIR_CORE}/*.cpp")

# Define source files using glob functions
set(C_SOURCES
    src/bluetooth/btferret/btlib.c
    ${DIR_Config_sources}
    ${DIR_FONTS_sources}
    ${DIR_GUI_sources}
    ${DIR_EPD_sources}
)

set(CXX_SOURCES
    main.cpp
    ${common_sources}
    ${DIR_DISPLAY_sources}
    ${DIR_BLUETOOTH_sources}
    ${DIR_CAMERA_sources}
    ${DIR_CORE_sources}
        src/core/common.h
)

add_executable(BikeTourAssistant ${C_SOURCES} ${CXX_SOURCES})
target_include_directories(BikeTourAssistant PUBLIC ${C_INCLUDE_DIRECTORIES})

target_compile_definitions(BikeTourAssistant PUBLIC USE_DEV_LIB)
target_compile_definitions(BikeTourAssistant PUBLIC USE_DEBUG)

target_link_libraries(BikeTourAssistant bluetooth)
target_link_libraries(BikeTourAssistant lgpio)
target_link_libraries(BikeTourAssistant pthread)
target_link_libraries(BikeTourAssistant jpeg)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
