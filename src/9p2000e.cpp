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
#include "styxe/decoder.hpp"


using namespace Solace;
using namespace styxe;


const StringLiteral _9P2000E::kProtocolVersion{"9P2000.e"};


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Request::Session& dest) {
	Decoder decoder{data};
	auto result = decoder
			>> dest.key[0]
			>> dest.key[1]
			>> dest.key[2]
			>> dest.key[3]
			>> dest.key[4]
			>> dest.key[5]
			>> dest.key[6]
			>> dest.key[7];

	if (!result)
		return result.moveError();

	return Result<ByteReader&, Error>{types::okTag, data};
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Request::ShortRead& dest) {
	Decoder decoder{data};
	auto result = decoder >> dest.fid
						  >> dest.path;
	if (!result)
		return result.moveError();

	return Result<ByteReader&, Error>{types::okTag, data};

}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Request::ShortWrite& dest) {
	Decoder decoder{data};
	auto result = decoder >> dest.fid
						  >> dest.path
						  >> dest.data;
	if (!result)
		return result.moveError();

	return Result<ByteReader&, Error>{types::okTag, data};
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Response::Session& ) {
	return Result<ByteReader&, Error>{types::okTag, data};
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Response::ShortRead& dest) {
	Decoder decoder{data};
	auto result = decoder >> dest.data;

	if (!result) return result.moveError();
	return Result<ByteReader&, Error>{types::okTag, data};
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000E::Response::ShortWrite& dest) {
	Decoder decoder{data};
	auto result = decoder >> dest.count;

	if (!result) return result.moveError();
	return Result<ByteReader&, Error>{types::okTag, data};
}


ResponseWriter&
styxe::operator<< (ResponseWriter& writer, _9P2000E::Response::Session const& response) {
	writer.messageType(messageCode(response));
	return writer;
}

ResponseWriter&
styxe::operator<< (ResponseWriter& writer, _9P2000E::Response::ShortRead const& response) {
	writer.messageType(messageCode(response))
			<< response.data;

	return writer;
}


ResponseWriter&
styxe::operator<< (ResponseWriter& writer, _9P2000E::Response::ShortWrite const& response) {
	writer.messageType(messageCode(response))
			<< response.count;

	return writer;
}


RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000E::Request::Session const& response) {
	writer.messageType(messageCode(response))
			<< response.key[0]
			<< response.key[1]
			<< response.key[2]
			<< response.key[3]
			<< response.key[4]
			<< response.key[5]
			<< response.key[6]
			<< response.key[7];

	return writer;
}

RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000E::Request::ShortRead const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.path;

	return writer;
}

RequestWriter&
styxe::operator<< (RequestWriter& writer, _9P2000E::Request::ShortWrite const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.path
			<< request.data;

	return writer;
}



