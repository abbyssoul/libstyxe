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
#ifndef STYXE_MESSAGEPARSER_HPP
#define STYXE_MESSAGEPARSER_HPP

#include "styxe/9p2000.hpp"
#include "styxe/9p2000e.hpp"
#include "styxe/9p2000u.hpp"
#include "styxe/9p2000L.hpp"


namespace styxe {

/// Type representing a request message
using RequestMessage = std::variant<
							Request::Version,
							Request::Auth,
							Request::Flush,
							Request::Attach,
							Request::Walk,
							Request::Open,
							Request::Create,
							Request::Read,
							Request::Write,
							Request::Clunk,
							Request::Remove,
							Request::Stat,
							Request::WStat,
							// 9P2000.u extended messages
							_9P2000U::Request::Auth,
							_9P2000U::Request::Attach,
							_9P2000U::Request::Create,
							_9P2000U::Request::WStat,
							// 9p2000.e exrta messages
							_9P2000E::Request::Session,
							_9P2000E::Request::ShortRead,
							_9P2000E::Request::ShortWrite,
							// 9P2000.L extra messages
							_9P2000L::Request::StatFS,
							_9P2000L::Request::LOpen,
							_9P2000L::Request::LCreate,
							_9P2000L::Request::Symlink,
							_9P2000L::Request::MkNode,
							_9P2000L::Request::Rename,
							_9P2000L::Request::ReadLink,
							_9P2000L::Request::GetAttr,
							_9P2000L::Request::SetAttr,
							_9P2000L::Request::XAttrWalk,
							_9P2000L::Request::XAttrCreate,
							_9P2000L::Request::ReadDir,
							_9P2000L::Request::FSync,
							_9P2000L::Request::Lock,
							_9P2000L::Request::GetLock,
							_9P2000L::Request::Link,
							_9P2000L::Request::MkDir,
							_9P2000L::Request::RenameAt,
							_9P2000L::Request::UnlinkAt
							>;

/// Type representing a response message
using ResponseMessage = std::variant<
							Response::Version,
							Response::Auth,
							Response::Attach,
							Response::Error,
							Response::Flush,
							Response::Walk,
							Response::Open,
							Response::Create,
							Response::Read,
							Response::Write,
							Response::Clunk,
							Response::Remove,
							Response::Stat,
							Response::WStat,
							// 9P2000.u extended messages
							_9P2000U::Response::Error,
							_9P2000U::Response::Stat,
							// 9p2000.e exrta messages
							_9P2000E::Response::Session,
							_9P2000E::Response::ShortRead,
							_9P2000E::Response::ShortWrite,
							// 9P2000.L extra messages
							_9P2000L::Response::LError,
							_9P2000L::Response::StatFS,
							_9P2000L::Response::LOpen,
							_9P2000L::Response::LCreate,
							_9P2000L::Response::Symlink,
							_9P2000L::Response::MkNode,
							_9P2000L::Response::Rename,
							_9P2000L::Response::ReadLink,
							_9P2000L::Response::GetAttr,
							_9P2000L::Response::SetAttr,
							_9P2000L::Response::XAttrWalk,
							_9P2000L::Response::XAttrCreate,
							_9P2000L::Response::ReadDir,
							_9P2000L::Response::FSync,
							_9P2000L::Response::Lock,
							_9P2000L::Response::GetLock,
							_9P2000L::Response::Link,
							_9P2000L::Response::MkDir,
							_9P2000L::Response::RenameAt,
							_9P2000L::Response::UnlinkAt
							>;

using RequestParseFunc = Solace::Result<RequestMessage, Error> (*)(Solace::ByteReader& );
using ResponseParseFunc = Solace::Result<ResponseMessage, Error> (*)(Solace::ByteReader& );

using RequestParseTable = std::array<RequestParseFunc, 1 << 8*sizeof(MessageHeader::type)>;
using ResponseParseTable = std::array<ResponseParseFunc, 1 << 8*sizeof(MessageHeader::type)>;


/**
 * 9p2000 message parser for unspecified protocol version.
 * Note: this parser only able to parse basic messages of the protocol.
 * In practice it means Version negotiation request.
 *
 * Once protocol version is negotiated - create a Parser instance for that version.
 * @see createParser for details.
 */
struct UnversionedParser {

	/**
	 * Parse 9P message header from a byte buffer.
	 * @param byteStream Byte buffer to read message header from.
	 * @return Resulting message header if parsed successfully or an error otherwise.
	 */
	Solace::Result<MessageHeader, Error>
	parseMessageHeader(Solace::ByteReader& byteStream) const;

	/**
	 * Parse a version request message from a byte stream.
	 * @param header Fixed-size message header.
	 * @param byteStream A byte stream to parse a request from.
	 * @return Either a parsed request message or an error.
	 */
	Solace::Result<Request::Version, Error>
	parseVersionRequest(MessageHeader header, Solace::ByteReader& byteStream) const;

	/**
	 * Get maximum message size in bytes. That includes size of a message header and the payload.
	 * @return
	 */
	size_type maxMessageSize() const noexcept { return headerSize() + maxPayloadSize; }

	size_type          maxPayloadSize;		//!< Initial value of the maximum message payload size in bytes.
};



/**
 * Base class for 9p message parsers.
 * This class incapsulate common parts between request and response parser.
 */
struct ParserBase {
	/// Type alias for messge code -> StringView mapping.
	using VersionedNameMapper = Solace::StringView (*)(Solace::byte);

	/**
	 * Construct a new ParserBase
	 * @param payloadSize Maximum negoriated payload size in bytes. Note: message size is payload + header..
	 * @param nameMapper A function to convert message code to strings. It depends on selected protocol version.
	 */
	ParserBase(size_type payloadSize, VersionedNameMapper nameMapper) noexcept
		: _maxPayloadSize{payloadSize}
		, _nameMapper{nameMapper}
	{}

	/**
	 * Get a string representation of the message name given the op-code.
	 * @param messageType Message op-code to convert to a string.
	 * @return A string representation of a given message code.
	 */
	Solace::StringView messageName(Solace::byte messageType) const;

	/**
	 * Get maximum message size in bytes. That includes size of a message header and the payload.
	 * @return Negotiated maximum size of a message in bytes.
	 */
	size_type maxMessageSize() const noexcept { return headerSize() + _maxPayloadSize; }

private:
	size_type				_maxPayloadSize;		/// Initial value of the maximum message payload size in bytes.
	VersionedNameMapper		_nameMapper;

};


/**
 * An implementation of 9p response message parser.
 *
 * The protocol is state-full as version, supported extentions and messages size are negotiated.
 * Thus this info must be preserved during communication. Instance of this class serves this purpose as well as
 * helps with message parsing.
 *
 * @note The implementation of the protocol does not allocate memory for any operation.
 * Message parser acts on an instance of the user provided Solace::ByteReader and any message data such as
 * name string or data read from a file is actually a pointer to the underlying ReadBuffer storage.
 * Thus it is user's responsibility to manage lifetime of that buffer.
 * (That is not technically correct as current implementation does allocate memory when dealing with Solace::Path
 * objects as there is no currently availiable PathView version)
 *
 * In order to create 9P2000 messages please @see MessageWriter.
 */
struct ResponseParser :
		public ParserBase {

	/**
	 * Construct a new instance of the parser.
	 * @param maxPayloadSize Maximum message paylaod size in bytes.
	 * @param nameMapper A pointer to a map-function to convert message op-codes to message name string.
	 * @param parserTable A table of version specific opcode parser methods.
	 */
	ResponseParser(size_type maxPayloadSize, VersionedNameMapper nameMapper, ResponseParseTable	parserTable) noexcept
		: ParserBase{maxPayloadSize, nameMapper}
		, _versionedResponseParser{Solace::mv(parserTable)}
	{}

	/**
	 * Parse 9P Response type message from a byte buffer.
	 * This is the primiry method used by a client to parse response from the server.
	 *
	 * @param header Message header.
	 * @param data Byte buffer to read message content from.
	 * @return Resulting message if parsed successfully or an error otherwise.
	 */
	Solace::Result<ResponseMessage, Error>
	parseResponse(MessageHeader const& header, Solace::ByteReader& data) const;

private:
	ResponseParseTable	_versionedResponseParser;  /// Parser V-table.
};



/**
 * An implementation of 9p request message parser.
 *
 * The protocol is state-full as version, supported extentions and messages size are negotiated.
 * Thus this info must be preserved during communication. Instance of this class serves this purpose as well as
 * helps with message parsing.
 *
 * @note The implementation of the protocol does not allocate memory for any operation.
 * Message parser acts on an instance of the user provided Solace::ByteReader and any message data such as
 * name string or data read from a file is actually a pointer to the underlying ReadBuffer storage.
 * Thus it is user's responsibility to manage lifetime of that buffer.
 * (That is not technically correct as current implementation does allocate memory when dealing with Solace::Path
 * objects as there is no currently availiable PathView version)
 *
 * In order to create 9P2000 messages please @see RequestWriter.
 */
struct RequestParser :
		public ParserBase {

	/**
	 * Construct a new instance of the parser.
	 * @param maxPayloadSize Maximum message paylaod size in bytes.
	 * @param nameMapper A pointer to a map-function to convert message op-codes to message name string.
	 * @param parserTable A table of version specific opcode parser methods.
	 */
	RequestParser(size_type maxPayloadSize, VersionedNameMapper	nameMapper, RequestParseTable parserTable) noexcept
		: ParserBase{maxPayloadSize, nameMapper}
		, _versionedRequestParser{Solace::mv(parserTable)}
	{}

	/**
	 * Parse 9P Request type message from a byte buffer.
	 * This is the primiry method used by a server implementation to parse requests from a client.

	 * @param header Message header.
	 * @param data Byte buffer to read message content from.
	 * @return Resulting message if parsed successfully or an error otherwise.
	 */
	Solace::Result<RequestMessage, Error>
	parseRequest(MessageHeader const& header, Solace::ByteReader& data) const;

private:

	RequestParseTable	_versionedRequestParser;   /// Parser V-table.
};



/**
 * Create a new parser for a given protocol version and max message size.
 * @param version Version of the protocol to use.
 * @param maxPayloadSize Maximum size of a message in bytes as negotioated with Request::Version
 * @return Either a valid parser or an error if invalid version of maxMessageSize are requested.
 */
Solace::Result<ResponseParser, Error>
createResponseParser(Solace::StringView version, size_type maxPayloadSize) noexcept;


/**
 * Create a new request message parser for a given protocol version.
 * @param version Version of the protocol to use.
 * @param maxPayloadSize Maximum size of a message in bytes as negotioated with Request::Version
 * @return Either a valid parser or an error if invalid version of maxMessageSize are requested.
 */
Solace::Result<RequestParser, Error>
createRequestParser(Solace::StringView version, size_type maxPayloadSize) noexcept;


// Type constraints
static_assert(std::is_move_assignable_v<UnversionedParser>,		"UnversionedParser should be movable");
static_assert(std::is_move_assignable_v<ParserBase>,			"ParserBase should be movable");
static_assert(std::is_move_assignable_v<ResponseParser>,		"ResponseParser should be movable");
static_assert(std::is_move_assignable_v<RequestParser>,			"RequestParser should be movable");

static_assert(std::is_move_assignable_v<RequestMessage>,		"RequestMessage should be movable");
static_assert(std::is_move_assignable_v<ResponseMessage>,		"ResponseMessage should be movable");


}  // end of namespace styxe
#endif  // STYXE_MESSAGEPARSER_HPP
