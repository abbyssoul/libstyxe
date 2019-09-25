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

	/**
	 * Get underlaying byte stream.
	 * @return Byte stream where data is read from.
	 */
	Solace::ByteReader& buffer() noexcept { return _src; }

private:
	/// Data stream to read bytes from.
    Solace::ByteReader& _src;
};



/** Decode uint8 value from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Solace::uint8& dest);

/** Decode uint16 value from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Solace::uint16& dest);

/** Decode uint32 value from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Solace::uint32& dest);

/** Decode uint64 value from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Solace::uint64& dest);

/** Decode a *String value from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Solace::StringView& dest);

/** Decode a raw byte view from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Solace::MemoryView& dest);

/** Decode a raw byte view from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Solace::MutableMemoryView& dest);

/** Decode a path view from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, WalkPath& dest);

/** Decode a file Qid from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Qid& dest);

/** Decode a Stat struct from the stream.
 * @param dest An address where to store decoded value.
 * @return void or Error if operation has failed.
 */
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Stat& dest);



inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Solace::uint8& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}

inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Solace::uint16& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}

inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Solace::uint32& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}

inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Solace::uint64& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}


inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Solace::StringView& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}

inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Solace::MemoryView& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}

inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Solace::MutableMemoryView& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}

inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, WalkPath& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}

inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Qid& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}

inline
Solace::Result<Decoder&, Error>
operator>> (Solace::Result<Decoder&, Error>&& decoder, Stat& dest) {
	return (decoder)
		? (decoder.unwrap() >> dest)
		: Solace::mv(decoder);
}


}  // namespace styxe
#endif  // STYXE_DECODER_HPP
