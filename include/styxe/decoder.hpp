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
#pragma once
#ifndef STYXE_DECODER_HPP
#define STYXE_DECODER_HPP


#include <styxe/9p2000.hpp>


namespace styxe {

/**
 * Helper class to decode data structures from the 9P2000 formatted messages.
 */
struct Decoder {

	constexpr Decoder(Solace::ByteReader& src)  noexcept
		: _src(src)
    {}

    Decoder(Decoder const&) = delete;
    Decoder& operator= (Decoder const&) = delete;

    Solace::Result<void, Solace::Error> read(Solace::uint8* dest);
    Solace::Result<void, Solace::Error> read(Solace::uint16* dest);
    Solace::Result<void, Solace::Error> read(Solace::uint32* dest);
    Solace::Result<void, Solace::Error> read(Solace::uint64* dest);
    Solace::Result<void, Solace::Error> read(Solace::StringView* dest);
    Solace::Result<void, Solace::Error> read(Solace::MemoryView* dest);
    Solace::Result<void, Solace::Error> read(Solace::MutableMemoryView* dest);
    Solace::Result<void, Solace::Error> read(Solace::Path* path);
    Solace::Result<void, Solace::Error> read(Qid* qid);
    Solace::Result<void, Solace::Error> read(Stat* stat);

    template<typename T, typename... Args>
    Solace::Result<void, Solace::Error> read(T* t, Args&&... args) {
        return read(t)
                .then([this, &args...]() { return read(std::forward<Args>(args)...); });
    }

private:
    Solace::ByteReader& _src;
};

}  // namespace styxe
#endif  // STYXE_DECODER_HPP
