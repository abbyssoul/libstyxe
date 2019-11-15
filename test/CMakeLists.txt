set(TEST_SOURCE_FILES
        main_gtest.cpp
        ci/teamcity_messages.cpp
        ci/teamcity_gtest.cpp
        testHarnes.cpp

        test_messageParser.cpp

        test_9P2000.cpp
        test_9P2000e.cpp
        test_9P2000u.cpp
        test_9P2000L.cpp

        test_9PDirListingWriter.cpp
        test_9P2000L_dirReader.cpp
    )


enable_testing()

add_executable(test_${PROJECT_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCE_FILES})

if(STYXE_GTEST_SUPPORT)
    message(STATUS, "Using local version of GTest")

    add_subdirectory(../${STYXE_EXTERNAL_DEP_GTEST_DIR} ${CMAKE_BINARY_DIR}/gtest EXCLUDE_FROM_ALL)
else()
    message(STATUS, "Using provided version of GTest")
endif()

target_link_libraries(test_${PROJECT_NAME}
    ${PROJECT_NAME}
    gtest
    $<$<NOT:$<PLATFORM_ID:Darwin>>:rt>
    )

add_test(NAME test_${PROJECT_NAME}
    COMMAND test_${PROJECT_NAME}
    )
