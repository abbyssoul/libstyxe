
set(SOURCE_FILES
    encoder.cpp
    decoder.cpp
    debug.cpp
    errorDomain.cpp

    9p2000.cpp
    9p2000_writer.cpp
    9p2000e.cpp
#    9p2000l.cpp
    messageWriter.cpp
    messageParser.cpp
    )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC ${CONAN_LIBS})

install(TARGETS ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
