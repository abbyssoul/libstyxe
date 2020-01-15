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



/**
 * The qid represents the server's unique identification for the file being accessed:
 * two files on the same server hierarchy are the same if and only if their qids are the same.
 */
struct Qid {
	Solace::uint64  path;		  //!< Unique identifier of a file used by the server.
	Solace::uint32	version;	  //!< Version of the file if FS supports file versioning.
	Solace::byte	type;		  //!< Type of the file this qid referse to. @see DirMode for details.
};


inline
bool operator== (Qid const& lhs, Qid const& rhs) noexcept {
	return (lhs.path == rhs.path &&
			lhs.version == rhs.version &&
			lhs.type == rhs.type);
}

inline
bool operator!= (Qid const& lhs, Qid const& rhs) noexcept {
		return !(lhs == rhs);
}

/**
 * Helper function to convert message type to byte-code.
 */
template <typename T>
constexpr Solace::byte messageCodeOf() noexcept = delete;


static_assert(sizeof(size_type) == 4, "Protocol size type is uint32");
static_assert(sizeof(Tag) == 2, "Tag is uint16");
static_assert(sizeof(Fid) == 4, "Fid is uint32");
static_assert(sizeof(var_datum_size_type) == 2, "Var datum size is uint16");
static_assert(sizeof(MessageHeader::type) == 1,	"MessageType should be 1 byte");
static_assert(headerSize() == 7,				"9P message header size is 7 bytes");

}  // end of namespace styxe
#endif  // STYXE_9P_HPP
