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

#include "styxe/9p2000u.hpp"

#include "parse_helper.hpp"


using namespace Solace;
using namespace styxe;


const StringLiteral _9P2000U::kProtocolVersion{"9P2000.u"};


RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000U::Request::Auth const& message) {
	writer.messageType(messageCode(message))
			<< message.afid
			<< message.uname
			<< message.aname
			<< message.n_uname;
	writer.updateMessageSize();

	return writer;
}


RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000U::Request::Attach const& message) {
	writer.messageType(messageCode(message))
			<< message.fid
			<< message.afid
			<< message.uname
			<< message.aname
			<< message.n_uname;
	writer.updateMessageSize();

	return writer;
}


RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000U::Request::Create const& message) {
	writer.messageType(messageCode(message))
			<< message.fid
			<< message.name
			<< message.perm
			<< message.mode.mode
			<< message.extension;
	writer.updateMessageSize();

	return writer;
}

RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000U::Request::WStat const& message) {
	writer.messageType(messageCode(message))
			<< message.fid
			<< message.stat;
	writer.updateMessageSize();

	return writer;
}


ResponseWriter&
styxe::operator<< (ResponseWriter& writer, _9P2000U::Response::Error const& message) {
	writer.messageType(messageCode(message))
			<< message.ename
			<< message.errcode;
	writer.updateMessageSize();

	return writer;
}


ResponseWriter&
styxe::operator<< (ResponseWriter& writer, _9P2000U::Response::Stat const& message) {
	writer.messageType(messageCode(message))
			<< message.dummySize
			<< message.data;
	writer.updateMessageSize();

	return writer;
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000U::Request::Auth& dest) {
	return (data >> static_cast<Request::Auth&>(dest))
			.then([&](ByteReader& reader) {
				return decode(reader, dest.n_uname);
			});
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000U::Request::Attach& dest) {
	return (data >> static_cast<Request::Attach&>(dest))
			.then([&](ByteReader& reader) {
				return decode(reader, dest.n_uname);
			});
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000U::Request::Create& dest) {
	return (data >> static_cast<Request::Create&>(dest))
			.then([&](ByteReader& reader) {
				return decode(reader, dest.extension);
			});
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000U::Request::WStat& dest) {
	return decode(data, dest.fid, dest.stat);
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000U::Response::Error& dest) {
	return (data >> static_cast<Response::Error&>(dest))
			.then([&](ByteReader& reader) {
				return decode(reader, dest.errcode);
			});
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000U::Response::Stat& dest) {
	return decode(data, dest.dummySize, dest.data);
}

