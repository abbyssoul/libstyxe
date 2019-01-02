
set(SOURCE_FILES
        9P2000.cpp
        debug.cpp
        decoder.cpp
        encoder.cpp
        requestBuilder.cpp
        responseBuilder.cpp
        )

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
#    CONAN_PKG::libsolace)

install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
