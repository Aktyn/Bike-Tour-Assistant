cmake_minimum_required(VERSION 3.0.0)
project(BikeTourAssistant VERSION 0.1.0 LANGUAGES C)

set(DIR_Config "./lib/Config")
set(DIR_FONTS "./lib/Fonts")
set(DIR_GUI "./lib/GUI")
set(DIR_EPD "./lib/LCD")

set(C_INCLUDE_DIRECTORIES
    ${DIR_Config}
    ${DIR_FONTS}
    ${DIR_GUI}
    ${DIR_EPD}
)

# Source directories
file(GLOB DIR_Config_sources "${DIR_Config}/*.c")
file(GLOB DIR_FONTS_sources "${DIR_FONTS}/*.c")
file(GLOB DIR_GUI_sources "${DIR_GUI}/*.c")
file(GLOB DIR_EPD_sources "${DIR_EPD}/*.c")

# Define source files using glob functions
set(C_SOURCES
    ${DIR_Config_sources}
    ${DIR_FONTS_sources}
    ${DIR_GUI_sources}
    ${DIR_EPD_sources}
)


add_executable(BikeTourAssistant main.c btferret/btlib.c ${C_SOURCES})
target_include_directories(BikeTourAssistant PUBLIC ${C_INCLUDE_DIRECTORIES})

target_compile_definitions(BikeTourAssistant PUBLIC USE_DEV_LIB)

target_link_libraries(BikeTourAssistant bluetooth)
target_link_libraries(BikeTourAssistant lgpio)
target_link_libraries(BikeTourAssistant pthread)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

