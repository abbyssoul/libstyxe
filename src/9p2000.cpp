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
#include "styxe/version.hpp"

#include "parse_helper.hpp"

#include <solace/assert.hpp>
#include <algorithm>  // std::min


using namespace Solace;
using namespace styxe;

static const Version   kLibVersion{STYXE_VERSION_MAJOR, STYXE_VERSION_MINOR, STYXE_VERSION_BUILD};

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



Version const&
styxe::getVersion() noexcept {
	return kLibVersion;
}


styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Walk& dest) {
	Decoder decoder{data};

	// FIXME: Response::Walk can't hold more then 16 qids!
	auto result = decoder >> dest.nqids;
	for (decltype(dest.nqids) i = 0; i < dest.nqids && result; ++i) {
		result = decoder >> dest.qids[i];
	}

	if (!result)
		return result.moveError();

	return styxe::Result<ByteReader&>{types::okTag, data};
}


styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Version& dest) {
	return decode(data, dest.msize, dest.version);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Auth& dest) {
	return decode(data, dest.qid);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Attach& dest) {
	return decode(data, dest.qid);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Error& dest) {
	return decode(data, dest.ename);
}


styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Open& dest) {
	return decode(data, dest.qid, dest.iounit);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Create& dest) {
	return decode(data, dest.qid, dest.iounit);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Read& dest) {
	return decode(data, dest.data);
}
styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Write& dest) {
	return decode(data, dest.count);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Stat& dest) {
	return decode(data, dest.dummySize, dest.data);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Clunk& ) { return styxe::Result<ByteReader&>{types::okTag, data}; }

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Remove& ) { return styxe::Result<ByteReader&>{types::okTag, data}; }

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::Flush& ) { return styxe::Result<ByteReader&>{types::okTag, data}; }

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Response::WStat& ) { return styxe::Result<ByteReader&>{types::okTag, data}; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Request parser
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Version& dest) {
	return decode(data, dest.msize, dest.version);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Auth& dest) {
	return decode(data, dest.afid, dest.uname, dest.aname);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Attach& dest) {
	return decode(data, dest.fid, dest.afid, dest.uname, dest.aname);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Flush& dest) {
	return decode(data, dest.oldtag);
}


styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Walk& dest) {
	return decode(data, dest.fid, dest.newfid, dest.path);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Open& dest) {
	return decode(data, dest.fid, dest.mode.mode);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Create& dest) {
	return decode(data, dest.fid, dest.name, dest.perm, dest.mode.mode);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Read& dest) {
	return decode(data, dest.fid, dest.offset, dest.count);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Write& dest) {
	return decode(data, dest.fid, dest.offset, dest.data);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Clunk& dest) {
	return decode(data, dest.fid);
}
styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Remove& dest) {
	return decode(data, dest.fid);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::Stat& dest) {
	return decode(data, dest.fid);
}

styxe::Result<ByteReader&>
styxe::operator>> (ByteReader& data, Request::WStat& dest) {
	return decode(data, dest.fid, dest.stat);
}


StringView
styxe::messageTypeToString(byte type) noexcept {
	auto mType = static_cast<styxe::MessageType>(type);
	switch (mType) {
	case MessageType::TVersion: return "TVersion";
	case MessageType::RVersion: return "RVersion";
	case MessageType::TAuth:    return "TAuth";
	case MessageType::RAuth:    return "RAuth";
	case MessageType::TAttach:  return "TAttach";
	case MessageType::RAttach:  return "RAttach";
	case MessageType::TError:   return "TError";
	case MessageType::RError:   return "RError";
	case MessageType::TFlush:   return "TFlush";
	case MessageType::RFlush:   return "RFlush";
	case MessageType::TWalk:    return "TWalk";
	case MessageType::RWalk:    return "RWalk";
	case MessageType::TOpen:    return "TOpen";
	case MessageType::ROpen:    return "ROpen";
	case MessageType::TCreate:  return "TCreate";
	case MessageType::RCreate:  return "RCreate";
	case MessageType::TRead:    return "TRead";
	case MessageType::RRead:    return "RRead";
	case MessageType::TWrite:   return "TWrite";
	case MessageType::RWrite:   return "RWrite";
	case MessageType::TClunk:   return "TClunk";
	case MessageType::RClunk:   return "RClunk";
	case MessageType::TRemove:  return "TRemove";
	case MessageType::RRemove:  return "RRemove";
	case MessageType::TStat:    return "TStat";
	case MessageType::RStat:    return "RStat";
	case MessageType::TWStat:   return "TWStat";
	case MessageType::RWStat:   return "RWStat";
	}

	return "Unsupported";
}
