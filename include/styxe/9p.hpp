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
#ifndef STYXE_9P_HPP
#define STYXE_9P_HPP

#include <solace/types.hpp>
#include <solace/variableSpan.hpp>


namespace styxe {

/** Network protocol uses fixed width int32 to represent size of data in bytes */
using size_type = Solace::uint32;

/** Network protocol uses fixed width int16 to represent variable datum size */
using var_datum_size_type = Solace::uint16;

/** Type of message Tag */
using Tag = Solace::uint16;

/** Type of file identifiers client uses to identify a ``current file`` on the server*/
using Fid = Solace::uint32;

/** A contigues sequence of string used by the protocol to encode a path */
using WalkPath = Solace::VariableSpan<Solace::StringView, var_datum_size_type, Solace::EncoderType::LittleEndian>;

/** String const for unknow version. */
extern const Solace::StringLiteral kUnknownProtocolVersion;

/** Special value of a message tag representing 'no tag'. */
extern const Tag kNoTag;

/** Special value of a message FID representing 'no Fid'. */
extern const Fid kNoFID;

/**
 * Fixed size common message header that all messages start with
 */
struct MessageHeader {
	size_type       messageSize{0};    //!< Size of the message including size of the header and size field itself
	Solace::byte	type{0};           //!< Type of the message. @see MessageType.
	Tag             tag{0};            //!< Message tag for concurrent messages.

	/**
	 * @brief Get the estimated payload size in bytes.
	 * @return Payload size in bytes.
	 */
	constexpr size_type payloadSize() const noexcept;
};


/**
 * Get size in bytes of the mandatory protocol message header.
 * @see MessageHeader
 * @return Size in bytes of the mandatory protocol message header.
 */
inline constexpr size_type headerSize() noexcept {
	// Note: don't use sizeof(MessageHeader) due to possible padding
	return sizeof(MessageHeader::messageSize) +
			sizeof(MessageHeader::type) +
			sizeof(MessageHeader::tag);
}

constexpr size_type MessageHeader::payloadSize() const noexcept {
	// Do we care about negative sizes?
	return messageSize - headerSize();
}


inline
constexpr MessageHeader makeHeaderWithPayload(Solace::byte type, Tag tag, size_type payloadSize) noexcept {
	return { headerSize() + payloadSize, type, tag };
}


}  // end of namespace styxe
#endif  // STYXE_9P_HPP