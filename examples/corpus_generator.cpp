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
#include "styxe/requestWriter.hpp"
#include "styxe/responseWriter.hpp"
#include "styxe/print.hpp"

#include <solace/array.hpp>
#include <solace/output_utils.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>


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


void dumpMessage(std::string const& corpusDir, MessageType type, ByteWriter const& buffer) {

	std::stringstream sb;
	sb << type;

	std::ofstream output(corpusDir + "/" + sb.str());

    auto writenData = buffer.viewRemaining();
    output.write(writenData.dataAs<char>(), writenData.size());
}


void dumpMessage(std::string const& corpusDir, TypedWriter&& req) {
	dumpMessage(corpusDir, req.type(), req.build());
	req.buffer().clear();
}

void dumpMessage(std::string const& corpusDir, RequestWriter::PathWriter&& req) {
	dumpMessage(corpusDir, req.type(), req.build());
	req.buffer().clear();
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
	byte data[32];
	auto userName = StringView{getenv("USER")};

	Parser proc;
	MemoryManager memManager{proc.maxPossibleMessageSize()};

	auto memoryResource = memManager.allocate(proc.maxPossibleMessageSize());
	if (!memoryResource) {
		std::cerr << "Feiled to allocate memory for a buffer: " << memoryResource.getError();
		return EXIT_FAILURE;
	}


	ByteWriter buffer{memoryResource.unwrap()};

    /// Dump request messages
	dumpMessage(corpusDir, RequestWriter{buffer}
				.version(Parser::PROTOCOL_VERSION));

	dumpMessage(corpusDir, RequestWriter{buffer}
            .auth(1, userName, "attachPoint"));

	dumpMessage(corpusDir, RequestWriter{buffer}
            .flush(3));

	dumpMessage(corpusDir, RequestWriter{buffer}
            .attach(3, 18, userName, "someFile"));

	dumpMessage(corpusDir, RequestWriter{buffer}
				.walk(18, 42)
				.path("one")
				.path("two")
				.path("file")
				.done());

	dumpMessage(corpusDir, RequestWriter{buffer}
				.open(42, OpenMode::READ));

	dumpMessage(corpusDir, RequestWriter{buffer}
				.create(42, "newFile", 0666, OpenMode::WRITE));

	dumpMessage(corpusDir, RequestWriter{buffer}
                .read(42, 12, 418));

	dumpMessage(corpusDir, RequestWriter{buffer}
				.write(24, 12)
				.data(wrapMemory(data).fill(0xf1))
				);

	dumpMessage(corpusDir, RequestWriter{buffer}
                .clunk(24));

	dumpMessage(corpusDir, RequestWriter{buffer}
                .remove(42));

	dumpMessage(corpusDir, RequestWriter{buffer}
                .stat(17));

	dumpMessage(corpusDir, RequestWriter{buffer}
                .writeStat(17, genStats(userName, userName)));

	dumpMessage(corpusDir, RequestWriter{buffer}
                .session(wrapMemory(data, 8)));
	dumpMessage(corpusDir, RequestWriter{buffer}
				.shortRead(3)
				.path("some")
				.path("location")
				.path("where")
				.path("file")
				.done());
	dumpMessage(corpusDir, RequestWriter{buffer}
				.shortWrite(7)
				.path("some")
				.path("location")
				.path("where")
				.path("file")
				.data(wrapMemory(data))
				);


    /// Dump response messages
	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
				.version(Parser::PROTOCOL_VERSION));

	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .auth({1, 543, 939938}));

	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .error("This is a test error. Please move on."));

	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .flush());
	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .attach({21, 4884, 9047302}));

    {
		auto steps = makeArrayOf<Qid>(
								  Qid{21, 4884, 9047302},
								  Qid{22, 3242, 8488484},
								  Qid{32, 9198, 8758379}
                              );
		dumpMessage(corpusDir, ResponseWriter{buffer, 1}
					.walk(steps.unwrap().view()));
    }

	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .open({21, 4884, 9047302}, 7277));
	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .create({71, 4884, 32432}, 23421));

	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .read(wrapMemory(data)));
	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .write(616));
	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .clunk());
	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .remove());
	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .stat(genStats(userName, userName)));
	dumpMessage(corpusDir, ResponseWriter{buffer, 1}
                .wstat());

	dumpMessage(corpusDir, ResponseWriter{buffer, 2}
                .session());
	dumpMessage(corpusDir, ResponseWriter{buffer, 2}
                .shortRead(wrapMemory(data)));
	dumpMessage(corpusDir, ResponseWriter{buffer, 2}
                .shortWrite(32));

    return EXIT_SUCCESS;
}
