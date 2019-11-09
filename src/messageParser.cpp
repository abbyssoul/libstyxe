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


const size_type         styxe::kMaxMessageSize = 8*1024;      // 8k should be enough for everyone, am I right?



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



namespace styxe::_9P2000 {

RequestParseTable getRequestParserTable() {
	RequestParseTable table{getBlankRequestParserTable()};

#define FILL_REQUEST(message) \
	table[asByte(MessageType::T##message)] = parseRequest<Request::message>


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
	table[asByte(MessageType::R##message)] = parseResponse<Response::message>

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

}  // namespace styxe::_9P2000



namespace styxe::_9P2000U {

RequestParseTable getRequestParserTable() {
	auto table = ::_9P2000::getRequestParserTable();


	table[asByte(::styxe::MessageType::TAuth)] = parseRequest<_9P2000U::Request::Auth>;
	table[asByte(::styxe::MessageType::TAttach)] = parseRequest<_9P2000U::Request::Attach>;
	table[asByte(::styxe::MessageType::TCreate)] = parseRequest<_9P2000U::Request::Create>;
	table[asByte(::styxe::MessageType::TWStat)] = parseRequest<_9P2000U::Request::WStat>;

	return table;
}

ResponseParseTable getResponseParserTable() {
	auto table = ::_9P2000::getResponseParserTable();

	table[asByte(::styxe::MessageType::RError)] = parseResponse<_9P2000U::Response::Error>;
	table[asByte(::styxe::MessageType::RStat)] = parseResponse<_9P2000U::Response::Stat>;

	return table;
}

}  // namespace styxe::_9P2000U


//----------------------------------------------------------------------------------------------------------------------
// 9p2000.e methods
//----------------------------------------------------------------------------------------------------------------------
namespace styxe::_9P2000E {

RequestParseTable getRequestParserTable() {
	auto table = ::_9P2000::getRequestParserTable();

	table[asByte(MessageType::TSession)] = parseRequest<Request::Session>;
	table[asByte(MessageType::TShortRead)] = parseRequest<Request::ShortRead>;
	table[asByte(MessageType::TShortWrite)] = parseRequest<Request::ShortWrite>;

	return table;
}

ResponseParseTable getResponseParserTable() {
	auto table = ::_9P2000::getResponseParserTable();

	table[asByte(MessageType::RSession)] = parseResponse<Response::Session>;
	table[asByte(MessageType::RShortRead)] = parseResponse<Response::ShortRead>;
	table[asByte(MessageType::RShortWrite)] = parseResponse<Response::ShortWrite>;

	return table;
}

}  // namespace styxe::_9P2000E

//----------------------------------------------------------------------------------------------------------------------
// 9p2000.L methods
//----------------------------------------------------------------------------------------------------------------------
namespace styxe::_9P2000L {

RequestParseTable getRequestParserTable() {
	auto table = _9P2000U::getRequestParserTable();

table[asByte(MessageType::Tstatfs)] = parseRequest<Request::StatFS>;
table[asByte(MessageType::Tlopen)] = parseRequest<Request::Open>;
table[asByte(MessageType::Tlcreate)] = parseRequest<Request::Create>;
table[asByte(MessageType::Tsymlink)] = parseRequest<Request::Symlink>;
table[asByte(MessageType::Tmknod)] = parseRequest<Request::MkNode>;
table[asByte(MessageType::Trename)] = parseRequest<Request::Rename>;
table[asByte(MessageType::Treadlink)] = parseRequest<Request::ReadLink>;
table[asByte(MessageType::Tgetattr)] = parseRequest<Request::GetAttr>;
table[asByte(MessageType::Tsetattr)] = parseRequest<Request::SetAttr>;
table[asByte(MessageType::Txattrwalk)] = parseRequest<Request::XAttrWalk>;
table[asByte(MessageType::Txattrcreate)] = parseRequest<Request::XAttrCreate>;
table[asByte(MessageType::Treaddir)] = parseRequest<Request::ReadDir>;
table[asByte(MessageType::Tfsync)] = parseRequest<Request::FSync>;
table[asByte(MessageType::Tlock)] = parseRequest<Request::Lock>;
table[asByte(MessageType::Tgetlock)] = parseRequest<Request::GetLock>;
table[asByte(MessageType::Tlink)] = parseRequest<Request::Link>;
table[asByte(MessageType::Tmkdir)] = parseRequest<Request::MkDir>;
table[asByte(MessageType::Trenameat)] = parseRequest<Request::RenameAt>;
table[asByte(MessageType::Tunlinkat)] = parseRequest<Request::UnlinkAt>;


	return table;
}

ResponseParseTable getResponseParserTable() {
	auto table = _9P2000U::getResponseParserTable();

	table[asByte(MessageType::Rlerror)] = parseResponse<Response::LError>;
	table[asByte(MessageType::Rstatfs)] = parseResponse<Response::StatFS>;
	table[asByte(MessageType::Rlopen)] = parseResponse<Response::Open>;
	table[asByte(MessageType::Rlcreate)] = parseResponse<Response::Create>;
	table[asByte(MessageType::Rsymlink)] = parseResponse<Response::Symlink>;
	table[asByte(MessageType::Rmknod)] = parseResponse<Response::MkNode>;
	table[asByte(MessageType::Rrename)] = parseResponse<Response::Rename>;
	table[asByte(MessageType::Rreadlink)] = parseResponse<Response::ReadLink>;
	table[asByte(MessageType::Rgetattr)] = parseResponse<Response::GetAttr>;
	table[asByte(MessageType::Rsetattr)] = parseResponse<Response::SetAttr>;
	table[asByte(MessageType::Rxattrwalk)] = parseResponse<Response::XAttrWalk>;
	table[asByte(MessageType::Rxattrcreate)] = parseResponse<Response::XAttrCreate>;
	table[asByte(MessageType::Rreaddir)] = parseResponse<Response::ReadDir>;
	table[asByte(MessageType::Rfsync)] = parseResponse<Response::FSync>;
	table[asByte(MessageType::Rlock)] = parseResponse<Response::Lock>;
	table[asByte(MessageType::Rgetlock)] = parseResponse<Response::GetLock>;
	table[asByte(MessageType::Rlink)] = parseResponse<Response::Link>;
	table[asByte(MessageType::Rmkdir)] = parseResponse<Response::MkDir>;
	table[asByte(MessageType::Rrenameat)] = parseResponse<Response::RenameAt>;
	table[asByte(MessageType::Runlinkat)] = parseResponse<Response::UnlinkAt>;

	return table;
}

}  // namespace styxe::_9P2000L


//----------------------------------------------------------------------------------------------------------------------
// Parser implementation
//----------------------------------------------------------------------------------------------------------------------

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
UnversionedParser::parseVersionRequest(MessageHeader header, ByteReader& data) const {
	auto isValid = validateHeader(header, data.remaining(), maxMessageSize());
	if (!isValid)
		return isValid.moveError();

	if (header.type != asByte(MessageType::TVersion))
		return Result<Request::Version, Error>{types::errTag, getCannedError(CannedError::UnsupportedMessageType) };

	Request::Version version;
	auto result = data >> version;
	if (!result)
		return result.moveError();

	return Result<Request::Version, Error>{types::okTag, std::move(version)};
}



Result<ResponseMessage, Error>
ResponseParser::parseResponse(MessageHeader const& header, ByteReader& data) const {
	auto isValid = validateHeader(header, data.remaining(), maxMessageSize());
	if (!isValid)
		return isValid.moveError();

	auto& decoder = _versionedResponseParser[header.type];
	return decoder(data);
}


Result<RequestMessage, Error>
RequestParser::parseRequest(MessageHeader const& header, ByteReader& data) const {
	auto isValid = validateHeader(header, data.remaining(), maxMessageSize());
	if (!isValid)
		return isValid.moveError();

	auto& decoder = _versionedRequestParser[header.type];
	return decoder(data);
}


StringView
ParserBase::messageName(byte messageType) const {
	return _nameMapper(messageType);
}




Result<ResponseParser, Error>
styxe::createResponseParser(Solace::StringView version, size_type maxPayloadSize) noexcept {
	if (version == kProtocolVersion) {
		return Result<ResponseParser, Error>{types::okTag, in_place,
					maxPayloadSize,
					messageTypeToString,
					_9P2000::getResponseParserTable() };
	} else if (version == _9P2000U::kProtocolVersion) {
		return Result<ResponseParser, Error>{types::okTag, in_place,
					maxPayloadSize,
					_9P2000U::messageTypeToString,
					_9P2000U::getResponseParserTable() };
	} else if (version == _9P2000E::kProtocolVersion) {
		return Result<ResponseParser, Error>{types::okTag, in_place,
					maxPayloadSize,
					_9P2000E::messageTypeToString,
					_9P2000E::getResponseParserTable() };
	} else if (version == _9P2000L::kProtocolVersion) {
		return Result<ResponseParser, Error>{types::okTag, in_place,
					maxPayloadSize,
					_9P2000L::messageTypeToString,
					_9P2000L::getResponseParserTable() };
	}

	return Result<ResponseParser, Error>{types::errTag, getCannedError(CannedError::UnsupportedProtocolVersion) };
}

Result<RequestParser, Error>
styxe::createRequestParser(StringView version, size_type maxPayloadSize) noexcept {
	if (version == kProtocolVersion) {
		return Result<RequestParser, Error>{types::okTag, in_place,
					maxPayloadSize,
					messageTypeToString,
					_9P2000::getRequestParserTable()};
	} else if (version == _9P2000U::kProtocolVersion) {
		return Result<RequestParser, Error>{types::okTag, in_place,
					maxPayloadSize,
					_9P2000U::messageTypeToString,
					_9P2000U::getRequestParserTable()};
	} else if (version == _9P2000E::kProtocolVersion) {
		return Result<RequestParser, Error>{types::okTag, in_place,
					maxPayloadSize,
					_9P2000E::messageTypeToString,
					_9P2000E::getRequestParserTable()};
	} else if (version == _9P2000L::kProtocolVersion) {
		return Result<RequestParser, Error>{types::okTag, in_place,
					maxPayloadSize,
					_9P2000L::messageTypeToString,
					_9P2000L::getRequestParserTable()};
	}

	return Result<RequestParser, Error>{types::errTag, getCannedError(CannedError::UnsupportedProtocolVersion) };
}
