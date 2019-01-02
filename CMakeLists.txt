cmake_minimum_required(VERSION 3.3)
project(styxe)

# Custom build target to generate code coverage:
# Don't want this to run on every build.
option(STYXE_GTEST_SUPPORT "Build without GTEST" ON)
option(COVERALLS "Generate coveralls data" OFF)
option(SANITIZE "Enable 'sanitize' compiler flag" OFF)

# Include common compile flag
include(cmake/compile_flags.cmake)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# Build project dependencies
set(STYXE_EXTERNAL_DEP_GTEST_DIR "external/gtest/googletest"
    CACHE PATH "The path to the Google Test framework.")

include_directories(include)

add_subdirectory(src)
add_subdirectory(test EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)

# ---------------------------------
# Show build configuration status:
# ---------------------------------
message(STATUS, "BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS, "CXXFLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS, "STYXE_SANITIZE: ${STYXE_SANITIZE}")
message(STATUS, "STYXE_COVERALLS: ${STYXE_COVERALLS}")
message(STATUS, "STYXE_GTEST_SUPPORT: ${STYXE_GTEST_SUPPORT}")
