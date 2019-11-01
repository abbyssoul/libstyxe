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
							_9P2000E::Request::ShortWrite
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
							_9P2000E::Response::ShortWrite
							>;

using RequestParseFunc = Solace::Result<RequestMessage, Error> (*)(Solace::ByteReader& );
using ResponseParseFunc = Solace::Result<ResponseMessage, Error> (*)(Solace::ByteReader& );

static_assert (sizeof(MessageHeader::type) == 1, "MessageType is expected to be 1 byte");
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
	 * @param buffer Byte buffer to read message header from.
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

	size_type const         maxPayloadSize;		/// Initial value of the maximum message payload size in bytes.
};



struct ParserBase {
	using VersionedNameMapper = Solace::StringView	(*)(Solace::byte);

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
	 * @return
	 */
	size_type maxMessageSize() const noexcept { return headerSize() + _maxPayloadSize; }

private:
	size_type const         _maxPayloadSize;		/// Initial value of the maximum message payload size in bytes.
	VersionedNameMapper		_nameMapper;

};

/**
 * An implementation of 9p message parser.
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
 * An implementation of 9p message parser.
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


}  // end of namespace styxe
#endif  // STYXE_MESSAGEPARSER_HPP
