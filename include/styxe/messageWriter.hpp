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
#ifndef STYXE_MESSAGEWRITER_HPP
#define STYXE_MESSAGEWRITER_HPP

#include "encoder.hpp"
#include "9p.hpp"


namespace styxe {

/** Encode a message header into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, MessageHeader value) {
	return encoder << value.messageSize
				   << value.type
				   << value.tag;

}


/**
* Helper type used to represent a message being built.
*/
struct MessageWriter {

	/**
	 * @brief Construct a new ResponseWriter.
	 * @param dest A byte writer stream where data to be written.
	 * @param tag Tag of the message being created.
	 */
	constexpr MessageWriter(Solace::ByteWriter& dest, Tag tag = kNoTag) noexcept
		: _encoder{dest}
		, _pos{dest.position()}
		, _header{headerSize(), 0, tag}
	{}

	/** Constructs a new TypedWriter
	 * @param buffer A byte stream to write the resulting message to.
	 * @param pos A position in the stream where the message header has been written.
	 * @param header Message header.
	 */
	constexpr MessageWriter(Solace::ByteWriter& buffer, Solace::ByteWriter::size_type pos, MessageHeader header) noexcept
		: _encoder{buffer}
		, _pos{pos}
		, _header{header}
	{}

	/** Finalize the message build.
	* @return ByteWriter stream
	*/
	Solace::ByteWriter& build();

	/** Get underlying data encoder
	* @return Encoder
	*/
   constexpr Encoder& encoder() noexcept { return _encoder; }

   constexpr MessageHeader header() const noexcept { return _header; }


   Encoder& messageType(Solace::byte type) {
	   _header.type = type;
	   return _encoder << _header;
   }


   Encoder& messageType(Solace::byte type, Tag tag) {
	   _header.type = type;
	   _header.tag = tag;

	   return _encoder << _header;
   }

private:
	/// Data encoder used to write data out
	Encoder							_encoder;

	/// Current position in the write stream where the message header starts
	Solace::ByteWriter::size_type	_pos;

	/// Message header
	MessageHeader					_header;
};

}  // end of namespace styxe
#endif  // STYXE_MESSAGEWRITER_HPP
