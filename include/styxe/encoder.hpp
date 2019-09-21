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

	/** Write a header into the output stream.
	 * @param customMessageType Byte encoding the type of the message.
	 * @param tag A message tag.
	 * @param payloadSize Size of the message payload in bytes. Does not includes message header size.
	 * @return Ref to this for fluency.
	 */
    Encoder& header(Solace::byte customMessageType, Tag tag, size_type payloadSize);

	/** Write a header into the output stream.
	 * @param type Type of the message, @see MessageType
	 * @param tag A message tag.
	 * @param payloadSize Size of the message payload in bytes. Does not includes message header size.
	 * @return Ref to this for fluency.
	 */
	Encoder& header(MessageType type, Tag tag, size_type payloadSize) {
        return header(static_cast<Solace::byte>(type), tag, payloadSize);
    }

	/** Encode an uint8 value into the output straam.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
    Encoder& encode(Solace::uint8 value);

	/** Encode an uint16 value into the output straam.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(Solace::uint16 value);

	/** Encode an uint32 value into the output straam.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(Solace::uint32 value);

	/** Encode an uint64 value into the output straam.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(Solace::uint64 value);

	/** Encode a string value into the output straam.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(Solace::StringView value);
	Encoder& encode(Solace::String const& value) = delete;

	/** Encode a Raw byte buffer into the output straam.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(Solace::MemoryView value);

	/** Encode a message header into the output stream.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(MessageHeader value);

	/** Encode a message type into the output stream.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(MessageType value) {
		return encode(static_cast<Solace::byte>(value));
	}

	/** Encode a file Qid into the output stream.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(Qid value);

	/** Encode a list of qids into the output stream.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(Solace::ArrayView<Qid> value);

	/** Encode a file stats into the output stream.
	 * @param value Value to encode.
	 * @return Ref to this for fluency.
	 */
	Encoder& encode(Stat const& value);

private:

	/// Output byte stream to write data to.
    Solace::ByteWriter& _dest;
};

}  // namespace styxe
#endif  // STYXE_ENCODER_HPP
