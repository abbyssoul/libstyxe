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

#include "styxe/9p2000.hpp"
#include "styxe/encoder.hpp"


using namespace Solace;
using namespace styxe;


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Version const& response) {
	writer.messageType(messageCode(response), kNoTag)
			<< response.msize
			<< response.version;

	return writer;
}

MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Auth const& response) {
	writer.messageType(messageCode(response))
			<< response.qid;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Error const& response) {
	writer.messageType(messageCode(response))
			<< response.ename;

	return writer;
}

MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Flush const& response) {
	writer.messageType(messageCode(response));
	return writer;
}

MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Attach const& response) {
	writer.messageType(messageCode(response))
			<< response.qid;

	return writer;
}

MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Walk const& response) {
	auto& e = writer.messageType(messageCode(response));
	e << response.nqids;

	for (size_t i = 0; i < response.nqids; ++i) {
			e << response.qids[i];
	}

	return writer;
}

MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Open const& response) {
	writer.messageType(messageCode(response))
			<< response.qid
			<< response.iounit;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Create const& response) {
	writer.messageType(messageCode(response))
			<< response.qid
			<< response.iounit;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Read const& response) {
	writer.messageType(messageCode(response))
			<< response.data;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Write const& response) {
	writer.messageType(messageCode(response))
			<< response.count;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Clunk const& response) {
	writer.messageType(messageCode(response));
	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Remove const& response) {
	writer.messageType(messageCode(response));
	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::Stat const& response) {
	writer.messageType(messageCode(response))
			<< response.dummySize
			<< response.data;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Response::WStat const& response) {
	writer.messageType(messageCode(response));
	return writer;
}



MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Version const& request) {
	writer.messageType(messageCode(request))
			<< request.msize
			<< request.version;
	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Auth const& request) {
	writer.messageType(messageCode(request))
			<< request.afid
			<< request.uname
			<< request.aname;

	return writer;
}

MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Flush const& request) {
	writer.messageType(messageCode(request))
			<< request.oldtag;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Attach const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.afid
			<< request.uname
			<< request.aname;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Walk const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.newfid
			<< request.path;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Open const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.mode.mode;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Create const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.name
			<< request.perm
			<< request.mode.mode;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Read const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.offset
			<< request.count;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Write const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.offset
			<< request.data;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Clunk const& request) {
	writer.messageType(messageCode(request))
			<< request.fid;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Remove const& request) {
	writer.messageType(messageCode(request))
			<< request.fid;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::Stat const& request) {
	writer.messageType(messageCode(request))
			<< request.fid;

	return writer;
}


MessageWriter&
styxe::operator<< (MessageWriter& writer, Request::WStat const& request) {
	writer.messageType(messageCode(request))
			<< request.fid
			<< request.stat;

	return writer;
}




size_type
styxe::protocolSize(Qid const&) noexcept {
	static constexpr size_type kQidSize = sizeof(Qid::type) + sizeof(Qid::version) + sizeof(Qid::path);

	// Qid has a fixed size of 13 bytes, lets keep it that way
	static_assert(kQidSize == 13, "Incorrect Qid struct size");

	return kQidSize;
}


size_type
styxe::protocolSize(styxe::Stat const& stat) noexcept {
	return  protocolSize(stat.size) +
			protocolSize(stat.type) +
			protocolSize(stat.dev) +
			protocolSize(stat.qid) +
			protocolSize(stat.mode) +
			protocolSize(stat.atime) +
			protocolSize(stat.mtime) +
			protocolSize(stat.length) +
			protocolSize(stat.name) +
			protocolSize(stat.uid) +
			protocolSize(stat.gid) +
			protocolSize(stat.muid);
}



var_datum_size_type
DirListingWriter::sizeStat(Stat const& stat) {
   return narrow_cast<var_datum_size_type>(::protocolSize(stat) - sizeof(stat.size));
}


bool DirListingWriter::encode(Stat const& stat) {
	auto const protoSize = ::protocolSize(stat);
    // Keep count of how many data we have traversed.
    _bytesTraversed += protoSize;
	if (_bytesTraversed <= _offset) {  // Client is only interested in data pass the offset.
        return true;
    }

    // Keep track of much data will end up in a buffer to prevent overflow.
    _bytesEncoded += protoSize;
	if (_bytesEncoded > _maxBytes) {
        return false;
    }

    // Only encode the data if we have some room left, as specified by 'count' arg.
	_encoder << stat;

    return true;
}

