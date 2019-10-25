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

#include "styxe/messageParser.hpp"
#include "styxe/decoder.hpp"

using namespace Solace;
using namespace styxe;


const size_type         styxe::kMaxMesssageSize = 8*1024;      // 8k should be enough for everyone, am I right?



Result<RequestMessage, Error>
invalidRequestType(ByteReader& ) {
	return Result<RequestMessage, Error>{types::errTag, getCannedError(CannedError::UnsupportedMessageType) };
}

Result<ResponseMessage, Error>
invalidResponseType(ByteReader& ) {
	return Result<ResponseMessage, Error>{types::errTag, getCannedError(CannedError::UnsupportedMessageType) };
}


template<typename T>
Result<RequestMessage, Error>
parseRequest(ByteReader& data) {
	T msg{};
	auto result = data >> msg;
	if (!result) {
		return result.moveError();
	}

	return Result<RequestMessage, Error>{types::okTag, std::move(msg)};
}

template<typename T>
Result<ResponseMessage, Error>
parseResponse(ByteReader& data) {
	T msg{};
	auto result = data >> msg;
	if (!result) {
		return result.moveError();
	}

	return Result<ResponseMessage, Error>{types::okTag, std::move(msg)};
}

/*
#define DEFINE_PARSER(type, message) \
	Result<type##Message, Error> \
	parse##message##type(ByteReader& data)	{ return parse##type<type::message>(data); }


DEFINE_PARSER(Request, Version)
DEFINE_PARSER(Request, Auth)
DEFINE_PARSER(Request, Attach)
DEFINE_PARSER(Request, Flush)
DEFINE_PARSER(Request, Walk)
DEFINE_PARSER(Request, Open)
DEFINE_PARSER(Request, Create)
DEFINE_PARSER(Request, Read)
DEFINE_PARSER(Request, Write)
DEFINE_PARSER(Request, Clunk)
DEFINE_PARSER(Request, Remove)
DEFINE_PARSER(Request, Stat)
DEFINE_PARSER(Request, WStat)


DEFINE_PARSER(Response, Version)
DEFINE_PARSER(Response, Error)
DEFINE_PARSER(Response, Auth)
DEFINE_PARSER(Response, Attach)
DEFINE_PARSER(Response, Flush)
DEFINE_PARSER(Response, Walk)
DEFINE_PARSER(Response, Open)
DEFINE_PARSER(Response, Create)
DEFINE_PARSER(Response, Read)
DEFINE_PARSER(Response, Write)
DEFINE_PARSER(Response, Clunk)
DEFINE_PARSER(Response, Remove)
DEFINE_PARSER(Response, Stat)
DEFINE_PARSER(Response, WStat)
*/

ResponseParseTable getBlankResponseParserTable() {
	ResponseParseTable table;
	for (auto& i : table) {
		i = invalidResponseType;
	}

	return table;
}

RequestParseTable getBlankRequestParserTable() {
	RequestParseTable table;
	for (auto& i : table) {
		i = invalidRequestType;
	}

	return table;
}



namespace _9P2000 {

RequestParseTable getRequestParserTable() {
	RequestParseTable table{getBlankRequestParserTable()};

#define FILL_REQUEST(message) \
	table[static_cast<byte>(MessageType::T##message)] = parseRequest<Request::message>


	FILL_REQUEST(Version);
	FILL_REQUEST(Auth);
	FILL_REQUEST(Attach);
	FILL_REQUEST(Flush);
	FILL_REQUEST(Walk);
	FILL_REQUEST(Open);
	FILL_REQUEST(Create);
	FILL_REQUEST(Read);
	FILL_REQUEST(Write);
	FILL_REQUEST(Clunk);
	FILL_REQUEST(Remove);
	FILL_REQUEST(Stat);
	FILL_REQUEST(WStat);
#undef FILL_RESPONSE

	return table;
}

ResponseParseTable getResponseParserTable() {
	ResponseParseTable table{getBlankResponseParserTable()};

#define FILL_RESPONSE(message) \
	table[static_cast<byte>(MessageType::R##message)] = parseResponse<Response::message>

	FILL_RESPONSE(Version);
	FILL_RESPONSE(Auth);
	FILL_RESPONSE(Attach);
	FILL_RESPONSE(Error);
	FILL_RESPONSE(Flush);
	FILL_RESPONSE(Walk);
	FILL_RESPONSE(Open);
	FILL_RESPONSE(Create);
	FILL_RESPONSE(Read);
	FILL_RESPONSE(Write);
	FILL_RESPONSE(Clunk);
	FILL_RESPONSE(Remove);
	FILL_RESPONSE(Stat);
	FILL_RESPONSE(WStat);
#undef FILL_RESPONSE

	return table;
}

}  // namespace _9P2000


namespace styxe::_9P2000E {
/*
DEFINE_PARSER(Request, Session)
DEFINE_PARSER(Request, ShortRead)
DEFINE_PARSER(Request, ShortWrite)

DEFINE_PARSER(Response, Session)
DEFINE_PARSER(Response, ShortRead)
DEFINE_PARSER(Response, ShortWrite)
*/

RequestParseTable getRequestParserTable() {
	auto table = ::_9P2000::getRequestParserTable();

	table[static_cast<byte>(MessageType::TSession)] = parseRequest<Request::Session>;
	table[static_cast<byte>(MessageType::TShortRead)] = parseRequest<Request::ShortRead>;
	table[static_cast<byte>(MessageType::TShortWrite)] = parseRequest<Request::ShortWrite>;

	return table;
}

ResponseParseTable getResponseParserTable() {
	auto table = ::_9P2000::getResponseParserTable();

	table[static_cast<byte>(MessageType::RSession)] = parseResponse<Response::Session>;
	table[static_cast<byte>(MessageType::RShortRead)] = parseResponse<Response::ShortRead>;
	table[static_cast<byte>(MessageType::RShortWrite)] = parseResponse<Response::ShortWrite>;

	return table;
}

}  // namespace styxe::_9P2000E





Solace::Result<void, Error>
validateHeader(MessageHeader header, ByteReader::size_type dataAvailible, size_type maxMessageSize) {
	// Message data sanity check
	// Just paranoid about huge messages exciding frame size getting through.
	if (header.messageSize > maxMessageSize) {
		return getCannedError(CannedError::IllFormedHeader_TooBig);
	}

	auto const expectedData = header.payloadSize();

	// Make sure we have been given enough data to read a message as requested in the message size.
	if (expectedData > dataAvailible) {
		return getCannedError(CannedError::NotEnoughData);
	}

	// Make sure there is no extra data in the buffer.
	if (expectedData < dataAvailible) {
		return getCannedError(CannedError::MoreThenExpectedData);
	}


	return Ok();
}


Result<MessageHeader, Error>
UnversionedParser::parseMessageHeader(ByteReader& src) const {
	auto const mandatoryHeaderSize = headerSize();

	Decoder decoder{src};
	MessageHeader header;

	auto result = decoder >> header.messageSize
						  >> header.type  // Read message type
						  >> header.tag;  // Tags are provided by aclient and can not be validated at this stage.

	if (!result) {
		return result.moveError();  // getCannedError(CannedError::IllFormedHeader);
	}

	// Sanity checks:
	if (header.messageSize < mandatoryHeaderSize) {
		return getCannedError(CannedError::IllFormedHeader_FrameTooShort);
	}

	if (header.messageSize > maxMessageSize()) {
		return getCannedError(CannedError::IllFormedHeader_FrameTooShort);
	}

	return Ok(header);
}


Result<Request::Version, Error>
UnversionedParser::parseMessage(MessageHeader header, ByteReader& data) const {
	auto isValid = validateHeader(header, data.remaining(), maxMessageSize());
	if (!isValid)
		return isValid.getError();

	if (header.type != static_cast<byte>(MessageType::TVersion))
		return Result<Request::Version, Error>{types::errTag, getCannedError(CannedError::UnsupportedMessageType) };

	Request::Version version;
	auto result = data >> version;
	if (!result)
		return result.moveError();

	return Result<Request::Version, Error>{types::okTag, std::move(version)};
}


Result<Parser, Error>
styxe::createParser(size_type maxMessageSize, Solace::StringView version) noexcept {

	if (version == kProtocolVersion) {
		return Result<Parser, Error>{types::okTag, in_place,
					maxMessageSize, _9P2000::getRequestParserTable(), _9P2000::getResponseParserTable() };
	}  else if (version == _9P2000E::kProtocolVersion) {
		return Result<Parser, Error>{types::okTag, in_place,
					maxMessageSize, _9P2000E::getRequestParserTable(), _9P2000E::getResponseParserTable() };
	}

	return Result<Parser, Error>{types::errTag, getCannedError(CannedError::UnsupportedVersion) };
}


Result<ResponseMessage, Error>
Parser::parseResponse(MessageHeader const& header, ByteReader& data) const {
	auto isValid = validateHeader(header, data.remaining(), maxMessageSize());
	if (!isValid)
		return isValid.getError();

	auto& decoder = _versionedResponseParser[header.type];
	return decoder(data);
}


Result<RequestMessage, Error>
Parser::parseRequest(MessageHeader const& header, ByteReader& data) const {
	auto isValid = validateHeader(header, data.remaining(), maxMessageSize());
	if (!isValid)
		return isValid.getError();

	auto& decoder = _versionedRequestParser[header.type];
	return decoder(data);
}
