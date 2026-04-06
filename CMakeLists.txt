# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(kernel)

# ── Version auto-increment (runs every build) ───────────────────────
add_custom_target(version_bump ALL
    COMMAND ${CMAKE_COMMAND}
        -DSRC_DIR=${CMAKE_SOURCE_DIR}
        -P ${CMAKE_SOURCE_DIR}/cmake/version_bump.cmake
    COMMENT "Bumping build number"
)
add_dependencies(${CMAKE_PROJECT_NAME}.elf version_bump)
