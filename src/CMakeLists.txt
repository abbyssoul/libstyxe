cmake_minimum_required(VERSION 3.12)

set(SOURCE_FILES
    encoder.cpp
    decoder.cpp
    errorDomain.cpp

    9p2000_writer.cpp
    9p2000.cpp
    9p2000e.cpp
    9p2000u.cpp
    9p2000L.cpp
    messageWriter.cpp
    messageParser.cpp
    )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS})

install(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
