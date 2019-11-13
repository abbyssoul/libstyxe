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
#include "styxe/9p2000u.hpp"
#include "styxe/9p2000e.hpp"
#include "styxe/9p2000L.hpp"
#include "styxe/messageWriter.hpp"

#include <solace/array.hpp>
#include <solace/output_utils.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <random>

using namespace Solace;
using namespace styxe;

using MessageNameMapper = Solace::StringView(*)(Solace::byte);

void genStats(Stat& result, StringView uid, StringView gid) {
	result.type = 1;
	result.dev = 3;
	result.qid.path = 123;
	result.qid.type = 3;
	result.qid.version = 32;
	result.mode = 0312;
	result.atime = 291818;
	result.mtime = 727272;
	result.length = 72;
	result.uid = uid;
	result.gid = gid;
	result.muid = uid;

	result.size = DirListingWriter::sizeStat(result);
}

Stat genStats(StringView uid, StringView gid) {
	Stat result;

	genStats(result, uid, gid);

    return result;
}


_9P2000U::v9fs_stat
genStatsExt(StringView uid, StringView gid) {
	_9P2000U::v9fs_stat result;

	genStats(result, uid, gid);

	result.extension = "Extras";
	result.n_uid = 31232;
	result.n_gid = 231;
	result.n_muid = 871;

	result.size = DirListingWriter::sizeStat(result);

	return result;
}


Qid randomQid(QidType type) {
	std::random_device rd;
	std::default_random_engine randomGen{rd()};
	std::linear_congruential_engine<uint32, 48271, 0, 2147483647> randomGen2{rd()};

	return {
		randomGen(),
		randomGen2(),
		static_cast<byte>(type)
	};
}


/// Message write helper
struct MessageDump {

	MessageDump& operator() (ResponseWriter& req) {
		auto header = req.header();
		auto buffer = req.encoder().buffer().viewWritten();

		auto messageName = messageCodeToName(header.type);
		dumpMessage(messageName, buffer);
		req.encoder().buffer().clear();

		return *this;
	}

	MessageDump& operator() (RequestWriter& req) {
		auto header = req.header();
		auto buffer = req.encoder().buffer().viewWritten();

		auto messageName = messageCodeToName(header.type);
		dumpMessage(messageName, buffer);
		req.encoder().buffer().clear();

		return *this;
	}

	MessageDump& operator() (PathWriter&& pw) {
		return operator() (pw.writer());
	}


	void dumpMessage(StringView messageType, MemoryView buffer) {
		std::stringstream sb;
		sb << messageType;

		std::ofstream output{corpusDirectory + "/" + sb.str()};
		if (!output) {
			std::cerr << "Failed to open output file: '"
					  << corpusDirectory << "/" << sb.str() << "'\n";
			return;
		}

		output.write(buffer.dataAs<char>(), buffer.size());
	}

	MessageNameMapper	messageCodeToName;
	std::string			corpusDirectory;
};


/// Dump request messages
void dumpAllRequests(MemoryResource& memoryResource, std::string corpusDir, StringView version,
					 MessageNameMapper messageCodeToName, MemoryView payload) {
	auto userName = StringView{getenv("USER")};
	Solace::uint32 const n_uname = 3213;
	auto dummyStat = genStats(userName, userName);

	ByteWriter buffer{memoryResource};
	RequestWriter requestWriter{buffer, 1};
	MessageDump dump{messageCodeToName, corpusDir};

	dump(requestWriter << Request::Version{kMaxMessageSize, version});
	dump(requestWriter << Request::Flush{3});

	dump(requestWriter << Request::Partial::Walk{18, 42}
		 << StringView{"one"}
		 << StringView{"two"}
		 << StringView{"file"});

	dump(requestWriter << Request::Open{42, OpenMode::READ});
	dump(requestWriter << Request::Read{42, 12, 418});
	dump(requestWriter << Request::Write{24, 12, payload});
	dump(requestWriter << Request::Clunk{24});
	dump(requestWriter << Request::Remove{42});
	dump(requestWriter << Request::Stat{17});

	if (version == _9P2000U::kProtocolVersion) {
		dump(requestWriter << _9P2000U::Request::Auth{1, userName, "attachPoint", n_uname});
		dump(requestWriter << _9P2000U::Request::Attach{3, 18, userName, "someFile", n_uname});
		dump(requestWriter << _9P2000U::Request::Create{42, "newFile", 0666, OpenMode::WRITE, "xtras"});
		dump(requestWriter << _9P2000U::Request::WStat{17, genStatsExt(userName, userName)});
	} else {
		dump(requestWriter << Request::Auth{1, userName, "attachPoint"});
		dump(requestWriter << Request::Attach{3, 18, userName, "someFile"});
		dump(requestWriter << Request::Create{42, "newFile", 0666, OpenMode::WRITE});
		dump(requestWriter << Request::WStat{17, dummyStat});
	}

	// Generate extra messages based on extention selected
	if (version == _9P2000E::kProtocolVersion) {
		dump(requestWriter << _9P2000E::Request::Session{{0x0F, 0xAF, 0x32, 0xFF, 0xDE, 0xAD, 0xBE, 0xEF}});

		dump(requestWriter << _9P2000E::Request::Partial::ShortRead{3}
			 << StringView{"some"}
			 << StringView{"location"}
			 << StringView{"where"}
			 << StringView{"file"});

		dump(requestWriter << _9P2000E::Request::Partial::ShortWrite{3}
			 << StringView{"some"}
			 << StringView{"location"}
			 << StringView{"where"}
			 << StringView{"file"}
			 << payload);
	} else if (version == _9P2000L::kProtocolVersion) {
		Solace::uint32 gid{45345};

		dump(requestWriter << _9P2000L::Request::StatFS{3213});
		dump(requestWriter << _9P2000L::Request::Open{1234, 1348763});
		dump(requestWriter << _9P2000L::Request::Create{});
		dump(requestWriter << _9P2000L::Request::Symlink{3123, "xfile", "yfile", gid});
		dump(requestWriter << _9P2000L::Request::MkNode{21132, "nnode", 23432, 123, 3212, gid});
		dump(requestWriter << _9P2000L::Request::Rename{123, 213, "xname"});
		dump(requestWriter << _9P2000L::Request::ReadLink{213});
		dump(requestWriter << _9P2000L::Request::GetAttr{123, 1232132});
		dump(requestWriter << _9P2000L::Request::SetAttr{});
		dump(requestWriter << _9P2000L::Request::XAttrWalk{3123, 3213, "attrx"});
		dump(requestWriter << _9P2000L::Request::XAttrCreate{3123, "attrxy", 321, 896123});
		dump(requestWriter << _9P2000L::Request::ReadDir{3213, 1, 432});
		dump(requestWriter << _9P2000L::Request::FSync{3213});
		dump(requestWriter << _9P2000L::Request::Lock{3213, 1, 3213, 23, 2048, 3213, "Awesome"});
		dump(requestWriter << _9P2000L::Request::GetLock{23123, 1, 1024, 2048, 3213, "Awesome"});
		dump(requestWriter << _9P2000L::Request::Link{3213, 123, "linkx"});
		dump(requestWriter << _9P2000L::Request::MkDir{3213, "s3cret", 32, gid});
		dump(requestWriter << _9P2000L::Request::RenameAt{321, "oldy", 3213, "goodie"});
		dump(requestWriter << _9P2000L::Request::UnlinkAt{321, "oldy", 3213});
	}
}


/// Dump response messages
void dumpAllResponses(MemoryResource& memoryResource, std::string corpusDir, StringView version,
					  MessageNameMapper messageCodeToName, MemoryView payload) {
	auto userName = StringView{getenv("USER")};
	auto dummyStat = genStats(userName, userName);

	ByteWriter buffer{memoryResource};
	ResponseWriter responseWriter{buffer, 1};
	MessageDump dump{messageCodeToName, corpusDir};

	dump(responseWriter << Response::Version{kMaxMessageSize, version});
	dump(responseWriter << Response::Auth{randomQid(QidType::AUTH)});
	dump(responseWriter << Response::Flush{});
	dump(responseWriter << Response::Attach{randomQid(QidType::MOUNT)});
	dump(responseWriter << Response::Walk{3, {
											  randomQid(QidType::FILE),
											  randomQid(QidType::FILE),
											  randomQid(QidType::FILE)}});

	dump(responseWriter << Response::Open{randomQid(QidType::FILE), 4096});
	dump(responseWriter << Response::Create{randomQid(QidType::FILE), 4096});
	dump(responseWriter << Response::Read{payload});
	dump(responseWriter << Response::Write{616});
	dump(responseWriter << Response::Clunk{});
	dump(responseWriter << Response::Remove{});
	dump(responseWriter << Response::WStat{});

	if (version == _9P2000U::kProtocolVersion) {
		dump(responseWriter << _9P2000U::Response::Error{"This is a test error. Please move on.", 32});

		auto extStat = genStatsExt(userName, userName);
		dump(responseWriter << _9P2000U::Response::Stat{narrow_cast<var_datum_size_type>(protocolSize(extStat)),
														extStat});
	} else {
		dump(responseWriter << Response::Error{"This is a test error. Please move on."});
		dump(responseWriter << Response::Stat{narrow_cast<var_datum_size_type>(protocolSize(dummyStat)), dummyStat});
	}

	// Generate extra messages based on extention selected
	if (version == _9P2000E::kProtocolVersion) {
		dump(responseWriter << _9P2000E::Response::Session{});
		dump(responseWriter << _9P2000E::Response::ShortRead{payload});
		dump(responseWriter << _9P2000E::Response::ShortWrite{32});
	} else if (version == _9P2000L::kProtocolVersion) {
		dump(responseWriter << _9P2000L::Response::LError{});
		dump(responseWriter << _9P2000L::Response::StatFS{});
		dump(responseWriter << _9P2000L::Response::Open{randomQid(QidType::FILE), 4096});
		dump(responseWriter << _9P2000L::Response::Create{randomQid(QidType::FILE), 4096});
		dump(responseWriter << _9P2000L::Response::Symlink{randomQid(QidType::LINK)});
		dump(responseWriter << _9P2000L::Response::MkNode{randomQid(QidType::MOUNT)});
		dump(responseWriter << _9P2000L::Response::Rename{});
		dump(responseWriter << _9P2000L::Response::ReadLink{"linksy"});
		dump(responseWriter << _9P2000L::Response::GetAttr{});
		dump(responseWriter << _9P2000L::Response::SetAttr{});
		dump(responseWriter << _9P2000L::Response::XAttrWalk{});
		dump(responseWriter << _9P2000L::Response::XAttrCreate{});
		dump(responseWriter << _9P2000L::Response::FSync{});
		dump(responseWriter << _9P2000L::Response::Lock{32});
		dump(responseWriter << _9P2000L::Response::GetLock{32, 1024, 2048, 3213, "Awesome"});
		dump(responseWriter << _9P2000L::Response::Link{});
		dump(responseWriter << _9P2000L::Response::MkDir{randomQid(QidType::DIR)});
		dump(responseWriter << _9P2000L::Response::RenameAt{});
		dump(responseWriter << _9P2000L::Response::UnlinkAt{});

		char dirBuffer[127];
		MutableMemoryView data = wrapMemory(dirBuffer);
		_9P2000L::DirEntry const entries[] = {
			{randomQid(QidType::FILE), 0, 31, StringView{"data"}},
			{randomQid(QidType::DIR), 4, 31, StringView{"Awesome file"}},
			{randomQid(QidType::FILE), 1, 32, StringView{"other file"}}
		};

		ByteWriter dirStream{data};
		styxe::Encoder encoder{dirStream};
		for (auto const& e : entries) {
			encoder << e;
		}

		dump(responseWriter << _9P2000L::Response::ReadDir{data});
	}

}


int main(int argc, char const **argv) {
    if (argc < 2) {  // No-arg call: list ports and exit
        std::cerr << "Usage: corpus_generator <DIRECTORY_NAME>" << std::endl;
        return EXIT_FAILURE;
    }

    // Check if provided argument names a directory:
    struct stat sb;
    if (stat(argv[1], &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        std::cerr << argv[1] << " - Is not a directory" << std::endl;
        return EXIT_FAILURE;
    }

	std::string corpusDir{argv[1]};
	StringView protocolVersion{kProtocolVersion};
	if (argc > 2) {
		protocolVersion = argv[2];
	}


	MemoryManager memManager{kMaxMessageSize};
	auto memoryResource = memManager.allocate(kMaxMessageSize);
	if (!memoryResource) {
		std::cerr << "Failed to allocate memory for a buffer: " << memoryResource.getError() << std::endl;
		return EXIT_FAILURE;
	}

	byte data[32];
	auto payload = wrapMemory(data);
	payload.fill(0xf1);

	MessageNameMapper mapper = messageTypeToString;
	if (_9P2000E::kProtocolVersion == protocolVersion) {
		mapper = _9P2000E::messageTypeToString;
	} else if (_9P2000U::kProtocolVersion == protocolVersion) {
		mapper = _9P2000U::messageTypeToString;
	} else if (_9P2000L::kProtocolVersion == protocolVersion) {
		mapper = _9P2000L::messageTypeToString;
	}


	dumpAllRequests(*memoryResource, corpusDir, protocolVersion, mapper, payload);
	dumpAllResponses(*memoryResource, corpusDir, protocolVersion, mapper, payload);

    return EXIT_SUCCESS;
}
