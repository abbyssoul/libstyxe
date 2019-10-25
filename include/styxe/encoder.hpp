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
#ifndef STYXE_ENCODER_HPP
#define STYXE_ENCODER_HPP


#include <solace/stringView.hpp>
#include <solace/byteWriter.hpp>

#include <solace/utils.hpp>  // mv<>
#include <solace/result.hpp>
#include <solace/error.hpp>


namespace styxe {

/** Network protocol uses fixed width int32 to represent size of data in bytes */
using size_type = Solace::uint32;

/**
 * Helper class to encode data into the protocol message format.
 */
struct Encoder {

	/** Construct an Encoder that writes data to the given stream.
	 * @param dest A byte stream to write data to.
	 */
	constexpr Encoder(Solace::ByteWriter& dest) noexcept
        : _dest{dest}
    {}

    Encoder(Encoder const&) = delete;
    Encoder& operator= (Encoder const&) = delete;

	/**
	 * Get underlaying output stream.
	 * @return Byte stream where data is being written.
	 */
	Solace::ByteWriter& buffer() noexcept { return _dest; }

private:

	/// Output byte stream to write data to.
    Solace::ByteWriter& _dest;
};



size_type protocolSize(Solace::uint8 const& value) noexcept;
size_type protocolSize(Solace::uint16 const& value) noexcept;
size_type protocolSize(Solace::uint32 const& value) noexcept;
size_type protocolSize(Solace::uint64 const& value) noexcept;
size_type protocolSize(Solace::StringView const& value) noexcept;
size_type protocolSize(Solace::MemoryView const& value) noexcept;



/** Encode an uint8 value into the output straam.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
Encoder& operator<< (Encoder& encoder, Solace::uint8 value);

/** Encode an uint16 value into the output straam.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
Encoder& operator<< (Encoder& encoder, Solace::uint16 value);

/** Encode an uint32 value into the output straam.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
Encoder& operator<< (Encoder& encoder, Solace::uint32 value);

/** Encode an uint64 value into the output straam.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
Encoder& operator<< (Encoder& encoder, Solace::uint64 value);

/** Encode a string value into the output straam.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
Encoder& operator<< (Encoder& encoder, Solace::StringView value);

/** Encode a Raw byte buffer into the output straam.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
Encoder& operator<< (Encoder& encoder, Solace::MemoryView value);

}  // namespace styxe
#endif  // STYXE_ENCODER_HPP
