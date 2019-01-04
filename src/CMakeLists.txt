
set(SOURCE_FILES
        9P2000.cpp
        debug.cpp
        decoder.cpp
        encoder.cpp
        requestBuilder.cpp
        responseBuilder.cpp
        )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS})

install(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
