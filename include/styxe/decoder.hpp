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

#include <solace/utils.hpp>  // fwd<>


namespace styxe {

/**
 * Helper class to decode data structures from the 9P2000 formatted messages.
 */
struct Decoder {

	/** Construct a Decoder that reads from the given stream.
	 * @param src A byte stream to decode data from.
	 */
	constexpr Decoder(Solace::ByteReader& src) noexcept
		: _src{src}
    {}

    Decoder(Decoder const&) = delete;
    Decoder& operator= (Decoder const&) = delete;

	/** Decode uint8 value from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Solace::uint8* dest);

	/** Decode uint16 value from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Solace::uint16* dest);

	/** Decode uint32 value from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Solace::uint32* dest);

	/** Decode uint64 value from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Solace::uint64* dest);

	/** Decode a *String value from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Solace::StringView* dest);

	/** Decode a raw byte view from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Solace::MemoryView* dest);

	/** Decode a raw byte view from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Solace::MutableMemoryView* dest);

	/** Decode a path view from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(WalkPath* dest);

	/** Decode a file Qid from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Qid* dest);

	/** Decode a Stat struct from the stream.
	 * @param dest An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
	Solace::Result<void, Error> read(Stat* dest);

	/** Decode a sequence of values from the input stream.
	 * @param t An address where to store decoded value.
	 * @param args An address where to store decoded value.
	 * @return void or Error if operation has failed.
	 */
    template<typename T, typename... Args>
	Solace::Result<void, Error> read(T* t, Args&&... args) {
		Solace::Result<void, Error> r{Solace::types::okTag};
		(r = read(t)) && ((r = read(Solace::fwd<Args>(args))) && ...);

		return r;
    }

private:
	/// Data stream to read bytes from.
    Solace::ByteReader& _src;
};

}  // namespace styxe
#endif  // STYXE_DECODER_HPP
