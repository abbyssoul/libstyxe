# TODO(abbyssoul): Disabled as not supported by clang (without gold linker that is)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")

set(TEST_SOURCE_FILES
        main_gtest.cpp
        ci/teamcity_messages.cpp
        ci/teamcity_gtest.cpp

        test_9P2000.cpp
        test_9PMessageBuilder.cpp
        )


if(STYXE_GTEST_SUPPORT)
    message(STATUS, "Using local version of GTest")

    add_subdirectory(../${STYXE_EXTERNAL_DEP_GTEST_DIR} ${CMAKE_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
    include_directories(SYSTEM ${gtest_SOURCE_DIR}/include)
else()
    message(STATUS, "Using provided version of GTest")
endif()

add_executable(test_${PROJECT_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCE_FILES})

add_test(NAME test_${PROJECT_NAME}
    COMMAND test_${PROJECT_NAME}
    )

target_link_libraries(test_${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}
    gtest
    $<$<NOT:$<PLATFORM_ID:Darwin>>:rt>
    )

#if(UNIX AND NOT APPLE)
#else()
#    target_link_libraries(test_${PROJECT_NAME} PRIVATE
#        ${PROJECT_NAME}
#        gtest)
#endif()
