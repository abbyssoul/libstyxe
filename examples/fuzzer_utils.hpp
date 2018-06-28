#pragma once
#ifndef FUZZER_UTILS_HPP
#define FUZZER_UTILS_HPP


#include "styxe/9p2000.hpp"

#include <vector>
#include <iostream>
#include <fstream>



extern "C" int LLVMFuzzerTestOneInput(const uint8_t in[], size_t len);

inline
void readDataAndTest(std::istream& in) {
    std::vector<uint8_t> buf(styxe::Protocol::MAX_MESSAGE_SIZE);
    in.read((char*)buf.data(), buf.size());
    const size_t got = in.gcount();

    buf.resize(got);
    buf.shrink_to_fit();

    LLVMFuzzerTestOneInput(buf.data(), got);
}

#if defined(__AFL_COMPILER)

int main(int argc, char* argv[]) {
#if defined(__AFL_LOOP)
   while(__AFL_LOOP(1000))
#endif
    {
        readDataAndTest(std::cin);
   }
}
#endif  // __AFL_COMPILER

#endif  // FUZZER_UTILS_HPP
