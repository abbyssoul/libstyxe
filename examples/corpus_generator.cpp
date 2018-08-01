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
#include "styxe/print.hpp"


#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>


using namespace Solace;


styxe::Protocol::Stat genStats() {
    styxe::Protocol::Stat result;

    result.type = 1;
    result.dev = 3;
    result.qid.path = 123;
    result.qid.type = 3;
    result.qid.version = 32;
    result.mode = 0312;
    result.atime = 291818;
    result.mtime = 727272;
    result.length = 72;

    result.size = styxe::Protocol::Encoder::protocolSize(result) - sizeof(result.size);

    return result;
}


void dumpMessage(std::string const& dest, ByteWriter& buffer) {
    std::ofstream output(dest);

    auto writenData = buffer.viewWritten();
    output.write(writenData.dataAs<char>(), writenData.size());
    buffer.clear();
}


void dumpMessage(std::string const& corpusDir, styxe::Protocol::RequestBuilder& req) {
    std::stringstream sb;
    sb << req.type();

    dumpMessage(corpusDir + "/" + sb.str(), req.buffer());
}


void dumpMessage(std::string const& corpusDir, styxe::Protocol::ResponseBuilder& resp) {
    std::stringstream sb;
    sb << resp.type();

    dumpMessage(corpusDir + "/" + sb.str(), resp.buffer());
}


int main(int argc, char const **argv) {
    if (argc < 2) {  // No-arg call: list ports and exit
        std::cout << "Usage: corpus_generator <DIRECTORY NAME>" << std::endl;
        return EXIT_FAILURE;
    }

    struct stat sb;

    if (stat(argv[1], &sb) != 0 || !S_ISDIR(sb.st_mode)) {
        std::cout << argv[1] << " - Is not a directory" << std::endl;
        return EXIT_FAILURE;
    }

    byte data[25];

    StringView userName(getenv("USER"));
    std::string corpusDir = argv[1];
    styxe::Protocol proc;

    MemoryManager memManager(proc.maxPossibleMessageSize());
    ByteWriter buffer(memManager.create(proc.maxPossibleMessageSize()));


    /// Dump request messages
    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .version());

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
            .auth(1, userName, "attachPoint"));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
            .flush(3));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
            .attach(3, 18, userName, "someFile"));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .walk(18, 42, Path({"one", "two", "file"})));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .open(42, styxe::Protocol::OpenMode::READ));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .create(42, "newFile", 0666, styxe::Protocol::OpenMode::WRITE));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .read(42, 12, 418));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .write(24, 12, wrapMemory(data).fill(0xf1)));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .clunk(24));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .remove(42));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .stat(17));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .writeStat(17, genStats()));

    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .session(wrapMemory(data)));
    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .shortRead(3, Path({"some", "location", "where", "file"})));
    dumpMessage(corpusDir, styxe::Protocol::RequestBuilder(buffer)
                .shortWrite(7, Path({"some", "location", "where", "file"}), wrapMemory(data)));


    /// Dump response messages
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .version(styxe::Protocol::PROTOCOL_VERSION));

    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .auth({1, 543, 939938}));

    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .error(StringView{"This is a test error. Please move on."}));

    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .flush());
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .attach({21, 4884, 9047302}));
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .walk({
                          {21, 4884, 9047302},
                          {22, 3242, 8488484},
                          {32, 9198, 8758379}
                      }));
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .open({21, 4884, 9047302}, 7277));
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .create({71, 4884, 32432}, 23421));

    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .read(wrapMemory(data)));
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .write(616));
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .clunk());
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .remove());
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .stat(genStats()));
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 1)
                .wstat());

    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 2)
                .session());
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 2)
                .shortRead(wrapMemory(data)));
    dumpMessage(corpusDir, styxe::Protocol::ResponseBuilder(buffer, 2)
                .shortWrite(32));

    return EXIT_SUCCESS;
}
