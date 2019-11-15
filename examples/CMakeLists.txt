# Build examples

# 9P message decode example
set(EXAMPLE_9pdecode_SOURCE_FILES 9pdecode.cpp)
add_executable(9pdecode ${EXAMPLE_9pdecode_SOURCE_FILES})
target_link_libraries(9pdecode ${PROJECT_NAME})


# 9P message corpus generator for fuzzer
set(EXAMPLE_corpus_generator_SOURCE_FILES corpus_generator.cpp)
add_executable(corpus_generator ${EXAMPLE_corpus_generator_SOURCE_FILES})
target_link_libraries(corpus_generator ${PROJECT_NAME})


# 9P message parser fuzzer
set(EXAMPLE_fuzz_parser_SOURCE_FILES corpus_generator.cpp)
add_executable(fuzz-parser ${EXAMPLE_fuzz_parser_SOURCE_FILES})
target_link_libraries(fuzz-parser ${PROJECT_NAME})


add_custom_target(examples
    DEPENDS 9pdecode corpus_generator fuzz-parser)
