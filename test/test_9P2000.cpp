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

#include <solace/output_utils.hpp>

#include <gtest/gtest.h>


using namespace Solace;
using namespace styxe;


inline uint16 operator "" _us(unsigned long long value) {  // NOLINT(runtime/int)
    return static_cast<uint16>(value);
}


void encode9P(ByteWriter& dest, Qid qid) {
    dest.writeLE(qid.type);
    dest.writeLE(qid.version);
    dest.writeLE(qid.path);
}


void encode9P(ByteWriter& dest, Stat const& stat) {
    dest.writeLE(stat.size);
    dest.writeLE(stat.type);
    dest.writeLE(stat.dev);

    encode9P(dest, stat.qid);

    dest.writeLE(stat.mode);
    dest.writeLE(stat.atime);
    dest.writeLE(stat.mtime);
    dest.writeLE(stat.length);

    dest.writeLE(static_cast<uint16>(stat.name.size()));
    dest.write(stat.name.view());

    dest.writeLE(static_cast<uint16>(stat.uid.size()));
    dest.write(stat.uid.view());

    dest.writeLE(static_cast<uint16>(stat.gid.size()));
    dest.write(stat.gid.view());

    dest.writeLE(static_cast<uint16>(stat.muid.size()));
    dest.write(stat.muid.view());
}

void writeHeader(ByteWriter& byteStream, size_type msgSize, MessageType type, byte tag) {
	byteStream.writeLE(msgSize);
	byteStream.writeLE(asByte(type));
	byteStream.writeLE(Tag(tag));
}


TEST(P9, testHeaderSize) {
    ASSERT_EQ(4u + 1u + 2u, headerSize());
}


TEST(P9, testParsingMessageHeader) {
    // Form a normal message with no data:
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};
	writeHeader(byteStream, 4 + 1 + 2, MessageType::TVersion, 1);

	auto reader = ByteReader{byteStream.viewWritten()};
	auto res = UnversionedParser{127}
			.parseMessageHeader(reader);
    ASSERT_TRUE(res.isOk());

    auto header = res.unwrap();
    ASSERT_EQ(4u + 1u + 2u, header.messageSize);
	ASSERT_EQ(asByte(MessageType::TVersion), header.type);
    ASSERT_EQ(1_us, header.tag);
}


TEST(P9, parsingMessageHeaderWithUnknownMessageType) {
    // Form a normal message with no data:
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};
	byteStream.writeLE(size_type(4 + 1 + 2));
	byteStream.writeLE(static_cast<byte>(-1));
	byteStream.writeLE(Tag(1));

	auto reader = ByteReader{byteStream.viewWritten()};
	ASSERT_TRUE(UnversionedParser{128}.parseMessageHeader(reader).isOk());
}


TEST(P9, testParsingHeaderWithInsufficientData) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};

    // Only write one header field. Should be not enough data to read a header.
	byteStream.writeLE(size_type(4 + 1 + 2));  // type and tag are not written deliberately

	auto reader = ByteReader{byteStream.viewWritten()};
	auto maybeHeader = UnversionedParser{16}.parseMessageHeader(reader);
	ASSERT_TRUE(maybeHeader.isError());
}


TEST(P9_2000, testParsingIllformedMessageHeader) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};
	// Set declared message size less then header size.
	writeHeader(byteStream, 1 + 2, MessageType::TVersion, 1);

	auto reader = ByteReader{byteStream.viewWritten()};
	ASSERT_TRUE(UnversionedParser{32}.parseMessageHeader(reader).isError());
}


TEST(P9, parsingIllFormedHeaderForMessagesLargerMTUShouldError) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};

	UnversionedParser proc{12};
    // Set declared message size to be more then negotiated message size
	writeHeader(byteStream, proc.maxMessageSize() + 100, MessageType::TVersion, 1);

	auto reader = ByteReader{byteStream.viewWritten()};
	ASSERT_TRUE(proc.parseMessageHeader(reader).isError());
}


TEST(P9_2000, parseIncorrectlySizedSmallerResponse) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};

    // Set declared message size to be more then negotiated message size
	writeHeader(byteStream, headerSize() + sizeof(int32), MessageType::RVersion, 1);
	byteStream.writeLE(byte(3));

	auto maybeParser = createParser(20, kProtocolVersion);
	ASSERT_TRUE(maybeParser.isOk());
	auto& parser = *maybeParser;

	auto reader = ByteReader{byteStream.viewWritten()};
	auto header = parser.parseMessageHeader(reader);
    ASSERT_TRUE(header.isOk());

	auto message = parser.parseResponse(header.unwrap(), reader);
    ASSERT_TRUE(message.isError());
}


TEST(P9_2000, parseIncorrectlySizedLargerResponse) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};

    // Set declared message size to be more then negotiated message size
	writeHeader(byteStream, headerSize() + sizeof(int32), MessageType::RVersion, 1);
	byteStream.writeLE(static_cast<int64>(999999));

	auto reader = ByteReader{byteStream.viewWritten()};

	auto maybeParser = createParser(20, kProtocolVersion);
	ASSERT_TRUE(maybeParser.isOk());
	auto& parser = *maybeParser;

	auto header = parser.parseMessageHeader(reader);
    ASSERT_TRUE(header.isOk());

	auto message = parser.parseResponse(*header, reader);
    ASSERT_TRUE(message.isError());
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 9P2000 Message parsing test suit
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class P9Messages : public ::testing::Test {
protected:

    // cppcheck-suppress unusedFunction
    void SetUp() override {
        _memBuf.view().fill(0xFE);

        _writer.rewind();
        _reader.rewind();
    }

    template<typename RequestType>
    Result<RequestType, Error>
	getRequestOrFail() {
        _reader.limit(_writer.limit());


		auto maybeParser = createParser(kMaxMessageSize, kProtocolVersion);
		if (!maybeParser) {
			[&maybeParser]() { FAIL() << "Failed to create a parser: " << maybeParser.getError(); } ();
			return maybeParser.moveError();
		}

		auto& proc = *maybeParser;
		auto const expectType = messageCode(RequestType{});

        return proc.parseMessageHeader(_reader)
				.then([expectType](MessageHeader header) {
					return (header.type != expectType)
							? Result<MessageHeader, Error>{getCannedError(CannedError::UnsupportedMessageType)}
							: Result<MessageHeader, Error>{types::okTag, std::move(header)};
                })
				.then([this, &proc](MessageHeader&& header) {
                    return proc.parseRequest(header, _reader);
                })
				.then([](RequestMessage&& msg) -> Result<RequestType, Error> {
					bool const isType = std::holds_alternative<RequestType>(msg);

                    if (!isType) {
                        []() { FAIL() << "Parsed request is on unexpected type"; } ();
						return getCannedError(CannedError::UnsupportedMessageType);
                    }

                    return Ok<RequestType>(std::get<RequestType>(std::move(msg)));
                })
                .mapError([](Error&& e) -> Error {
                    [&e]() {
                        FAIL() << e.toString();
                    } ();

                    return e;
                });
    }


    template<typename ResponseType>
    Result<ResponseType, Error>
	getResponseOrFail() {
        _reader.limit(_writer.limit());

		auto maybeParser = createParser(kMaxMessageSize, kProtocolVersion);
		if (!maybeParser) {
			[&maybeParser]() { FAIL() << "Failed to create a parser: " << maybeParser.getError(); } ();
			return maybeParser.moveError();
		}

		auto& proc = *maybeParser;

		auto const expectType = messageCode(ResponseType{});
		return proc.parseMessageHeader(_reader)
				.then([expectType](MessageHeader header) {
					return (header.type == expectType)
							? Result<MessageHeader, Error>{types::okTag, std::move(header)}
							: Result<MessageHeader, Error>{types::errTag, getCannedError(CannedError::UnsupportedMessageType)};
                })
				.then([this, &proc](MessageHeader&& header) {
                    return proc.parseResponse(header, _reader);
                })
				.then([](ResponseMessage&& msg) -> Result<ResponseType, Error> {
					bool const isType = std::holds_alternative<ResponseType>(msg);

                    if (!isType) {
                        []() { FAIL() << "Parsed request is on unexpected type"; } ();
						return getCannedError(CannedError::UnsupportedMessageType);
                    }

                    return Ok<ResponseType>(std::get<ResponseType>(std::move(msg)));
                })
                .mapError([](Error&& e) {
					[&e]() { FAIL() << e.toString(); } ();

                    return e;
                });
    }

protected:

	MemoryManager   _memManager{kMaxMessageSize};
	MemoryResource  _memBuf{_memManager.allocate(kMaxMessageSize).unwrap()};
    ByteWriter      _writer{_memBuf};
    ByteReader      _reader{_memBuf};
};




TEST_F(P9Messages, createVersionRequest) {
	auto const testVersion = kProtocolVersion;

	auto writer = RequestWriter{_writer, kNoTag};
	writer << Request::Version{kMaxMessageSize, testVersion};
	writer.build();

	getRequestOrFail<Request::Version>()
			.then([testVersion](Request::Version const& request) {
				EXPECT_EQ(kMaxMessageSize, request.msize);
                EXPECT_EQ(testVersion, request.version);
            });
}

TEST_F(P9Messages, createVersionResponse) {
	auto writer = ResponseWriter{_writer, kNoTag};
	writer << Response::Version{718, "9Pe"};
	writer.build();

	getResponseOrFail<Response::Version>()
            .then([](Response::Version const& response) {
                ASSERT_EQ(718, response.msize);
                ASSERT_EQ("9Pe", response.version);
            });
}

TEST_F(P9Messages, parseVersionResponse) {
    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, headerSize() + sizeof(int32) + sizeof(int16) + 2, MessageType::RVersion, 1);
	_writer.writeLE(static_cast<int32>(512));
	_writer.writeLE(static_cast<int16>(2));
    _writer.write(StringLiteral("9P").view());
    _writer.flip();

	getResponseOrFail<Response::Version>()
            .then([](Response::Version const& response) {
                ASSERT_EQ(512, response.msize);
                ASSERT_EQ("9P", response.version);
            });
}


TEST_F(P9Messages, createAuthRequest) {
	RequestWriter writer{_writer};
	writer <<  Request::Auth{312, "User mcUsers", "Somewhere near"};
	writer.build();

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
	writer.build();


	getResponseOrFail<Response::Auth>()
            .then([qid](Response::Auth&& response) {
                ASSERT_EQ(qid, response.qid);
            });
}


TEST_F(P9Messages, parseAuthResponse) {
    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, headerSize() + 13, MessageType::RAuth, 1);

	_writer.writeLE(byte(13));     // QID.type
	_writer.writeLE(static_cast<uint32>(91));   // QID.version
	_writer.writeLE(static_cast<uint64>(441));  // QID.path
    _writer.flip();

	getResponseOrFail<Response::Auth>()
            .then([](Response::Auth&& response) {
                EXPECT_EQ(13, response.qid.type);
                EXPECT_EQ(91, response.qid.version);
                EXPECT_EQ(441, response.qid.path);
            });
}

// No such thing as error request!

TEST_F(P9Messages, createErrorResponse) {
    auto const testError = StringLiteral{"Something went right :)"};
	ResponseWriter writer{_writer, 3};
	writer << Response::Error{testError};
	writer.build();

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

    _writer.flip();

	getResponseOrFail<Response::Error>()
            .then([expectedErrorMessage](Response::Error&& response) {
                EXPECT_EQ(expectedErrorMessage, response.ename);
            });
}


TEST_F(P9Messages, createFlushRequest) {
	RequestWriter writer{_writer};
	writer << Request::Flush{7711};
	writer.build();

	getRequestOrFail<Request::Flush>()
            .then([](Request::Flush&& request) {
                ASSERT_EQ(7711, request.oldtag);
            });
}


TEST_F(P9Messages, createFlushResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::Flush{};
	writer.build();

	getResponseOrFail<Response::Flush>();
}


TEST_F(P9Messages, parseFlushResponse) {
    writeHeader(_writer, headerSize(), MessageType::RFlush, 1);
    _writer.flip();

	getResponseOrFail<Response::Flush>();
}


TEST_F(P9Messages, createAttachRequest) {
	RequestWriter writer{_writer};
	writer << Request::Attach{3310, 1841, "McFace", "close to u"};
	writer.build();

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
	writer.build();

	getResponseOrFail<Response::Attach>()
            .then([qid](Response::Attach&& response) {
                ASSERT_EQ(qid, response.qid);
            });
}

TEST_F(P9Messages, parseAttachResponse) {
    writeHeader(_writer, headerSize() + 13, MessageType::RAttach, 1);

	_writer.writeLE(byte(81));                      // QID.type
	_writer.writeLE(static_cast<uint32>(3));        // QID.version
	_writer.writeLE(static_cast<uint64>(1049));     // QID.path
    _writer.flip();

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
	writer.build();

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
	writer.build();

	getResponseOrFail<Response::Open>()
            .then([qid](Response::Open&& response) {
                ASSERT_EQ(qid, response.qid);
                ASSERT_EQ(817, response.iounit);
            });
}

TEST_F(P9Messages, parseOpenResponse) {
	auto const qid = Qid {4173, 71, 2};
    size_type const iounit = 998;

    writeHeader(_writer,
                headerSize() +
				styxe::protocolSize(qid) +
				styxe::protocolSize(iounit),
                MessageType::ROpen, 1);
    // qid
    encode9P(_writer, qid);
    // iounit
	_writer.writeLE(iounit);
    _writer.flip();

	getResponseOrFail<Response::Open>()
            .then([qid, iounit](Response::Open&& response) {
                EXPECT_EQ(qid, response.qid);
                EXPECT_EQ(iounit, response.iounit);
            });
}



TEST_F(P9Messages, createCreateRequest) {
	RequestWriter writer{_writer};
	writer << Request::Create{1734, "mcFance", 11, OpenMode::EXEC};
	writer.build();

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
	writer.build();

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

    writeHeader(_writer,
                headerSize() +
				styxe::protocolSize(qid) +
				styxe::protocolSize(iounit),
                MessageType::RCreate, 1);

    encode9P(_writer, qid);
    // iounit
	_writer.writeLE(iounit);
    _writer.flip();

	getResponseOrFail<Response::Create>()
            .then([qid](Response::Create&& response) {
                EXPECT_EQ(qid, response.qid);
                EXPECT_EQ(778, response.iounit);
            });
}


TEST_F(P9Messages, createReadRequest) {
	RequestWriter writer{_writer};
	writer << Request::Read{7234, 18, 772};
	writer.build();

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
	writer << Response::Read{data};
	writer.build();

	getResponseOrFail<Response::Read>()
            .then([data](Response::Read&& response) {
                ASSERT_EQ(data, response.data);
            });
}

TEST_F(P9Messages, parseReadResponse) {
    auto const messageData = StringLiteral{"This is a very important data d-_^b"};
	uint32 const dataLen = messageData.size();

    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, headerSize() + sizeof(uint32) + dataLen, MessageType::RRead, 1);
    // iounit
	_writer.writeLE(dataLen);
    _writer.write(messageData.view());
    _writer.flip();

	getResponseOrFail<Response::Read>()
            .then([dataLen, messageData](Response::Read&& response) {
                EXPECT_EQ(dataLen, response.data.size());
                EXPECT_EQ(response.data, messageData.view());
            });
}


TEST_F(P9Messages, createWriteRequest) {
	char const messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

	RequestWriter writer{_writer, 1};
	writer << Request::Write{15927, 98, data};
	writer.build();

	getRequestOrFail<Request::Write>()
            .then([data](Request::Write&& request) {
                ASSERT_EQ(15927, request.fid);
                ASSERT_EQ(98, request.offset);
                ASSERT_EQ(data, request.data);
            });
}

TEST_F(P9Messages, createWriteResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::Write{71717};
	writer.build();

	getResponseOrFail<Response::Write>()
            .then([](Response::Write&& response) {
                ASSERT_EQ(71717, response.count);
            });
}

TEST_F(P9Messages, parseWriteResponse) {
    writeHeader(_writer, headerSize() + sizeof(uint32), MessageType::RWrite, 1);
    // iounit
	_writer.writeLE(static_cast<uint32>(81177));
    _writer.flip();

	getResponseOrFail<Response::Write>()
            .then([](Response::Write&& response) {
                EXPECT_EQ(81177, response.count);
            });
}



TEST_F(P9Messages, createClunkRequest) {
	RequestWriter writer{_writer, 1};
	writer << Request::Clunk{37509};
	writer.build();

	getRequestOrFail<Request::Clunk>()
            .then([](Request::Clunk&& request) {
                ASSERT_EQ(37509, request.fid);
            });
}

TEST_F(P9Messages, createClunkResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::Clunk{};
	writer.build();

	getResponseOrFail<Response::Clunk>();
}

TEST_F(P9Messages, parseClunkResponse) {
    writeHeader(_writer, headerSize(), MessageType::RClunk, 1);
    _writer.flip();

	getResponseOrFail<Response::Clunk>();
}



TEST_F(P9Messages, createRemoveRequest) {
	RequestWriter writer{_writer, 1};
	writer << Request::Remove{54329};
	writer.build();

	getRequestOrFail<Request::Remove>()
            .then([](Request::Remove&& request) {
                ASSERT_EQ(54329, request.fid);
            });
}

TEST_F(P9Messages, createRemoveResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::Remove{};
	writer.build();

	getResponseOrFail<Response::Remove>();
}

TEST_F(P9Messages, parseRemoveResponse) {
    writeHeader(_writer, headerSize(), MessageType::RRemove, 1);
    _writer.flip();

	getResponseOrFail<Response::Remove>();
}



TEST_F(P9Messages, createStatRequest) {
	RequestWriter writer{_writer, 1};
	writer << Request::Stat{7872};
	writer.build();

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
	writer.build();

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

    writeHeader(_writer,
				headerSize() + sizeof(statResponse.dummySize) + styxe::protocolSize(statResponse.data),
                MessageType::RStat,
                1);
	_writer.writeLE(statResponse.dummySize);
    encode9P(_writer, statResponse.data);
    _writer.flip();

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
	writer.build();

	getRequestOrFail<Request::WStat>()
            .then([stat](Request::WStat&& request) {
                ASSERT_EQ(8193, request.fid);
                ASSERT_EQ(stat, request.stat);
            });
}

TEST_F(P9Messages, createWStatResponse) {
	ResponseWriter writer{_writer, 1};
	writer << Response::WStat{};
	writer.build();

	getResponseOrFail<Response::WStat>();
}

TEST_F(P9Messages, parseWStatResponse) {
    writeHeader(_writer, headerSize(), MessageType::RWStat, 1);
    _writer.flip();

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
	writer.build();

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
	writer.build();

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
	writer.build();

	getResponseOrFail<Response::Walk>()
			.then([&](Response::Walk&& response) {
				ASSERT_EQ(message.nqids, response.nqids);
				ASSERT_EQ(message.qids[2], response.qids[2]);
            });
}

TEST_F(P9Messages, parseWalkResponse) {
    auto const qid = Qid {
            87,
            5481,
            17
    };

    // Set declared message size to be more then negotiated message size
	auto const headPosition = _writer.position();
    writeHeader(_writer, 0, MessageType::RWalk, 1);
    // nwqid
	_writer.writeLE(uint16(1));
    encode9P(_writer, qid);


	auto const totalSize = _writer.position();
    _writer.limit(totalSize);
    _writer.position(headPosition);
	_writer.writeLE(size_type(totalSize));

	getResponseOrFail<Response::Walk>()
            .then([](Response::Walk&& response) {
				EXPECT_EQ(1, response.nqids);
				EXPECT_EQ(17, response.qids[0].type);
                EXPECT_EQ(5481, response.qids[0].version);
				EXPECT_EQ(87, response.qids[0].path);
            });
}
