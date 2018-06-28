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

#include <solace/base64.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>


using namespace Solace;
using namespace styxe;


std::ostream& operator<< (std::ostream& ostr, ImmutableMemoryView const& view) {
    if (view.size() > 0) {
        // We use custom output printing each byte as \0 bytest and \n are not printable otherwise.
        for (const auto& b : view) {
            ostr << std::hex << static_cast<int>(b);
        }
    } else {
        ostr << "<null>";
    }

    return ostr;
}


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


std::ostream& operator<< (std::ostream& ostr, Protocol::Stat const& stat) {
    ostr << "{"
         << "size: " << stat.size << ", "
         << "type: " << stat.type << ", "
         << "dev: " << stat.dev << ", "
         << "qid: {"
             << "type: " << stat.qid.type << ", "
             << "ver: " << stat.qid.version << ", "
             << "path: " << stat.qid.path << ", "
         << "}, "
         << "mode: " << stat.mode << ", "
         << "atime: " << stat.atime << ", "
         << "mtime: " << stat.mtime << ", "
         << "length: " << stat.length << ", "

         << "name: \"" << stat.name << "\", "
         << "uid: \"" << stat.uid << "\", "
         << "gid: \"" << stat.gid << "\", "
         << "muid: \"" << stat.muid << "\""
         << "}";

    return ostr;
}




void onVersionRequest(Protocol::Request::Version& req) {
    std::cout << req.msize << " \"" << req.version << "\"" <<std::endl;
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
    std::cout << req.fid << " " << req.newfid << " "
              << req.path.getComponentsCount() << " [";

    for (auto& c: req.path) {
        std::cout << std::quoted(c.c_str()) << " ";
    }
    std::cout << "]" << std::endl;
}


void onSessionRequest(Protocol::Request::Session& req) {
    std::cout << req.key << std::endl;
}


void onShortReadRequest(Protocol::Request::SRead& req) {
    std::cout << req.fid << " " << std::quoted(req.path.toString().to_str()) << std::endl;
}


void onShortWriteRequest(Protocol::Request::SWrite& req) {
    std::cout << req.fid << " " << std::quoted(req.path.toString().to_str())
              << " DATA[" << req.data << "]"
              << std::endl;
}


void readAndPrintMessage(std::istream& in, ByteBuffer& buffer, styxe::Protocol& proc) {
    in.read(buffer.viewRemaining().dataAs<char>(), buffer.limit());
    const size_t got = in.gcount();
    buffer.limit(got);

    proc.parseMessageHeader(buffer)
            .then([&](Protocol::MessageHeader&& header) {
//                std::cout << "→ [" << std::setw(5) << header.size << "] " << header.type << " " << header.tag << ": ";
                std::cout << "→ [" << std::setw(5) << header.size << "] <" << header.tag << ">" << header.type << ": ";

                return proc.parseRequest(header, buffer);
            })
    .then([&](Protocol::Request&& req) {
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
        case Protocol::MessageType::TSWrite:      onShortWriteRequest(req.asShortWrite());break;

        default:
            std::cerr << "<Unknown message type>" << std::endl;
        }
    })
    .orElse([](Error&& err) {
        std::cerr << "Error parsing message: " << err.toString() << std::endl;
    });
}


/**
 * A simple example of decoding a 9P message from a file / stdin and printing it in a human readable format.
 */
int main(int argc, const char **argv) {
    styxe::Protocol proc;
    MemoryManager memManager(proc.maxPossibleMessageSize());
    ByteBuffer buffer(memManager.create(proc.maxPossibleMessageSize()));

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::ifstream input(argv[i]);
            if (!input) {
                std::cerr << "Failed to open file: " << std::quoted(argv[i]) << std::endl;
                return EXIT_FAILURE;
            }

            readAndPrintMessage(input, buffer, proc);
            buffer.clear();
        }
    } else {
        readAndPrintMessage(std::cin, buffer, proc);
    }

    return EXIT_SUCCESS;
}

