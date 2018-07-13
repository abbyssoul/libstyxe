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

#include "styxe/9p2000.hpp"
#include "styxe/print.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace Solace;
using namespace styxe;


void dispayRequest(styxe::Protocol::Request&&) { /*no-op*/ }
void dispayResponse(styxe::Protocol::Response&&) { /*no-op*/ }


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    Solace::ReadBuffer reader(Solace::wrapMemory(data, size));

    // Case1: parse message header
    styxe::Protocol proc;

    proc.parseMessageHeader(reader)
            .then([&](styxe::Protocol::MessageHeader&& header) {
                bool const isRequest = (static_cast<Solace::byte>(header.type) % 2) == 0;
                if (isRequest) {
                    proc.parseRequest(header, reader)
                            .then(dispayRequest);
                } else {
                    proc.parseResponse(header, reader)
                            .then(dispayResponse);
                }
            });

    return 0;  // Non-zero return values are reserved for future use.
}


inline
void readDataAndTest(std::istream& in) {
    std::vector<uint8_t> buf(styxe::Protocol::MAX_MESSAGE_SIZE);
    in.read(reinterpret_cast<char*>(buf.data()), buf.size());
    size_t const got = in.gcount();

    buf.resize(got);
    buf.shrink_to_fit();

    LLVMFuzzerTestOneInput(buf.data(), got);
}



#if defined(__AFL_COMPILER)

int main(int argc, char* argv[]) {
#if defined(__AFL_LOOP)
   while (__AFL_LOOP(1000))
#endif
    {
        readDataAndTest(std::cin);
   }
}

#else  // __AFL_COMPILER

int main(int argc, const char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << "fuzz <input file>..." << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i) {
        std::ifstream input(argv[i]);
        readDataAndTest(input);
    }

    return EXIT_SUCCESS;
}
#endif  // __AFL_COMPILER
