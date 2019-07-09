# Examples of using libstyxe

[Examples](../examples) directory in the repository contains some examples of using this library.
Please note that this library does not come with network capabilities thus examples only limited to writing and reading
9P messages from files. This functionality can be advantageous for fuzz testing the library.

  * [9pdecode](../examples/9pdecode.cpp) Is a useful CLI tool to read serialised 9P messages from files and print then in a human readable format.
  * [Corpus generator](../examples/corpus_generator.cpp) is another CLI tool to create all supported 9P messages - including 9P2000.e - and write them into files.
  * [fuzz-parser](../examples/fuzz-parser.cpp) is a ALF / fuzz tester entry point. It serves to fuzz-test the parser.
