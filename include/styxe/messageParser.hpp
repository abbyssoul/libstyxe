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
#include "styxe/9p2000l.hpp"


namespace styxe {

/// Type representing request message
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
							_9P2000E::Request::Session,
							_9P2000E::Request::ShortRead,
							_9P2000E::Request::ShortWrite
							>;

/// Type representing response message
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
							_9P2000E::Response::Session,
							_9P2000E::Response::ShortRead,
							_9P2000E::Response::ShortWrite
							>;

using RequestParseFunc = Solace::Result<RequestMessage, Error> (*)(Solace::ByteReader& );
using ResponseParseFunc = Solace::Result<ResponseMessage, Error> (*)(Solace::ByteReader& );

static_assert (sizeof(MessageHeader::type) == 1, "MessageType is expected to be 1 byte");
using RequestParseTable = std::array<RequestParseFunc, 1 << 8*sizeof(MessageHeader::type)>;
using ResponseParseTable = std::array<ResponseParseFunc, 1 << 8*sizeof(MessageHeader::type)>;


struct UnversionedParser {

	/**
	 * Parse 9P message header from a byte buffer.
	 * @param buffer Byte buffer to read message header from.
	 * @return Resulting message header if parsed successfully or an error otherwise.
	 */
	Solace::Result<MessageHeader, Error>
	parseMessageHeader(Solace::ByteReader& byteStream) const;

	Solace::Result<Request::Version, Error>
	parseMessage(MessageHeader header, Solace::ByteReader& byteStream) const;

	size_type maxMessageSize() const noexcept { return headerSize() + maxPayloadSize; }

	size_type const         maxPayloadSize;                /// Initial value of the maximum message size in bytes.
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
struct Parser :
		public UnversionedParser {

	Parser(Parser const& ) = delete;
	Parser& operator= (Parser const& ) = delete;

	/**
	 * Construct a new instance of the protocol.
	 * Usually one would create an instance per connection as protocol stores state per estanblished session.
	 * @param maxMassageSize Maximum message size in bytes. This will be used by during version and size negotiation.
	 * @param version Supported protocol version. This is advertized by the protocol during version/size negotiation.
	 */
	Parser(size_type maxMassageSize,
		   RequestParseTable	versionedRequestParser,
		   ResponseParseTable	versionedResponseParser) noexcept
		: UnversionedParser{maxMassageSize}
		, _versionedRequestParser{Solace::mv(versionedRequestParser)}
		, _versionedResponseParser{Solace::mv(versionedResponseParser)}
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

	/**
	 * Parse 9P Request type message from a byte buffer.
	 * This is the primiry method used by a server implementation to parse requests from a client.

	 * @param header Message header.
	 * @param data Byte buffer to read message content from.
	 * @return Resulting message if parsed successfully or an error otherwise.
	 */
	Solace::Result<RequestMessage, Error>
	parseRequest(MessageHeader const& header, Solace::ByteReader& data) const;

	Solace::StringView messageName(Solace::byte messageType) const;

private:
	RequestParseTable	_versionedRequestParser;  /// Parser V-table.
	ResponseParseTable	_versionedResponseParser;  /// Parser V-table.
};


/**
 * Create a new parser for a given protocol version and max message size.
 * @param maxMessageSize Maximum size of a message in bytes as negotioated with Request::Version
 * @param version Version of the protocol to use.
 * @return Either a valid parser or an error if invalid version of maxMessageSize are requested.
 */
Solace::Result<Parser, Error>
createParser(size_type maxMessageSize, Solace::StringView version) noexcept;


}  // end of namespace styxe
#endif  // STYXE_MESSAGEPARSER_HPP
