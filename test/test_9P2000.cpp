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
/*******************************************************************************
 * libstyxe Unit Test Suit
 * @file: test/test_9P2000.cpp
 * @author: soultaker
 *
 *******************************************************************************/
#include "styxe/messageWriter.hpp"
#include "styxe/messageParser.hpp"

#include "testHarnes.hpp"

#include <solace/output_utils.hpp>


using namespace Solace;
using namespace styxe;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 9P2000 Message parsing test suit
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class P9Messages : public TestHarnes {
protected:

    template<typename RequestType>
    Result<RequestType, Error>
	getRequestOrFail() {
		ByteReader reader{_writer.viewWritten()};

		auto maybeParser = createRequestParser(kProtocolVersion, kMaxMessageSize);
		if (!maybeParser) {
			logFailure(maybeParser.getError());
			return maybeParser.moveError();
		}

		auto& proc = *maybeParser;
		auto constexpr expectType = messageCodeOf<RequestType>();
		auto headerParser = UnversionedParser{kMaxMessageSize};
		return headerParser.parseMessageHeader(reader)
				.then([](MessageHeader header) {
					return (header.type != expectType)
							? Result<MessageHeader, Error>{getCannedError(CannedError::UnsupportedMessageType)}
							: Result<MessageHeader, Error>{types::okTag, std::move(header)};
                })
				.then([&reader, &proc](MessageHeader&& header) {
					return proc.parseRequest(header, reader);
                })
				.then([](RequestMessage&& msg) -> Result<RequestType, Error> {
					bool const isType = std::holds_alternative<RequestType>(msg);

                    if (!isType) {
                        []() { FAIL() << "Parsed request is on unexpected type"; } ();
						return getCannedError(CannedError::UnsupportedMessageType);
                    }

                    return Ok<RequestType>(std::get<RequestType>(std::move(msg)));
                })
				.mapError([this](Error&& e) -> Error {
					logFailure(e);

                    return e;
                });
    }


    template<typename ResponseType>
    Result<ResponseType, Error>
	getResponseOrFail() {
		ByteReader reader{_writer.viewWritten()};

		auto maybeParser = createResponseParser(kProtocolVersion, kMaxMessageSize);
		if (!maybeParser) {
			logFailure(maybeParser.getError());
			return maybeParser.moveError();
		}

		auto& proc = *maybeParser;

		auto constexpr expectType = messageCodeOf<ResponseType>();
		auto headerParser = UnversionedParser{kMaxMessageSize};
		return headerParser.parseMessageHeader(reader)
				.then([](MessageHeader header) {
					return (header.type == expectType)
							? Result<MessageHeader, Error>{types::okTag, std::move(header)}
							: Result<MessageHeader, Error>{types::errTag, getCannedError(CannedError::UnsupportedMessageType)};
                })
				.then([&reader, &proc](MessageHeader&& header) {
					return proc.parseResponse(header, reader);
                })
				.then([](ResponseMessage&& msg) -> Result<ResponseType, Error> {
					bool const isType = std::holds_alternative<ResponseType>(msg);

                    if (!isType) {
                        []() { FAIL() << "Parsed request is on unexpected type"; } ();
						return getCannedError(CannedError::UnsupportedMessageType);
                    }

                    return Ok<ResponseType>(std::get<ResponseType>(std::move(msg)));
                })
				.mapError([this](Error&& e) {
					logFailure(e);

                    return e;
                });
    }

};




TEST_F(P9Messages, createVersionRequest) {
	auto const testVersion = kProtocolVersion;

	auto writer = RequestWriter{_writer, kNoTag};
	writer << Request::Version{kMaxMessageSize, testVersion};

	getRequestOrFail<Request::Version>()
			.then([testVersion](Request::Version const& request) {
				EXPECT_EQ(kMaxMessageSize, request.msize);
                EXPECT_EQ(testVersion, request.version);
            });
}

TEST_F(P9Messages, createVersionResponse) {
	auto writer = ResponseWriter{_writer, kNoTag};
	writer << Response::Version{718, "9Pe"};

	getResponseOrFail<Response::Version>()
            .then([](Response::Version const& response) {
                ASSERT_EQ(718, response.msize);
                ASSERT_EQ("9Pe", response.version);
            });
}

TEST_F(P9Messages, parseVersionResponse) {
	styxe::Encoder encoder{_writer};
    // Set declared message size to be more then negotiated message size
	encoder << makeHeaderWithPayload(asByte(MessageType::RVersion), 1, sizeof(int32) + sizeof(int16) + 2)
			<< uint32{512}
			<< StringLiteral("9P");

	getResponseOrFail<Response::Version>()
            .then([](Response::Version const& response) {
                ASSERT_EQ(512, response.msize);
                ASSERT_EQ("9P", response.version);
            });
}


TEST_F(P9Messages, createAuthRequest) {
	RequestWriter writer{_writer};
	writer <<  Request::Auth{312, "User mcUsers", "Somewhere near"};

	getRequestOrFail<Request::Auth>()
            .then([](Request::Auth&& request) {
                ASSERT_EQ(312, request.afid);
                ASSERT_EQ("User mcUsers", request.uname);
                ASSERT_EQ("Somewhere near", request.aname);
            });
}


TEST_F(P9Messages, createAuthResponse) {
    auto const qid = Qid {
			8187,
            71,
			17
    };

	ResponseWriter writer{_writer, 1};
	writer << Response::Auth{qid};

	getResponseOrFail<Response::Auth>()
            .then([qid](Response::Auth&& response) {
                ASSERT_EQ(qid, response.qid);
            });
}


TEST_F(P9Messages, parseAuthResponse) {
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RAuth), 1, 13)
			<< byte{13}	      // Qid.type
			<< uint32{91}	  // Qid.version
			<< uint64{4451};  // Qid.path

	getResponseOrFail<Response::Auth>()
            .then([](Response::Auth&& response) {
                EXPECT_EQ(13, response.qid.type);
                EXPECT_EQ(91, response.qid.version);
				EXPECT_EQ(4451, response.qid.path);
            });
}

// No such thing as error request!

TEST_F(P9Messages, createErrorResponse) {
    auto const testError = StringLiteral{"Something went right :)"};
	ResponseWriter writer{_writer, 3};
	writer << Response::Error{testError};

	getResponseOrFail<Response::Error>()
            .then([testError](Response::Error&& response) {
                ASSERT_EQ(testError, response.ename);
            });
}

TEST_F(P9Messages, createPartialErrorResponse) {
	auto const testError = StringLiteral{"Something went right :)"};
	ResponseWriter writer{_writer, 3};
	writer << Response::Partial::Error{}
		   << "Something "
		   << "went "
		   << "right :)";

	getResponseOrFail<Response::Error>()
			.then([testError](Response::Error&& response) {
				ASSERT_EQ(testError, response.ename);
			});
}


TEST_F(P9Messages, parseErrorResponse) {
	auto const expectedErrorMessage = StringLiteral{"All good!"};

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RError), 1, styxe::protocolSize(expectedErrorMessage))
			<< static_cast<StringView>(expectedErrorMessage);

	getResponseOrFail<Response::Error>()
            .then([expectedErrorMessage](Response::Error&& response) {
                EXPECT_EQ(expectedErrorMessage, response.ename);
            });
}


TEST_F(P9Messages, createFlushRequest) {
	RequestWriter writer{_writer};
	writer << Request::Flush{7711};

	getRequestOrFail<Request::Flush>()
            .then([](Request::Flush&& request) {
                ASSERT_EQ(7711, request.oldtag);
            });
}


TEST_F(P9Messages, createFlushResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::Flush{};

	getResponseOrFail<Response::Flush>();
}


TEST_F(P9Messages, parseFlushResponse) {
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RFlush), 1, 0);

	getResponseOrFail<Response::Flush>();
}


TEST_F(P9Messages, createAttachRequest) {
	RequestWriter writer{_writer};
	writer << Request::Attach{3310, 1841, "McFace", "close to u"};

	getRequestOrFail<Request::Attach>()
            .then([](Request::Attach&& request) {
                ASSERT_EQ(3310, request.fid);
                ASSERT_EQ(1841, request.afid);
                ASSERT_EQ("McFace", request.uname);
                ASSERT_EQ("close to u", request.aname);
            });
}

TEST_F(P9Messages, createAttachResponse) {
    auto const qid = Qid {
			7771,
            91,
			3
    };

	ResponseWriter writer{_writer, 1};
	writer << Response::Attach{qid};

	getResponseOrFail<Response::Attach>()
            .then([qid](Response::Attach&& response) {
                ASSERT_EQ(qid, response.qid);
            });
}

TEST_F(P9Messages, parseAttachResponse) {
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RAttach), 1, 13)
			<< byte{81}			// Qid.type
			<< uint32{3}		// Qid.version
			<< uint64{1049};    // Qid.path

	getResponseOrFail<Response::Attach>()
            .then([](Response::Attach&& response) {
                EXPECT_EQ(81, response.qid.type);
                EXPECT_EQ(3, response.qid.version);
                EXPECT_EQ(1049, response.qid.path);
            });
}


TEST_F(P9Messages, createOpenRequest) {
	RequestWriter writer{_writer};
	writer << Request::Open{517, OpenMode::RDWR};

	getRequestOrFail<Request::Open>()
            .then([](Request::Open&& request) {
                ASSERT_EQ(517, request.fid);
                ASSERT_EQ(OpenMode::RDWR, request.mode);
            });
}


TEST_F(P9Messages, createOpenResponse) {
	auto const qid = Qid {881, 13, 23};

	ResponseWriter writer{_writer, 1};
	writer << Response::Open{qid, 817};

	getResponseOrFail<Response::Open>()
            .then([qid](Response::Open&& response) {
                ASSERT_EQ(qid, response.qid);
                ASSERT_EQ(817, response.iounit);
            });
}

TEST_F(P9Messages, parseOpenResponse) {
	auto const qid = Qid {4173, 71, 2};
    size_type const iounit = 998;

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::ROpen), 1,
									 styxe::protocolSize(qid) +
									 styxe::protocolSize(iounit))
			<< qid
			<< iounit;

	getResponseOrFail<Response::Open>()
            .then([qid, iounit](Response::Open&& response) {
                EXPECT_EQ(qid, response.qid);
                EXPECT_EQ(iounit, response.iounit);
            });
}



TEST_F(P9Messages, createCreateRequest) {
	RequestWriter writer{_writer};
	writer << Request::Create{1734, "mcFance", 11, OpenMode::EXEC};

	getRequestOrFail<Request::Create>()
            .then([](Request::Create&& request) {
                ASSERT_EQ(1734, request.fid);
                ASSERT_EQ("mcFance", request.name);
                ASSERT_EQ(11, request.perm);
                ASSERT_EQ(OpenMode::EXEC, request.mode);
            });
}

TEST_F(P9Messages, createCreateResponse) {
    auto const qid = Qid {
			323,
            8,
			13
    };

	ResponseWriter writer{_writer, 1};
	writer << Response::Create{qid, 718};

	getResponseOrFail<Response::Create>()
            .then([qid](Response::Create&& response) {
                ASSERT_EQ(qid, response.qid);
                ASSERT_EQ(718, response.iounit);
            });
}

TEST_F(P9Messages, parseCreateResponse) {
    auto const qid = Qid {
			323,
            8,
			13
    };
    size_type const iounit = 778;

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RCreate), 1,
									 styxe::protocolSize(qid) +
									 styxe::protocolSize(iounit))
			<< qid
			<< iounit;

	getResponseOrFail<Response::Create>()
            .then([qid](Response::Create&& response) {
                EXPECT_EQ(qid, response.qid);
                EXPECT_EQ(778, response.iounit);
            });
}


TEST_F(P9Messages, createReadRequest) {
	RequestWriter writer{_writer};
	writer << Request::Read{7234, 18, 772};

	getRequestOrFail<Request::Read>()
            .then([](Request::Read&& request) {
                ASSERT_EQ(7234, request.fid);
                ASSERT_EQ(18, request.offset);
                ASSERT_EQ(772, request.count);
            });
}


TEST_F(P9Messages, createReadResponse) {
	char const content[] = "Good news everyone!";
    auto data = wrapMemory(content);
	ResponseWriter writer{_writer, 1};

	Response::Read resp{};
	resp.data = data;
	writer << resp;

	getResponseOrFail<Response::Read>()
            .then([data](Response::Read&& response) {
                ASSERT_EQ(data, response.data);
            });
}


TEST_F(P9Messages, createPartialReadResponse) {
	char const content[] = "Good news no-one :)";
	auto data = wrapMemory(content);
	ResponseWriter writer{_writer, 1};
	writer << Response::Partial::Read{}
		   << data;

	getResponseOrFail<Response::Read>()
			.then([data](Response::Read&& response) {
				ASSERT_EQ(data, response.data);
			});
}


TEST_F(P9Messages, parseReadResponse) {
	auto const messageText = StringLiteral{"This is a very important data d-_^b"};
	auto const messageData = messageText.view();
	uint32 const dataLen = messageData.size();

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RRead), 1, sizeof(uint32) + dataLen)
			<< messageData;

	getResponseOrFail<Response::Read>()
            .then([dataLen, messageData](Response::Read&& response) {
                EXPECT_EQ(dataLen, response.data.size());
				EXPECT_EQ(response.data, messageData);
            });
}


TEST_F(P9Messages, createWriteRequest) {
	char const messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

	RequestWriter writer{_writer, 1};
	writer << Request::Write{15927, 98, data};

	getRequestOrFail<Request::Write>()
            .then([data](Request::Write&& request) {
                ASSERT_EQ(15927, request.fid);
                ASSERT_EQ(98, request.offset);
                ASSERT_EQ(data, request.data);
            });
}

TEST_F(P9Messages, createPartialWriteRequest) {
	char const messageData[] = "This is a very important data BLAH! d-_^b";
	auto data = wrapMemory(messageData);

	RequestWriter writer{_writer};
	writer << Request::Partial::Write{76927, 9898}
		   << data;

	getRequestOrFail<Request::Write>()
			.then([data](Request::Write&& request) {
				ASSERT_EQ(76927, request.fid);
				ASSERT_EQ(9898, request.offset);
				ASSERT_EQ(data, request.data);
			});
}


TEST_F(P9Messages, createWriteResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::Write{71717};

	getResponseOrFail<Response::Write>()
            .then([](Response::Write&& response) {
                ASSERT_EQ(71717, response.count);
            });
}

TEST_F(P9Messages, parseWriteResponse) {
	uint32 const iounit = 81177;
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RWrite), 1, sizeof(int32))
			<< iounit;

	getResponseOrFail<Response::Write>()
			.then([&](Response::Write&& response) {
				EXPECT_EQ(iounit, response.count);
            });
}



TEST_F(P9Messages, createClunkRequest) {
	RequestWriter writer{_writer, 1};
	writer << Request::Clunk{37509};

	getRequestOrFail<Request::Clunk>()
            .then([](Request::Clunk&& request) {
                ASSERT_EQ(37509, request.fid);
            });
}

TEST_F(P9Messages, createClunkResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::Clunk{};

	getResponseOrFail<Response::Clunk>();
}

TEST_F(P9Messages, parseClunkResponse) {
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RClunk), 1, 0);

	getResponseOrFail<Response::Clunk>();
}



TEST_F(P9Messages, createRemoveRequest) {
	RequestWriter writer{_writer, 1};
	writer << Request::Remove{54329};

	getRequestOrFail<Request::Remove>()
            .then([](Request::Remove&& request) {
                ASSERT_EQ(54329, request.fid);
            });
}

TEST_F(P9Messages, createRemoveResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::Remove{};

	getResponseOrFail<Response::Remove>();
}

TEST_F(P9Messages, parseRemoveResponse) {
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RRemove), 1, 0);

	getResponseOrFail<Response::Remove>();
}



TEST_F(P9Messages, createStatRequest) {
	RequestWriter writer{_writer, 1};
	writer << Request::Stat{7872};

	getRequestOrFail<Request::Stat>()
			.then([](Request::Stat&& request) {
                ASSERT_EQ(7872, request.fid);
            });
}

TEST_F(P9Messages, createStatResponse) {
    Stat stat;
    stat.atime = 12;
    stat.dev = 3310;
    stat.gid = "Nice user";
    stat.length = 414;
    stat.mode = 111;
    stat.mtime = 17;
    stat.name = "File McFileface";
    stat.qid.path = 68171;
    stat.qid.type = 7;
    stat.qid.version = 4;
    stat.size = 124;
    stat.type = 3;
    stat.uid = "User McUserface -2";

	ResponseWriter writer{_writer, 1};
	writer << Response::Stat{stat.size, stat};

	getResponseOrFail<Response::Stat>()
            .then([stat](Response::Stat&& response) {
                ASSERT_EQ(stat, response.data);
            });
}

TEST_F(P9Messages, parseStatResponse) {
    Response::Stat statResponse;
    statResponse.dummySize = 1;

    statResponse.data.atime = 21;
    statResponse.data.dev = 8828;
    statResponse.data.gid = "Some user";
    statResponse.data.length = 818177;
    statResponse.data.mode = 111;
    statResponse.data.mtime = 17;
    statResponse.data.name = "File McFileface";
    statResponse.data.qid.path = 61;
    statResponse.data.qid.type = 15;
    statResponse.data.qid.version = 404;
    statResponse.data.type = 1;
    statResponse.data.uid = "User McUserface";
    statResponse.data.size = DirListingWriter::sizeStat(statResponse.data);

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RStat), 1,
									 sizeof(statResponse.dummySize) + styxe::protocolSize(statResponse.data))
			<< statResponse.dummySize
			<< statResponse.data;

	getResponseOrFail<Response::Stat>()
            .then([statResponse](Response::Stat&& response) {
                ASSERT_EQ(statResponse.dummySize, response.dummySize);
                ASSERT_EQ(statResponse.data, response.data);
            });
}


TEST_F(P9Messages, createWStatRequest) {
    Stat stat;
    stat.atime = 21;
    stat.dev = 8828;
    stat.gid = "Other user";
    stat.length = 818177;
    stat.mode = 111;
    stat.mtime = 17;
    stat.name = "la-la McFile";
    stat.qid.path = 61;
    stat.qid.type = 15;
    stat.qid.version = 404;
    stat.size = 124;
    stat.type = 1;
    stat.uid = "Userface McUse";

	RequestWriter writer{_writer};
	writer << Request::WStat{8193, stat};

	getRequestOrFail<Request::WStat>()
            .then([stat](Request::WStat&& request) {
                ASSERT_EQ(8193, request.fid);
                ASSERT_EQ(stat, request.stat);
            });
}


TEST_F(P9Messages, createWStatResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::WStat{};

	getResponseOrFail<Response::WStat>();
}

TEST_F(P9Messages, parseWStatResponse) {
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RWStat), 1, 0);

	getResponseOrFail<Response::WStat>();
}


TEST_F(P9Messages, createWalkRequest) {
	byte buffer[5 + 8 + 2*2];
	ByteWriter pathWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{pathWriter};
	encoder << StringView{"space"}
			<< StringView{"knowhere"};

	RequestWriter writer{_writer};
	writer << Request::Walk{213, 124, WalkPath{2, wrapMemory(buffer)}};

	getRequestOrFail<Request::Walk>()
			.then([](Request::Walk&& request) {
                EXPECT_EQ(213, request.fid);
                EXPECT_EQ(124, request.newfid);
				EXPECT_EQ(2, request.path.size());
				EXPECT_EQ("space", *request.path.begin());
			});
}


TEST_F(P9Messages, createPartialWalkRequest) {
	RequestWriter writer{_writer};
	writer << Request::Partial::Walk{213, 124}
		   << StringView{"space"}
		   << StringView{"knowhere"};

	getRequestOrFail<Request::Walk>()
			.then([](Request::Walk&& request) {
				EXPECT_EQ(213, request.fid);
				EXPECT_EQ(124, request.newfid);
				EXPECT_EQ(2, request.path.size());
				EXPECT_EQ("space", *request.path.begin());
			});
}


TEST_F(P9Messages, createWalkEmptyPathRequest) {
	RequestWriter writer{_writer};
	writer << Request::Walk{7374, 542, WalkPath(0, MemoryView{})};

	getRequestOrFail<Request::Walk>()
            .then([](Request::Walk&& request) {
                ASSERT_EQ(7374, request.fid);
                ASSERT_EQ(542, request.newfid);
                ASSERT_TRUE(request.path.empty());
            });
}


TEST_F(P9Messages, createWalkResponse) {
	Response::Walk message;
	message.nqids = 3;
	message.qids[2].path = 21;
	message.qids[2].version = 117;
	message.qids[2].type = 81;

	ResponseWriter writer{_writer, 1};
	writer << message;

	getResponseOrFail<Response::Walk>()
			.then([&](Response::Walk&& response) {
				ASSERT_EQ(message.nqids, response.nqids);
				ASSERT_EQ(message.qids[2], response.qids[2]);
            });
}

TEST_F(P9Messages, parseWalkResponse) {
	uint16 const nQids = 1;
    auto const qid = Qid {
            87,
            5481,
            17
    };

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RWalk), 1, 2 + 13)
			<< nQids
			<< qid;

	getResponseOrFail<Response::Walk>()
            .then([](Response::Walk&& response) {
				EXPECT_EQ(1, response.nqids);
				EXPECT_EQ(17, response.qids[0].type);
                EXPECT_EQ(5481, response.qids[0].version);
				EXPECT_EQ(87, response.qids[0].path);
            });
}

