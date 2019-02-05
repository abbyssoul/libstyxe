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
#include "styxe/version.hpp"
#include "styxe/print.hpp"

#include <solace/base16.hpp>
#include <solace/output_utils.hpp>

#include <clime/parser.hpp>


#include <iostream>
#include <fstream>
#include <iomanip>


using namespace Solace;
using namespace styxe;


template<typename T>
struct Quoted {
    Quoted(T const& t, char quote='\'')
        : _t(t)
        , _quote(quote)
    {}

    friend std::ostream& operator<< (std::ostream& ostr, Quoted<T> const& q) {
        return ostr << q._quote << q._t << q._quote;
    }

    char const _quote;
    T const& _t;
};

template<typename T>
Quoted<T> quote(T const& t, char q='\'') { return Quoted<T>(t, q); }


std::ostream& operator<< (std::ostream& ostr, Protocol::OpenMode mode) {
    switch (mode) {
    case Protocol::OpenMode::READ: ostr << "READ"; break;
    case Protocol::OpenMode::WRITE: ostr << "WRITE"; break;
    case Protocol::OpenMode::RDWR: ostr << "RDWR"; break;
    case Protocol::OpenMode::EXEC: ostr << "EXEC"; break;
    case Protocol::OpenMode::TRUNC: ostr << "TRUNC"; break;
    case Protocol::OpenMode::CEXEC: ostr << "CEXEC"; break;
    case Protocol::OpenMode::RCLOSE: ostr << "RCLOSE"; break;
    }

    return ostr;
}

std::ostream& operator<< (std::ostream& ostr, Protocol::Qid const& qid) {
    return ostr << '{'
                << "type: " << static_cast<int>(qid.type) << ", "
                << "ver: "  << qid.version << ", "
                << "path: " << qid.path
                << '}';

}

std::ostream& operator<< (std::ostream& ostr, Protocol::Stat const& stat) {
    return ostr << '{'
         << "size: "    << stat.size    << ", "
         << "type: "    << stat.type    << ", "
         << "dev: "     << stat.dev     << ", "
         << "qid: "     << stat.qid     << ", "
         << "mode: "    << stat.mode    << ", "
         << "atime: "   << stat.atime   << ", "
         << "mtime: "   << stat.mtime   << ", "
         << "length: "  << stat.length  << ", "
         << "name: \""  << stat.name    << "\", "
         << "uid: \""   << stat.uid     << "\", "
         << "gid: \""   << stat.gid     << "\", "
         << "muid: \""  << stat.muid    << "\""
         << '}';
}


void onVersionRequest(Protocol::Request::Version& req) {
    std::cout << req.msize << ' ' << quote(req.version, '\"') <<std::endl;
}

void onAuthRequest(Protocol::Request::Auth& req) {
    std::cout << req.afid << " \"" << req.uname << "\" \"" << req.aname << "\"" << std::endl;
}

void onAttachRequest(Protocol::Request::Attach& req) {
    std::cout << req.fid << " " << req.afid << " \"" << req.uname << "\" \"" << req.aname << "\"" << std::endl;
}

void onClankRequest(Protocol::Request::Clunk& req) {
    std::cout << req.fid << std::endl;
}

void onFlushRequest(Protocol::Request::Flush& req) {
    std::cout << req.oldtag << std::endl;
}

void onOpenRequest(Protocol::Request::Open& req) {
    std::cout << req.fid << " " << req.mode << std::endl;
}

void onCreateRequest(Protocol::Request::Create& req) {
    std::cout << req.fid << " \"" << req.name << "\" " << req.perm << " " << req.mode << std::endl;
}


void onReadRequest(Protocol::Request::Read& req) {
    std::cout << req.fid << " " << req.offset << " " << req.count << std::endl;
}

void onWriteRequest(Protocol::Request::Write& req) {
    std::cout << req.fid << " " << req.offset << " " << req.data.size()
              << " DATA[" << req.data << "]"
              << std::endl;
}

void onRemoveRequest(Protocol::Request::Remove& req) {
    std::cout << req.fid << std::endl;
}

void onStatRequest(Protocol::Request::StatRequest& req) {
    std::cout << req.fid << std::endl;
}

void onWStatRequest(Protocol::Request::WStat& req) {
    std::cout << req.fid << " " << req.stat << std::endl;
}


void onWalkRequest(Protocol::Request::Walk& req) {
    std::cout << req.fid << ' '
              << req.newfid << ' '
              << req.path.getComponentsCount() << ' ' << '[';

    for (auto& c : req.path) {
        std::cout << '\''<< c << '\'' << ' ';
    }
    std::cout << ']' << std::endl;
}


void onSessionRequest(Protocol::Request::Session& req) {
    std::cout << wrapMemory(req.key) << std::endl;
}


void onShortReadRequest(Protocol::Request::SRead& req) {
    std::cout << req.fid << ' '
              << '\'' << req.path.toString() << '\''
              << std::endl;
}


void onShortWriteRequest(Protocol::Request::SWrite& req) {
    std::cout << req.fid << ' '
              << '\'' << req.path.toString() << '\''
              << " DATA[" << req.data << "]"
              << std::endl;
}


void onVoidResponse() {
    std::cout << std::endl;
}

void onReadResponse(Protocol::Response::Read const& resp) {
    std::cout << resp.data.size()
              << " DATA[" << resp.data << ']'
              << std::endl;
}

void onWriteResponse(Protocol::Response::Write const& resp) {
    std::cout << resp.count
              << std::endl;
}

void onStatResponse(Protocol::Stat const& stat) {
    std::cout << stat << std::endl;
}

void onCreateResponse(Protocol::Response::Create const& resp) {
    std::cout << resp.qid << " " << resp.iounit << std::endl;
}

void onOpenResponse(Protocol::Response::Open const& resp) {
    std::cout << resp.qid << " " << resp.iounit << std::endl;
}

void onAttachResponse(Protocol::Response::Attach const& resp) {
    std::cout << resp.qid << std::endl;
}

void onAuthResponse(Protocol::Response::Auth const& resp) {
    std::cout << resp.qid << std::endl;
}

void onVersionResponse(Protocol::Response::Version const& resp) {
    std::cout << resp.msize << " \"" << resp.version << '\"' <<std::endl;
}

void onErrorResponse(Protocol::Response::Error const& resp) {
    std::cout << " \"" << resp.ename << '\"' <<std::endl;
}

void onWalkResponse(Protocol::Response::Walk const& resp) {
    std::cout << resp.nqids
              << '[';
     for (uint i = 0; i < resp.nqids; ++i) {
         std::cout << resp.qids[i] << ' ';
     }
     std::cout << ']' << std::endl;
}



void dispayRequest(Protocol::Request&& req) {
    switch (req.type()) {
    case Protocol::MessageType::TVersion:     onVersionRequest(req.asVersion());     break;
    case Protocol::MessageType::TAuth:        onAuthRequest(req.asAuth());           break;
    case Protocol::MessageType::TAttach:      onAttachRequest(req.asAttach());       break;
    case Protocol::MessageType::TFlush:       onFlushRequest(req.asFlush());         break;
    case Protocol::MessageType::TWalk:        onWalkRequest(req.asWalk());           break;
    case Protocol::MessageType::TOpen:        onOpenRequest(req.asOpen());           break;
    case Protocol::MessageType::TCreate:      onCreateRequest(req.asCreate());   break;
    case Protocol::MessageType::TRead:        onReadRequest(req.asRead());           break;
    case Protocol::MessageType::TWrite:       onWriteRequest(req.asWrite());      break;
    case Protocol::MessageType::TClunk:       onClankRequest(req.asClunk());         break;
    case Protocol::MessageType::TRemove:      onRemoveRequest(req.asRemove());    break;
    case Protocol::MessageType::TStat:        onStatRequest(req.asStat());           break;
    case Protocol::MessageType::TWStat:       onWStatRequest(req.asWstat());      break;

    case Protocol::MessageType::TSession:     onSessionRequest(req.asSession());  break;
    case Protocol::MessageType::TSRead:       onShortReadRequest(req.asShortRead());    break;
    case Protocol::MessageType::TSWrite:      onShortWriteRequest(req.asShortWrite());  break;

    default:
        std::cerr << "<Unknown message type>" << std::endl;
    }
}

void dispayResponse(Protocol::Response&& resp) {
    switch (resp.type) {
    case Protocol::MessageType::RVersion:     onVersionResponse(resp.version);     break;
    case Protocol::MessageType::RAuth:        onAuthResponse(resp.auth);           break;
    case Protocol::MessageType::RAttach:      onAttachResponse(resp.attach);       break;
    case Protocol::MessageType::RError:       onErrorResponse(resp.error);      break;
    case Protocol::MessageType::RFlush:       onVoidResponse();         break;
    case Protocol::MessageType::RWalk:        onWalkResponse(resp.walk);           break;
    case Protocol::MessageType::ROpen:        onOpenResponse(resp.open);           break;
    case Protocol::MessageType::RCreate:      onCreateResponse(resp.create);   break;
    case Protocol::MessageType::RRead:        onReadResponse(resp.read);           break;
    case Protocol::MessageType::RWrite:       onWriteResponse(resp.write);      break;
    case Protocol::MessageType::RClunk:       onVoidResponse();         break;
    case Protocol::MessageType::RRemove:      onVoidResponse();    break;
    case Protocol::MessageType::RStat:        onStatResponse(resp.stat);           break;
    case Protocol::MessageType::RWStat:       onVoidResponse();      break;

    case Protocol::MessageType::RSession:     onVoidResponse();  break;
    case Protocol::MessageType::RSRead:       onReadResponse(resp.read);    break;
    case Protocol::MessageType::RSWrite:      onWriteResponse(resp.write);  break;

    default:
        std::cerr << "<Unknown message type>" << std::endl;
    }
}




void readAndPrintMessage(std::istream& in, MemoryResource& buffer, styxe::Protocol& proc) {

    // Message header is fixed size - so it is safe to attempt to read it.
    in.read(buffer.view().dataAs<char>(), proc.headerSize());

    auto reader = ByteReader(buffer);
    reader.limit(in.gcount());

    proc.parseMessageHeader(reader)
            .then([&](Protocol::MessageHeader&& header) {
                in.read(buffer.view().dataAs<char>(), header.payloadSize());

                reader.rewind()
                        .limit(in.gcount());

                bool const isRequest = (static_cast<byte>(header.type) % 2) == 0;
                if (isRequest) {
                    std::cout << "→ [" << std::setw(5) << header.messageSize
                              << "] <" << header.tag << ">"
                              << header.type << ": ";

                    proc.parseRequest(header, reader)
                            .then(dispayRequest);
                } else {
                    std::cout << "→ [" << std::setw(5) << header.messageSize << "] "
                              << header.type << " "
                              << header.tag << ": ";

                    proc.parseResponse(header, reader)
                            .then(dispayResponse);
                }
            })
            .orElse([](Error&& err) {
                std::cerr << "Error parsing message header: " << err.toString() << std::endl;
            });
}


/**
 * A simple example of decoding a 9P message from a file / stdin and printing it in a human readable format.
 */
int main(int argc, const char **argv) {

    Optional<uint> inputFiles;
    auto maxMessageSize = Protocol::MAX_MESSAGE_SIZE;
    auto requiredVersion = Protocol::PROTOCOL_VERSION;

    auto const parseArgs = clime::Parser("Decode and print 9P message")
            .options({
                         clime::Parser::printVersion("9pdecode", {1, 0, 0}),
                         clime::Parser::printHelp(),
                         {{"m", "msize"}, "Maximum message size", &maxMessageSize},
                         {{"p", "proc"}, "Protocol version", &requiredVersion}
            })
            .arguments({{"*", "Files", [&inputFiles] (StringView, clime::Parser::Context const& c) -> Optional<Error> {

                             if (!inputFiles) {
                                inputFiles = Optional<uint>(c.offset);
                             }

                             return none;
                         }}})
            .parse(argc, argv);

    if (!parseArgs) {
        std::cerr << "Error: " << parseArgs.getError().toString() << std::endl;
        return EXIT_FAILURE;
    }

    Protocol proc(maxMessageSize);
    MemoryManager memManager(proc.maxPossibleMessageSize());
    auto buffer = memManager.allocate(proc.maxPossibleMessageSize());


    if (inputFiles) {
        for (uint i = inputFiles.get(); i < argc; ++i) {
            std::ifstream input(argv[i]);
            if (!input) {
                std::cerr << "Failed to open file: " << std::quoted(argv[i]) << std::endl;
                return EXIT_FAILURE;
            }

            readAndPrintMessage(input, buffer, proc);
        }
    } else {
        readAndPrintMessage(std::cin, buffer, proc);
    }

    return EXIT_SUCCESS;
}

