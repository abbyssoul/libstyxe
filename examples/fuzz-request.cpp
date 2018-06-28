/*
*  Copyright 2018 Ivan Ryabov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
#include "fuzzer_utils.hpp"
#include <iomanip>


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    Solace::ReadBuffer reader(Solace::wrapMemory(data, size));

    // Case1: parse message header
    styxe::Protocol proc;
    proc.parseMessageHeader(reader)
            .then([&](styxe::Protocol::MessageHeader&& header) {
                return proc.parseRequest(header, reader);
            });

    return 0;  // Non-zero return values are reserved for future use.
}


#if !defined (__AFL_COMPILER)
int main(int argc, const char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << "fuzz <input file>..." << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        std::ifstream input(argv[i]);

        if (!input) {
            std::cerr << "Failed to open file: " << std::quoted(argv[i]) << std::endl;
            return EXIT_FAILURE;
        }

        readDataAndTest(input);
    }

    return EXIT_SUCCESS;
}
#endif
