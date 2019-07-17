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

#include <styxe/9p2000.hpp>
#include <styxe/version.hpp>
#include <styxe/print.hpp>

#include <solace/output_utils.hpp>


#include <iostream>
#include <fstream>
#include <iomanip>

#include <getopt.h>


using namespace Solace;
using namespace styxe;


template<typename T>
struct Quoted {
    Quoted(T const& t, char quote)
        : _quote(quote)
        , _t(t)
    {}

    friend std::ostream& operator<< (std::ostream& ostr, Quoted<T> const& q) {
        return ostr << q._quote << q._t << q._quote;
    }

    char const _quote;
    T const& _t;
};

template<typename T>
Quoted<T> quote(T const& t, char q='\"') { return Quoted<T>(t, q); }



std::ostream& operator<< (std::ostream& ostr, OpenMode mode) {
    byte const op = (mode.mode & 0x03);
    switch (op) {
    case OpenMode::READ:  ostr << "READ"; break;
    case OpenMode::WRITE: ostr << "WRITE"; break;
    case OpenMode::RDWR:  ostr << "RDWR"; break;
    case OpenMode::EXEC:  ostr << "EXEC"; break;
    }

    // Extra modes:
    if (mode.mode & OpenMode::TRUNC)
        ostr << "(TRUNC)";
    if (mode.mode & OpenMode::CEXEC)
        ostr << "(CEXEC)";
    if (mode.mode & OpenMode::RCLOSE)
        ostr << "(RCLOSE)";

    return ostr;
}

std::ostream& operator<< (std::ostream& ostr, Qid const& qid) {
    return ostr << '{'
                << "type: " << static_cast<int>(qid.type) << ", "
                << "ver: "  << qid.version << ", "
                << "path: " << qid.path
                << '}';

}

std::ostream& operator<< (std::ostream& ostr, Stat const& stat) {
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

std::ostream& operator<< (std::ostream& ostr, MessageHeader const& header) {
    bool const isRequest = (static_cast<byte>(header.type) % 2) == 0;
    ostr  << (isRequest ? "→" : "←");

    return ostr << " ["
                << std::setw(5) << header.messageSize
                << "] <" << header.tag << "> "
                << header.type;
}



struct VisitRequest {

    void operator()(Request::Version const& req) {
        std::cout << ": " << req.msize << ' ' << quote(req.version) <<std::endl;
    }

    void operator()(Request::Auth const& req) {
        std::cout << ": " << req.afid << ' ' << quote(req.uname) << ' ' << quote(req.aname) << std::endl;
    }

    void operator()(Request::Attach const& req) {
        std::cout << ": " << req.fid << ' ' << req.afid << ' ' << quote(req.uname) << ' ' << quote(req.aname) << std::endl;
    }

    void operator()(Request::Clunk const& req) {
        std::cout << ": " << req.fid << std::endl;
    }

    void operator()(Request::Flush const& req) {
        std::cout << ": " << req.oldtag << std::endl;
    }

    void operator()(Request::Open const& req) {
        std::cout << ": " << req.fid << ' ' << req.mode << std::endl;
    }

    void operator()(Request::Create const& req) {
        std::cout << ": " << req.fid << ' ' << quote(req.name) << ' ' << req.perm << ' ' << req.mode << std::endl;
    }


    void operator()(Request::Read const& req) {
        std::cout << ": " << req.fid << ' ' << req.offset << ' ' << req.count << std::endl;
    }

    void operator()(Request::Write const& req) {
        std::cout << ": " << req.fid << ' '
                  << req.offset << ' '
                  << req.data.size()
                  << " DATA[" << req.data << ']'
                  << std::endl;
    }

    void operator()(Request::Remove const& req) {
        std::cout << ": " << req.fid << std::endl;
    }

    void operator()(Request::StatRequest const& req) {
        std::cout << ": " << req.fid << std::endl;
    }

    void operator()(Request::WStat const& req) {
        std::cout << ": " << req.fid << ' ' << req.stat << std::endl;
    }

    void operator()(Request::Walk const& req) {
        std::cout << ": "
                  << req.fid << ' '
                  << req.newfid << ' '
                  << req.path.getComponentsCount() << ' ' << '[';

        for (Path::size_type i = 0; i < req.path.getComponentsCount(); ++i) {
            std::cout << quote(req.path.getComponent(i));
            if (i + 1 != req.path.getComponentsCount())
                std::cout << ", ";
        }

        std::cout << ']' << std::endl;
    }

    void operator()(Request_9P2000E::Session const& req) {
        std::cout << ": " << wrapMemory(req.key) << std::endl;
    }

    void operator()(Request_9P2000E::SRead const& req) {
        std::cout << ": " << req.fid << ' '
                  << quote(req.path.toString())
                  << std::endl;
    }

    void operator()(Request_9P2000E::SWrite const& req) {
        std::cout << ": " << req.fid << ' '
                  << quote(req.path.toString())
                  << " DATA[" << req.data << "]"
                  << std::endl;
    }
};


struct VisitResponse {

    void operator()(Response::Version const& resp) {
        std::cout << ": " << resp.msize << ' ' << quote(resp.version) <<std::endl;
    }
    void operator()(Response::Auth const& resp) {
        std::cout << ": " << resp.qid << std::endl;
    }

    void operator()(Response::Attach const& resp) {
        std::cout << ": " << resp.qid << std::endl;
    }

    void operator()(Response::Error const& resp) {
        std::cout << ": " << quote(resp.ename) <<std::endl;
    }

    void operator()(Response::Flush const& /*res*/) {
        std::cout << std::endl;
    }

    void operator()(Response::Walk const& resp) {
        std::cout << ": " << resp.nqids
                  << " [";
         for (uint i = 0; i < resp.nqids; ++i) {
             std::cout << resp.qids[i];
             if (i + 1 != resp.nqids)
                std::cout << ", ";
         }
         std::cout << ']' << std::endl;
    }

    void operator()(Response::Open const& resp) {
        std::cout << ": " << resp.qid << " " << resp.iounit << std::endl;
    }

    void operator()(Response::Create const& resp) {
        std::cout << ": " << resp.qid << " " << resp.iounit << std::endl;
    }


    void operator()(Response::Read const& resp) {
        std::cout << ": " << resp.data.size()
                  << " DATA[" << resp.data << ']'
                  << std::endl;
    }
    void operator()(Response::Write const& resp) {
        std::cout << ": " << resp.count
                  << std::endl;
    }

    void operator()(Response::Clunk& /*res*/) { std::cout << std::endl; }
    void operator()(Response::Remove& /*res*/) { std::cout << std::endl; }

    void operator()(Response::Stat const& stat) {
        std::cout << ": " << stat.data << std::endl;
    }

    void operator()(Response::WStat& /*res*/) { std::cout << std::endl; }
    void operator()(Response_9P2000E::Session& /*res*/) { std::cout << std::endl; }
};


void readAndPrintMessage(std::istream& in, MemoryResource& buffer, Parser& proc) {

    // Message header is fixed size - so it is safe to attempt to read it.
    in.read(buffer.view().dataAs<char>(), headerSize());

    auto reader = ByteReader{buffer};
    reader.limit(in.gcount());

    proc.parseMessageHeader(reader)
            .then([&](MessageHeader&& header) {
                std::cout << header;

                in.read(buffer.view().dataAs<char>(), header.payloadSize());
                reader.rewind()
                        .limit(in.gcount());

                bool const isRequest = (static_cast<byte>(header.type) % 2) == 0;
                if (isRequest) {
                    proc.parseRequest(header, reader)
                            .then([](RequestMessage&& reqMsg) { std::visit(VisitRequest{}, reqMsg); });
                } else {
                    proc.parseResponse(header, reader)
                            .then([](ResponseMessage&& resp) { std::visit(VisitResponse{}, resp); });
                }
            })
            .orElse([](Error&& err) {
                std::cerr << "Error parsing message header: " << err.toString() << std::endl;
            });
}


/// Print app usage
int usage(const char* progname) {
    std::cout << "Usage: " << progname
              << "[-m <size>] "
              << "[-p <version>] "
              << "[-h] "
              << " [FILE]..."
              << std::endl;

    std::cout << "Read 9P2000 message and display it\n\n"
              << "Options: \n"
              << " -m <size> - Use maximum buffer size for messages [Default: " << kMaxMesssageSize << "]\n"
              << " -p <version> - Use specific protocol version [Default: " << Parser::PROTOCOL_VERSION << "]\n"
              << " -h - Display help and exit\n"
              << std::endl;

    return EXIT_SUCCESS;
}

/**
 * A simple example of decoding a 9P message from a file / stdin and printing it in a human readable format.
 */
int main(int argc, char* const* argv) {
    size_type maxMessageSize = kMaxMesssageSize;
    StringView requiredVersion = Parser::PROTOCOL_VERSION;

    int c;
    while ((c = getopt(argc, argv, "m:p:h")) != -1)
        switch (c) {
        case 'm': {
            int requestedSize = atoi(optarg);
            if (requestedSize <= 0) {
                fprintf(stderr, "Option -%c requires positive interger value.\n", optopt);
                return EXIT_FAILURE;
            }
            maxMessageSize = requestedSize;
        } break;
        case 'p':
            requiredVersion = StringView{optarg};
            break;
        case 'h':
            return usage(argv[0]);
        default:
            return EXIT_FAILURE;
    }

    Parser proc{maxMessageSize, requiredVersion};
    MemoryManager memManager{proc.maxPossibleMessageSize()};
    auto buffer = memManager.allocate(proc.maxPossibleMessageSize());


    if (optind < argc) {
        for (int i = optind; i < argc; ++i) {
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
