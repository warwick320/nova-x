# version_bump.cmake — runs every build via add_custom_target
file(READ "${SRC_DIR}/version.txt" VER)
string(STRIP "${VER}" VER)

file(READ "${SRC_DIR}/build_number.txt" BN)
string(STRIP "${BN}" BN)
math(EXPR BN "${BN} + 1")
file(WRITE "${SRC_DIR}/build_number.txt" "${BN}\n")

set(FULL "v${VER}+build.${BN}")
message(STATUS "Version: ${FULL}")

file(WRITE "${SRC_DIR}/main/version.h"
     "#pragma once\n"
     "#define KERNEL_VERSION \"${FULL}\"\n"
     "#define KERNEL_VERSION_MAJOR ${VER}\n"
     "#define KERNEL_BUILD_NUMBER ${BN}\n")

file(WRITE "${SRC_DIR}/readme.md"
     "# nova x kernel devloping\n"
     "version ${FULL}\n")
