cmake_minimum_required(VERSION 3.3)
project(styxe)


# Custom build target to generate code coverage:
# Don't want this to run on every build.
option(STYXE_GTEST_SUPPORT "Build without GTEST" ON)
option(STYXE_COVERALLS "Generate coveralls data" OFF)
option(STYXE_SANITIZE "Enable 'sanitize' compiler flag" OFF)
option(STYXE_LIBSOLACE_SUPPORT "Build without libsolace" ON)

# Include common compile flag
include(cmake/compile_flags.cmake)

# Require at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# ---------------------------------
# Build project dependencies
# ---------------------------------
set(STYXE_EXTERNAL_DEP_GTEST_DIR "external/gtest/googletest"
    CACHE PATH "The path to the Google Test framework.")

if (STYXE_LIBSOLACE_SUPPORT)
    if(STYXE_GTEST_SUPPORT)
        set(SOLACE_GTEST_SUPPORT OFF CACHE BOOL "Don't use dependencie's gtest" FORCE)
        message(STATUS, "Turning solace GTEST off")
    endif()

    message(STATUS, "Adding local libsolce")
    add_subdirectory(external/libsolace EXCLUDE_FROM_ALL)
    include_directories(${solace_SOURCE_DIR}/include)
endif()

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
message(STATUS, "STYXE_LIBSOLACE_SUPPORT: ${STYXE_LIBSOLACE_SUPPORT}")
message(STATUS, "STYXE_GTEST_SUPPORT: ${STYXE_GTEST_SUPPORT}")
