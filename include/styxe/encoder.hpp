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


#include <styxe/9p2000.hpp>


namespace styxe {

/**
 * Helper class to encode data into the protocol message format.
 */
struct Encoder {
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Solace::uint8 const& value) noexcept;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Solace::uint16 const& value) noexcept;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Solace::uint32 const& value) noexcept;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Solace::uint64 const& value) noexcept;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Solace::StringView const& value) noexcept;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
    static size_type protocolSize(Solace::String const& value) = delete;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Qid const& value) noexcept;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Stat const& value) noexcept;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Solace::ArrayView<Qid> value) noexcept;
    /**
     * Compute the number of bytes in the buffer required to store a given value.
     * @param value Value to store in the message.
     * @return Number of bytes required to represent the value given.
     */
	static size_type protocolSize(Solace::MemoryView const& value) noexcept;

public:

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

/** Encode a file Qid into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, Qid value) {
	return encoder << value.type
				   << value.version
				   << value.path;

}

/** Encode a file stats into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, Stat const& value) {
	return encoder << value.size
				   << value.type
				   << value.dev
				   << value.qid
				   << value.mode
				   << value.atime
				   << value.mtime
				   << value.length
				   << value.name
				   << value.uid
				   << value.gid
				   << value.muid;
}


/** Encode a message type into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, MessageType value) {
	return encoder << static_cast<Solace::byte>(value);
}

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

/** Encode a list of qids into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, Solace::ArrayView<Qid> value) {
	// Encode variable datum size first:
	encoder << Solace::narrow_cast<var_datum_size_type>(value.size());

	// Datum
	for (auto const& qid : value) {
		encoder << qid;
	}

	return encoder;
}

template<typename T>
Solace::Result<Encoder&, Error>
operator<< (Solace::Result<Encoder&, Error>&& encoder, T value) {
	return (encoder) ? (encoder.unwrap() << value) : Solace::mv(encoder);
}


}  // namespace styxe
#endif  // STYXE_ENCODER_HPP
