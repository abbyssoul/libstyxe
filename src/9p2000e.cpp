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

#include "styxe/9p2000e.hpp"

#include "parse_helper.hpp"
#include "write_helper.hpp"


using namespace Solace;
using namespace styxe;


const StringLiteral _9P2000E::kProtocolVersion{"9P2000.e"};


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Request::Session& dest) {
	return decode(data,
				dest.key[0],
				dest.key[1],
				dest.key[2],
				dest.key[3],
				dest.key[4],
				dest.key[5],
				dest.key[6],
				dest.key[7]);
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Request::ShortRead& dest) {
	return decode(data, dest.fid, dest.path);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Request::ShortWrite& dest) {
	return decode(data, dest.fid, dest.path, dest.data);
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Response::Session& ) {
	return Result<ByteReader&, Error>{types::okTag, data};
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Response::ShortRead& dest) {
	return decode(data, dest.data);
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Response::ShortWrite& dest) {
	return decode(data, dest.count);
}


ResponseWriter&
styxe::operator<< (ResponseWriter& writer, _9P2000E::Response::Session const& message) {
	return encode(writer, message);
}

ResponseWriter&
styxe::operator<< (ResponseWriter& writer, _9P2000E::Response::ShortRead const& message) {
	return encode(writer, message, message.data);
}


ResponseWriter&
styxe::operator<< (ResponseWriter& writer, _9P2000E::Response::ShortWrite const& message) {
	return encode(writer, message, message.count);
}


RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000E::Request::Session const& message) {
	return encode(writer,
				  message,
				  message.key[0],
				  message.key[1],
				  message.key[2],
				  message.key[3],
				  message.key[4],
				  message.key[5],
				  message.key[6],
				  message.key[7]);
}

RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000E::Request::ShortRead const& message) {
	return encode(writer, message, message.fid, message.path);
}

RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000E::Request::ShortWrite const& message) {
	return encode(writer, message, message.fid, message.path, message.data);
}


StringView
styxe::_9P2000E::messageTypeToString(byte type) noexcept {
	auto mType = static_cast<_9P2000E::MessageType>(type);
	switch (mType) {
	case _9P2000E::MessageType::TSession:		return "TSession";
	case _9P2000E::MessageType::RSession:		return "RSession";
	case _9P2000E::MessageType::TShortRead:		return "TShortRead";
	case _9P2000E::MessageType::RShortRead:		return "RShortRead";
	case _9P2000E::MessageType::TShortWrite:	return "TShortWrite";
	case _9P2000E::MessageType::RShortWrite:	return "RShortWrite";
	}

	return styxe::messageTypeToString(type);
}

