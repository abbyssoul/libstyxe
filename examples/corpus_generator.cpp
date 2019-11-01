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

#include "styxe/styxe.hpp"

#include <solace/array.hpp>
#include <solace/output_utils.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <random>

using namespace Solace;
using namespace styxe;


Stat genStats(StringView uid, StringView gid) {
	Stat result;

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

		auto messageName = parser.messageName(header.type);
		dumpMessage(messageName, buffer);
		req.encoder().buffer().clear();

		return *this;
	}

	MessageDump& operator() (RequestWriter& req) {
		auto header = req.header();
		auto buffer = req.encoder().buffer().viewWritten();

		auto messageName = parser.messageName(header.type);
		dumpMessage(messageName, buffer);
		req.encoder().buffer().clear();

		return *this;
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

	ParserBase& parser;
	std::string corpusDirectory;
};


/// Dump request messages
void dumpAllRequests(MemoryResource& memoryResource, std::string corpusDir, MemoryView payload) {
	auto userName = StringView{getenv("USER")};
	auto dummyStat = genStats(userName, userName);

	auto maybeParser = createRequestParser(_9P2000E::kProtocolVersion, kMaxMessageSize);
	if (!maybeParser) {
		std::cerr << "Failed to create parser: " << maybeParser.getError() << std::endl;
		return;
	}

	ByteWriter buffer{memoryResource};
	RequestWriter requestWriter{buffer, 1};
	MessageDump dump{*maybeParser, corpusDir};

	dump(requestWriter << Request::Version{kMaxMessageSize, _9P2000E::kProtocolVersion});
	dump(requestWriter << Request::Auth{1, userName, "attachPoint"});
	dump(requestWriter << Request::Flush{3});
	dump(requestWriter << Request::Attach{3, 18, userName, "someFile"});

	{
		byte pathBuffer[3 + 3 + 4 + 2*3];
		ByteWriter pathWriter{wrapMemory(pathBuffer)};
		styxe::Encoder encoder{pathWriter};
		encoder << StringView{"one"}
				<< StringView{"two"}
				<< StringView{"file"};

		dump(requestWriter << Request::Walk{18, 42, WalkPath{3, wrapMemory(pathBuffer)}});
	}

	dump(requestWriter << Request::Open{42, OpenMode::READ});
	dump(requestWriter << Request::Create{42, "newFile", 0666, OpenMode::WRITE});
	dump(requestWriter << Request::Read{42, 12, 418});
	dump(requestWriter << Request::Write{24, 12, payload});
	dump(requestWriter << Request::Clunk{24});
	dump(requestWriter << Request::Remove{42});
	dump(requestWriter << Request::Stat{17});
	dump(requestWriter << Request::WStat{17, dummyStat});

	dump(requestWriter << _9P2000E::Request::Session{{0x0F, 0xAF, 0x32, 0xFF, 0xDE, 0xAD, 0xBE, 0xEF}});
	{
		byte pathBuffer[4 + 8 + 5 + 4 + 2*4];
		ByteWriter pathWriter{wrapMemory(pathBuffer)};
		styxe::Encoder encoder{pathWriter};
		encoder << StringView{"some"}
				<< StringView{"location"}
				<< StringView{"where"}
				<< StringView{"file"};

		dump(requestWriter << _9P2000E::Request::ShortRead{3, WalkPath{4, wrapMemory(pathBuffer)}});
	}
	{
		byte pathBuffer[4 + 8 + 5 + 4 + 2*4];
		ByteWriter pathWriter{wrapMemory(pathBuffer)};
		styxe::Encoder encoder{pathWriter};
		encoder << StringView{"some"}
				<< StringView{"location"}
				<< StringView{"where"}
				<< StringView{"file"};

		dump(requestWriter << _9P2000E::Request::ShortWrite{
				 3,
				 WalkPath{4, wrapMemory(pathBuffer)},
				 payload});
	}
}


/// Dump response messages
void dumpAllResponses(MemoryResource& memoryResource, std::string corpusDir, MemoryView payload) {
	auto userName = StringView{getenv("USER")};
	auto dummyStat = genStats(userName, userName);

	auto maybeParser = createRequestParser(_9P2000E::kProtocolVersion, kMaxMessageSize);
	if (!maybeParser) {
		std::cerr << "Failed to create parser: " << maybeParser.getError() << std::endl;
		return;
	}

	ByteWriter buffer{memoryResource};
	ResponseWriter responseWriter{buffer, 1};
	MessageDump dump{*maybeParser, corpusDir};

	dump(responseWriter << Response::Version{kMaxMessageSize, _9P2000E::kProtocolVersion});
	dump(responseWriter << Response::Auth{randomQid(QidType::AUTH)});
	dump(responseWriter << Response::Error{"This is a test error. Please move on."});
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
	dump(responseWriter << Response::Stat{narrow_cast<var_datum_size_type>(protocolSize(dummyStat)), dummyStat});
	dump(responseWriter << Response::WStat{});

	dump(responseWriter << _9P2000E::Response::Session{});
	dump(responseWriter << _9P2000E::Response::ShortRead{payload});
	dump(responseWriter << _9P2000E::Response::ShortWrite{32});
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

    std::string corpusDir = argv[1];

	MemoryManager memManager{kMaxMessageSize};
	auto memoryResource = memManager.allocate(kMaxMessageSize);
	if (!memoryResource) {
		std::cerr << "Feiled to allocate memory for a buffer: " << memoryResource.getError() << std::endl;
		return EXIT_FAILURE;
	}

	byte data[32];
	auto payload = wrapMemory(data);
	payload.fill(0xf1);

	dumpAllRequests(*memoryResource, corpusDir, payload);
	dumpAllResponses(*memoryResource, corpusDir, payload);

    return EXIT_SUCCESS;
}
