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
	static size_type protocolSize(Solace::Path const& value) noexcept;
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

    constexpr Encoder(Solace::ByteWriter& dest) noexcept
        : _dest{dest}
    {}

    Encoder(Encoder const&) = delete;
    Encoder& operator= (Encoder const&) = delete;

    Encoder& header(Solace::byte customMessageType, Tag tag, size_type payloadSize);
    Encoder& header(MessageType type, Tag tag, size_type payloadSize) {
        return header(static_cast<Solace::byte>(type), tag, payloadSize);
    }

    Encoder& encode(Solace::uint8 value);
    Encoder& encode(Solace::uint16 value);
    Encoder& encode(Solace::uint32 value);
    Encoder& encode(Solace::uint64 value);
    Encoder& encode(Solace::StringView str);
    Encoder& encode(Solace::String const& str) = delete;
    Encoder& encode(Solace::MemoryView data);
    Encoder& encode(Solace::Path const& path);

	Encoder& encode(MessageHeader header);

	Encoder& encode(MessageType msgType) {
		return encode(static_cast<Solace::byte>(msgType));
	}

	Encoder& encode(Qid qid);
    Encoder& encode(Solace::ArrayView<Qid> qids);
    Encoder& encode(Stat const& stat);

private:

    Solace::ByteWriter& _dest;
};

}  // namespace styxe
#endif  // STYXE_ENCODER_HPP
