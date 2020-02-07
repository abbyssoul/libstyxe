# Build examples

# 9P message decode example
set(EXAMPLE_9pdecode_SOURCE_FILES 9pdecode.cpp)
add_executable(9pdecode ${EXAMPLE_9pdecode_SOURCE_FILES})
target_link_libraries(9pdecode ${PROJECT_NAME})


# 9P message corpus generator for fuzzer
set(EXAMPLE_9p_corpus_generator_SOURCE_FILES 9p-corpus.cpp)
add_executable(9p-corpus ${EXAMPLE_9p_corpus_generator_SOURCE_FILES})
target_link_libraries(9p-corpus ${PROJECT_NAME})


# 9P message parser fuzzer
set(EXAMPLE_9p_fuzz_parser_SOURCE_FILES 9p-fuzz-parser.cpp)
add_executable(9p-fuzz-parser ${EXAMPLE_9p_fuzz_parser_SOURCE_FILES})
target_link_libraries(9p-fuzz-parser ${PROJECT_NAME})


add_custom_target(examples
    DEPENDS 9pdecode 9p-corpus 9p-fuzz-parser)
