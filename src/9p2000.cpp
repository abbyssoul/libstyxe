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
#include "styxe/messageParser.hpp"


#include <solace/assert.hpp>
#include <algorithm>  // std::min


using namespace Solace;
using namespace styxe;


const StringLiteral     styxe::kUnknownProtocolVersion{"unknown"};
const StringLiteral		styxe::kProtocolVersion{"9P2000"};

const Tag               styxe::kNoTag = static_cast<Tag>(~0);
const Fid               styxe::kNoFID = static_cast<Fid>(~0);


const byte OpenMode::READ;
const byte OpenMode::WRITE;
const byte OpenMode::RDWR;
const byte OpenMode::EXEC;
const byte OpenMode::TRUNC;
const byte OpenMode::CEXEC;
const byte OpenMode::RCLOSE;


template<typename...Args>
Result<ByteReader&, Error>
decode(ByteReader& data, Args&& ...args) {
	Decoder decoder{data};
	auto result = (decoder >> ... >> args);
	if (!result) return result.moveError();

	return Result<ByteReader&, Error>{types::okTag, data};
}



Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Walk& dest) {
	Decoder decoder{data};

	// FIXME: Response::Walk can't hold more then 16 qids!
	auto result = decoder >> dest.nqids;
	for (decltype(dest.nqids) i = 0; i < dest.nqids && result; ++i) {
		result = decoder >> dest.qids[i];
	}

	if (!result)
		return result.moveError();

	return Result<ByteReader&, Error>{types::okTag, data};
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Version& dest) {
	return decode(data, dest.msize, dest.version);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Auth& dest) {
	return decode(data, dest.qid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Attach& dest) {
	return decode(data, dest.qid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Error& dest) {
	return decode(data, dest.ename);
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Open& dest) {
	return decode(data, dest.qid, dest.iounit);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Create& dest) {
	return decode(data, dest.qid, dest.iounit);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Read& dest) {
	return decode(data, dest.data);
}
Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Write& dest) {
	return decode(data, dest.count);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Stat& dest) {
	return decode(data, dest.dummySize, dest.data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Clunk& ) { return Result<ByteReader&, Error>{types::okTag, data}; }

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Remove& ) { return Result<ByteReader&, Error>{types::okTag, data}; }

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::Flush& ) { return Result<ByteReader&, Error>{types::okTag, data}; }

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Response::WStat& ) { return Result<ByteReader&, Error>{types::okTag, data}; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Request parser
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Version& dest) {
	return decode(data, dest.msize, dest.version);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Auth& dest) {
	return decode(data, dest.afid, dest.uname, dest.aname);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Attach& dest) {
	return decode(data, dest.fid, dest.afid, dest.uname, dest.aname);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Flush& dest) {
	return decode(data, dest.oldtag);
}


Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Walk& dest) {
	return decode(data, dest.fid, dest.newfid, dest.path);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Open& dest) {
	return decode(data, dest.fid, dest.mode.mode);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Create& dest) {
	return decode(data, dest.fid, dest.name, dest.perm, dest.mode.mode);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Read& dest) {
	return decode(data, dest.fid, dest.offset, dest.count);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Write& dest) {
	return decode(data, dest.fid, dest.offset, dest.data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Clunk& dest) {
	return decode(data, dest.fid);
}
Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Remove& dest) {
	return decode(data, dest.fid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::Stat& dest) {
	return decode(data, dest.fid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, Request::WStat& dest) {
	return decode(data, dest.fid, dest.stat);
}
