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

#include <styxe/styxe.hpp>

#include <solace/output_utils.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

#include <getopt.h>


using namespace Solace;
using namespace styxe;


template<typename T>
struct Quoted {
	constexpr Quoted(T const& t, char quote) noexcept
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
constexpr Quoted<T> quote(T const& t, char q='\"') noexcept { return Quoted<T>(t, q); }


struct NamedField {
	const char* name;

	friend std::ostream& operator<< (std::ostream& ostr, NamedField const& q) {
		return ostr << ' ' << q.name << '=';
	}
};


/// IO manipulator
constexpr NamedField field(const char* name) noexcept {
	return NamedField{name};
}


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

std::ostream& operator<< (std::ostream& ostr, WalkPath const& path) {
	WalkPath::size_type i = 0;
	WalkPath::size_type const count = path.size();
	for (auto pathSegment : path) {
		ostr << pathSegment;
		if (i + 1 != count)
			ostr << '/';
		++i;
	}

	return ostr;
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

std::ostream& operator<< (std::ostream& ostr, _9P2000L::DirEntry const& ent) {
	return ostr << '{'
				<< "qid: " << ent.qid << ", "
				<< "offset: " << ent.offset << ", "
				<< "type: " << static_cast<int>(ent.type) << ", "
				<< "name: \"" << ent.name << "\""
				<< '}';
}


void
printHeader(std::ostream& ostr, ParserBase const& parser, MessageHeader const& header) {
	bool const isRequest = ((header.type % 2) == 0);
    ostr  << (isRequest ? "→" : "←");

	ostr << " ["
		 << std::setw(5) << header.messageSize
		 << "] <" << header.tag << "> "
		 << parser.messageName(header.type);
}



struct VisitRequest {

    void operator()(Request::Version const& req) {
		std::cout << ':'
				  << field("msize") << req.msize
				  << field("version") << req.version;
    }

    void operator()(Request::Auth const& req) {
		std::cout << ':'
				  << field("afid") << req.afid
				  << field("uname") << req.uname
				  << field("aname") << req.aname;
    }

    void operator()(Request::Attach const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("afid") << req.afid
				  << field("uname") << req.uname
				  << field("aname") << req.aname;
    }

    void operator()(Request::Clunk const& req) {
		std::cout << ':'
				  << field("fid") << req.fid;
    }

    void operator()(Request::Flush const& req) {
		std::cout << ':'
				  << field("oldtag") << req.oldtag;
    }

    void operator()(Request::Open const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("mode") << req.mode;
    }

    void operator()(Request::Create const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("name") << req.name
				  << field("perm") << req.perm
				  << field("mode") << req.mode;
    }


    void operator()(Request::Read const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("offset") << req.offset
				  << field("count") << req.count;
    }

    void operator()(Request::Write const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("offset") << req.offset
				  << field("data") << req.data;
    }

    void operator()(Request::Remove const& req) {
		std::cout << ':'
				  << field("fid") << req.fid;
    }

	void operator()(Request::Stat const& req) {
		std::cout << ':'
				  << field("fid") << req.fid;
    }

    void operator()(Request::WStat const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("stat") << req.stat;
    }

    void operator()(Request::Walk const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("newfid") << req.newfid
				  << field("path") << req.path;
    }


	void operator()(_9P2000U::Request::Auth const& req) {
		operator() (static_cast<Request::Auth const&>(req));
		std::cout
				<< field("n_uname") << req.n_uname;

	}
	void operator()(_9P2000U::Request::Attach const& req) {
		operator() (static_cast<Request::Attach const&>(req));
		std::cout
				<< field("n_uname") << req.n_uname;
	}

	void operator()(_9P2000U::Request::Create const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("name") << req.name
				  << field("perm") << req.perm
				  << field("mode") << req.mode
				  << field("extension") << req.extension;
	}

	void operator()(_9P2000U::Request::WStat const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("stat") << req.stat;
	}


	void operator()(_9P2000E::Request::Session const& req) {
		std::cout << ':' << wrapMemory(req.key);
    }

	void operator()(_9P2000E::Request::ShortRead const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("path") << req.path;
    }

	void operator()(_9P2000E::Request::ShortWrite const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("path") << req.path
				  << field("data") << req.data;
    }

	void operator()(_9P2000L::Request::StatFS const& req) {
		std::cout << ':'
				  << field("fid") << req.fid;
	}

	void operator()(_9P2000L::Request::LOpen const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("flags") << req.flags;
	}

	void operator()(_9P2000L::Request::LCreate const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("name") << req.name
				  << field("flags") << req.flags
				  << field("mode") << req.mode
				  << field("gid") << req.gid;
	}

	void operator()(_9P2000L::Request::Symlink const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("name") << req.name
				  << field("symtgt") << req.symtgt
				  << field("gid") << req.gid;
	}

	void operator()(_9P2000L::Request::MkNode const& req) {
		std::cout << ':'
				  << field("dfid") << req.dfid
				  << field("name") << req.name
				  << field("mode") << req.mode
				  << field("major") << req.major
				  << field("minor") << req.minor
				  << field("gid") << req.gid;
	}

	void operator()(_9P2000L::Request::Rename const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("dfid") << req.dfid
				  << field("name") << req.name;
	}

	void operator()(_9P2000L::Request::ReadLink const& req) {
		std::cout << ':'
				  << field("fid") << req.fid;
	}

	void operator()(_9P2000L::Request::GetAttr const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("request_mask") << req.request_mask;
	}

	void operator()(_9P2000L::Request::SetAttr const& req) {
		std::cout << ':'
					 << field("fid") << req.fid
					 << field("valid") << req.valid
					 << field("mode") << req.mode
					 << field("uid") << req.uid
					 << field("gid") << req.gid
					 << field("size") << req.size

					 << field("atime_sec") << req.atime_sec
					 << field("atime_nsec") << req.atime_nsec
					 << field("mtime_sec") << req.mtime_sec
					 << field("mtime_nsec") << req.mtime_nsec;
	}

	void operator()(_9P2000L::Request::XAttrWalk const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("newfid") << req.newfid
				  << field("name") << req.name;
	}

	void operator()(_9P2000L::Request::XAttrCreate const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("name") << req.name
				  << field("attr_size") << req.attr_size
				  << field("flags") << req.flags;
	}

	void operator()(_9P2000L::Request::ReadDir const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("offset") << req.offset
				  << field("count") << req.count;
	}

	void operator()(_9P2000L::Request::FSync const& req) {
		std::cout << ':'
				  << field("fid") << req.fid;
	}

	void operator()(_9P2000L::Request::Lock const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("type") << req.type
				  << field("flags") << req.flags
				  << field("start") << req.start
				  << field("length") << req.length
				  << field("proc_id") << req.proc_id
				  << field("client_id") << req.client_id;
	}

	void operator()(_9P2000L::Request::GetLock const& req) {
		std::cout << ':'
				  << field("fid") << req.fid
				  << field("type") << req.type
				  << field("start") << req.start
				  << field("length") << req.length
				  << field("proc_id") << req.proc_id
				  << field("client_id") << req.client_id;
	}

	void operator()(_9P2000L::Request::Link const& req) {
		std::cout << ':'
				  << field("dfid") << req.dfid
				  << field("fid") << req.fid
				  << field("name") << req.name;
	}

	void operator()(_9P2000L::Request::MkDir const& req) {
		std::cout << ':'
				  << field("dfid") << req.dfid
				  << field("name") << req.name
				  << field("mode") << req.mode
				  << field("gid") << req.gid;
	}

	void operator()(_9P2000L::Request::RenameAt const& req) {
		std::cout << ':'
				  << field("olddirfid") << req.olddirfid
				  << field("oldname") << req.oldname
				  << field("newdirfid") << req.newdirfid
				  << field("newname") << req.newname;
	}

	void operator()(_9P2000L::Request::UnlinkAt const& req) {
		std::cout << ':'
				  << field("dfid") << req.dfid
				  << field("name") << req.name
				  << field("flags") << req.flags;
	}

};


struct VisitResponse {

    void operator()(Response::Version const& resp) {
		std::cout << ':'
				  << field("msize") << resp.msize
				  << field("version") << resp.version;
	}

	void operator()(Response::Auth const& resp) {
		std::cout << ':'
				  << field("qid") << resp.qid;
    }

    void operator()(Response::Attach const& resp) {
		std::cout << ':'
				  << field("qid") << resp.qid;
    }

    void operator()(Response::Error const& resp) {
		std::cout << ':'
				  << field("ename") << quote(resp.ename);
    }

	void operator()(Response::Flush const& /*res*/) noexcept { }

    void operator()(Response::Walk const& resp) {
		std::cout << ':'
				  << resp.nqids
                  << " [";

		const auto nqids = resp.nqids;
		size_t i = 0;
		for (auto const& qid : resp.qids) {
			std::cout << qid;
			if (i + 1 != nqids)
			std::cout << ", ";
		}
		 std::cout << ']';
    }

    void operator()(Response::Open const& resp) {
		std::cout << ':'
				  << field("qid") << resp.qid
				  << field("iounit") << resp.iounit;
	}

    void operator()(Response::Create const& resp) {
		std::cout << ':'
				  << field("qid") << resp.qid
				  << field("iounit") << resp.iounit;
	}


    void operator()(Response::Read const& resp) {
		std::cout << ':'
				  << field("data") << resp.data;
    }
    void operator()(Response::Write const& resp) {
		std::cout << ':'
				  << field("count") << resp.count;
    }

	void operator()(Response::Clunk& /*res*/) noexcept {  }
	void operator()(Response::Remove& /*res*/) noexcept { }

	void operator()(Response::Stat const& resp) {
		std::cout << ':'
				  << field("size") << resp.dummySize
				  << field("data") << resp.data;
	}

	void operator()(Response::WStat const& /*res*/) noexcept { }

	void operator()(_9P2000U::Response::Error const& resp) {
		std::cout << ':'
				  << field("ename") << resp.ename
				  << field("errcode") << resp.errcode;
	}

	void operator()(_9P2000U::Response::Stat const& resp) {
		std::cout << ':'
				  << field("size") << resp.dummySize
				  << field("data") << resp.data;

	}

	void operator()(_9P2000E::Response::Session const& /*res*/) noexcept { }

	void operator()(_9P2000E::Response::ShortRead const& resp) {
		std::cout << ':'
				  << resp.data;
	}

	void operator()(_9P2000E::Response::ShortWrite const& resp) {
		std::cout << ':'
				  << field("count") << resp.count;
	}

	void operator()(_9P2000L::Response::LError const& resp) {
		std::cout << ':'
				  << field("ecode") << resp.ecode;

	}
	void operator()(_9P2000L::Response::StatFS const& resp) {
		std::cout << ':'
				  << field("type") << resp.type
				  << field("bsize") << resp.bsize
				  << field("blocks") << resp.blocks
				  << field("bfree") << resp.bfree
				  << field("bavail") << resp.bavail
				  << field("files") << resp.files
				  << field("ffree") << resp.ffree
				  << field("fsid") << resp.fsid
				  << field("namelen") << resp.namelen;
	}

	// Following two methods are not required as the content of Response::Open and Response::LOpen are the same
//	void operator()(_9P2000L::Response::LOpen const& resp);
//	void operator()(_9P2000L::Response::LCreate const& resp);

	void operator()(_9P2000L::Response::Symlink const& resp) {
		std::cout << ':'
				  << field("qid") << resp.qid;
	}

	void operator()(_9P2000L::Response::MkNode const& resp) {
		std::cout << ':'
				  << field("qid") << resp.qid;

	}

	void operator()(_9P2000L::Response::Rename const&) noexcept {}

	void operator()(_9P2000L::Response::ReadLink const& resp) {
		std::cout << ':'
				  << field("target") << resp.target;
	}

	void operator()(_9P2000L::Response::GetAttr const& resp) {
		std::cout << ':'
				  << field("valid") << resp.valid
				  << field("qid") << resp.qid
				  << field("mode") << resp.mode
				  << field("uid") << resp.uid
				  << field("gid") << resp.gid
				  << field("nlink") << resp.nlink
				  << field("rdev") << resp.rdev
				  << field("size") << resp.size
				  << field("blksize") << resp.blksize
				  << field("blocks") << resp.blocks
				  << field("atime_sec") << resp.atime_sec
				  << field("atime_nsec") << resp.atime_nsec
				  << field("mtime_sec") << resp.mtime_sec
				  << field("mtime_nsec") << resp.mtime_nsec
				  << field("ctime_sec") << resp.ctime_sec
				  << field("ctime_nsec") << resp.ctime_nsec
				  << field("btime_sec") << resp.btime_sec
				  << field("btime_nsec") << resp.btime_nsec
				  << field("gen") << resp.gen
				  << field("data_version") << resp.data_version;
	}

	void operator()(_9P2000L::Response::SetAttr const&) noexcept {}

	void operator()(_9P2000L::Response::XAttrWalk const& resp) {
		std::cout << ':'
				  << field("size") << resp.size;
	}

	void operator()(_9P2000L::Response::XAttrCreate const&) noexcept { }

	void operator()(_9P2000L::Response::ReadDir const& resp) {
		std::cout << ':'
				  << '[';
//				  << field("data") << resp.data;

		_9P2000L::DirEntryReader dirReader{resp.data};
		for (auto const& ent : dirReader) {
			std::cout << ent;
		}

		std::cout << ']';
	}

	void operator()(_9P2000L::Response::FSync const&) noexcept { }

	void operator()(_9P2000L::Response::Lock const& resp) {
		std::cout << ':'
				  << field("status") << static_cast<uint32>(resp.status);
	}

	void operator()(_9P2000L::Response::GetLock const& resp) {
		std::cout << ':'
				  << field("type") << static_cast<uint32>(resp.type)
				  << field("start") << resp.start
				  << field("length") << resp.length
				  << field("proc_id") << resp.proc_id
				  << field("client_id") << quote(resp.client_id);
	}

	void operator()(_9P2000L::Response::Link const&) noexcept { }

	void operator()(_9P2000L::Response::MkDir const& resp) {
		std::cout << ':'
					 << field("qid") << resp.qid;
	}
	void operator()(_9P2000L::Response::RenameAt const&) noexcept { }
	void operator()(_9P2000L::Response::UnlinkAt const&) noexcept { }

};


void readAndPrintMessage(std::istream& in, MemoryResource& buffer, RequestParser& tparser, ResponseParser& rparser) {
    // Message header is fixed size - so it is safe to attempt to read it.
    in.read(buffer.view().dataAs<char>(), headerSize());

    auto reader = ByteReader{buffer};
    reader.limit(in.gcount());

	auto headerParser = UnversionedParser{narrow_cast<size_type>(buffer.size())};
	headerParser.parseMessageHeader(reader)
            .then([&](MessageHeader&& header) {
                in.read(buffer.view().dataAs<char>(), header.payloadSize());
                reader.rewind()
                        .limit(in.gcount());

				bool const isRequest = (header.type % 2) == 0;
                if (isRequest) {
					tparser.parseRequest(header, reader)
							.then([&tparser, &header](RequestMessage&& reqMsg) {
								printHeader(std::cout, tparser, header);
								std::visit(VisitRequest{}, reqMsg);
							})
							.orElse([](Error&& err) {
								std::cerr << "Error parsing message: " << err.toString() << std::endl;
							});

                } else {
					rparser.parseResponse(header, reader)
							.then([&rparser, &header](ResponseMessage&& resp) {
								printHeader(std::cout, rparser, header);
								std::visit(VisitResponse{}, resp);
							})
							.orElse([](Error&& err) {
								std::cerr << "Error parsing message: " << err.toString() << std::endl;
							});
				}

				std::cout << std::endl;
            })
            .orElse([](Error&& err) {
                std::cerr << "Error parsing message header: " << err.toString() << std::endl;
            });
}


/// Print app usage
int usage(const char* progname, size_type defaultMessageSize, StringView defaultVersion) {
    std::cout << "Usage: " << progname
              << "[-m <size>] "
              << "[-p <version>] "
              << "[-h] "
              << " [FILE]..."
              << std::endl;

    std::cout << "Read 9P2000 message and display it\n\n"
              << "Options: \n"
			  << " -m <size> - Use maximum buffer size for messages [Default: " << defaultMessageSize << "]\n"
			  << " -p <version> - Use specific protocol version [Default: " << defaultVersion<< "]\n"
              << " -h - Display help and exit\n"
              << std::endl;

    return EXIT_SUCCESS;
}

/**
 * A simple example of decoding a 9P message from a file / stdin and printing it in a human readable format.
 */
int main(int argc, char* const* argv) {
	size_type maxMessageSize = kMaxMessageSize;
	StringView requiredVersion = _9P2000U::kProtocolVersion;

    int c;
	while ((c = getopt(argc, argv, "m:p:h")) != -1) {
		switch (c) {
		case 'm': {
			int requestedSize = atoi(optarg);
			if (requestedSize <= 0) {
				fprintf(stderr, "Option -%c requires positive interger value.\n", optopt);
				return EXIT_FAILURE;
			}
			// Note: safe to cast due to the check performed above
			maxMessageSize = static_cast<size_type>(requestedSize);
		} break;
		case 'p':
			requiredVersion = StringView{optarg};
			break;
		case 'h':
			return usage(argv[0], maxMessageSize, requiredVersion);
		default:
			return EXIT_FAILURE;
		}
	}

	auto maybeReqParser = createRequestParser(requiredVersion, maxMessageSize);
	if (!maybeReqParser) {
		std::cerr << "Failed to create parser: " << maybeReqParser.getError() << std::endl;
		return EXIT_FAILURE;
	}

	auto maybeRespParser = createResponseParser(requiredVersion, maxMessageSize);
	if (!maybeRespParser) {
		std::cerr << "Failed to create parser: " << maybeRespParser.getError() << std::endl;
		return EXIT_FAILURE;
	}

	auto& tparser = *maybeReqParser;
	auto& rparser = *maybeRespParser;

	MemoryManager memManager{maxMessageSize};
	auto memoryResource = memManager.allocate(maxMessageSize);
	if (!memoryResource) {
		std::cerr << "Feiled to allocate memory for a buffer: " << memoryResource.getError() << std::endl;
		return EXIT_FAILURE;
	}

	auto& buffer = memoryResource.unwrap();
    if (optind < argc) {
        for (int i = optind; i < argc; ++i) {
            std::ifstream input(argv[i]);
            if (!input) {
                std::cerr << "Failed to open file: " << std::quoted(argv[i]) << std::endl;
                return EXIT_FAILURE;
            }

			readAndPrintMessage(input, buffer, tparser, rparser);
        }
    } else {
		readAndPrintMessage(std::cin, buffer, tparser, rparser);
    }

    return EXIT_SUCCESS;
}
