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
 * libcadence Unit Test Suit
 * @file: test/test_9P2000.cpp
 * @author: soultaker
 *
 *******************************************************************************/
#include "styxe/9p2000.hpp"  // Class being tested

#include <solace/exception.hpp>
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

void writeHeader(ByteWriter& writer, size_type msgSize, MessageType type, byte tag) {
    writer.writeLE(msgSize);
    writer.writeLE(static_cast<byte>(type));
    writer.writeLE(Tag(tag));
}



TEST(P9_2000, testHeaderSize) {
    Protocol proc;

    ASSERT_EQ(4u + 1u + 2u, headerSize());
}


TEST(P9_2000, testSettingFrameSize) {
    Protocol proc(127);

    ASSERT_EQ(127u, proc.maxPossibleMessageSize());
    ASSERT_EQ(127u, proc.maxNegotiatedMessageSize());

    proc.maxNegotiatedMessageSize(56);
    ASSERT_EQ(127u, proc.maxPossibleMessageSize());
    ASSERT_EQ(56u, proc.maxNegotiatedMessageSize());

    ASSERT_ANY_THROW(proc.maxNegotiatedMessageSize(300));
}

TEST(P9_2000, testParsingMessageHeader) {
    MemoryManager _mem(1024);
    Protocol proc;

    // Form a normal message with no data:
    auto memBuffer = _mem.allocate(512);
    auto writer = ByteWriter{memBuffer};
    writeHeader(writer, 4 + 1 + 2, MessageType::TVersion, 1);

    auto reader = ByteReader{memBuffer};
    auto res = proc.parseMessageHeader(reader);
    ASSERT_TRUE(res.isOk());

    auto header = res.unwrap();
    ASSERT_EQ(4u + 1u + 2u, header.messageSize);
    ASSERT_EQ(MessageType::TVersion, header.type);
    ASSERT_EQ(1_us, header.tag);
}


TEST(P9_2000, parsingMessageHeaderWithUnknownMessageType) {
    Protocol proc;

    // Form a normal message with no data:
    byte memBuffer[512];
    auto writer = ByteWriter{wrapMemory(memBuffer)};

    writer.writeLE(size_type(4 + 1 + 2));
    writer.writeLE(static_cast<byte>(-1));
    writer.writeLE(Tag(1));

    auto reader = ByteReader{wrapMemory(memBuffer)};
    ASSERT_TRUE(proc.parseMessageHeader(reader).isError());
}

TEST(P9_2000, testParsingHeaderWithInsufficientData) {
    MemoryManager _mem(1024);
    Protocol proc;

    auto memBuffer = _mem.allocate(512);
    auto writer = ByteWriter{memBuffer};

    // Only write one header field. Should be not enough data to read a header.
    writer.writeLE(size_type(4 + 1 + 2));

    auto reader = ByteReader{memBuffer};
    auto res = proc.parseMessageHeader(reader);

    ASSERT_TRUE(res.isError());
}


TEST(P9_2000, testParsingIllformedMessageHeader) {
    MemoryManager _mem(1024);

    auto memBuffer = _mem.allocate(512);
    auto writer = ByteWriter{memBuffer};
    // Set declared message size less then header size.
    writeHeader(writer, 1 + 2, MessageType::TVersion, 1);

    Protocol proc;
    auto reader = ByteReader{memBuffer};
    ASSERT_TRUE(proc.parseMessageHeader(reader).isError());
}

TEST(P9_2000, parsingIllFormedHeaderForMessagesLargerMTUShouldError) {
    MemoryManager _mem(1024);
    Protocol proc;

    auto memBuffer = _mem.allocate(512);
    auto writer = ByteWriter{memBuffer};

    proc.maxNegotiatedMessageSize(20);
    // Set declared message size to be more then negotiated message size
    writeHeader(writer, proc.maxNegotiatedMessageSize() + 100, MessageType::TVersion, 1);

    auto reader = ByteReader{memBuffer};
    ASSERT_TRUE(proc.parseMessageHeader(reader).isError());
}


TEST(P9_2000, parseIncorrectlySizedSmallerResponse) {
    MemoryManager _mem(1024);
    Protocol proc;

    auto memBuffer = _mem.allocate(512);
    auto writer = ByteWriter{memBuffer};

    // Set declared message size to be more then negotiated message size
    writeHeader(writer, headerSize() + sizeof(int32), MessageType::RVersion, 1);
    writer.writeLE(byte(3));

    auto reader = ByteReader{memBuffer};
    auto header = proc.parseMessageHeader(reader);
    ASSERT_TRUE(header.isOk());

    auto message = proc.parseResponse(header.unwrap(), reader);
    ASSERT_TRUE(message.isError());
}

TEST(P9_2000, parseIncorrectlySizedLargerResponse) {
    MemoryManager _mem(1024);
    Protocol proc;

    auto memBuffer = _mem.allocate(headerSize() + sizeof(int32)*2);
    auto writer = ByteWriter{memBuffer};

    // Set declared message size to be more then negotiated message size
    writeHeader(writer, headerSize() + sizeof(int32), MessageType::RVersion, 1);
    writer.writeLE(static_cast<int64>(999999));

    auto reader = ByteReader{memBuffer};
    auto header = proc.parseMessageHeader(reader);
    ASSERT_TRUE(header.isOk());

    auto message = proc.parseResponse(header.unwrap(), reader);
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
    getRequestOfFail(MessageType expectType) {
        _reader.limit(_writer.limit());

        return proc.parseMessageHeader(_reader)
                .then([expectType](MessageHeader&& header) {
                    return (header.type != expectType)
                    ? Result<MessageHeader, Error>(Err(getCannedError(CannedError::UnsupportedMessageType)))
                    : Result<MessageHeader, Error>(Ok(std::move(header)));
                })
                .then([this](MessageHeader&& header) {
                    return proc.parseRequest(header, _reader);
                })
                .then([this](RequestMessage&& msg) -> Result<RequestType, Error> {
                    const bool isType = std::holds_alternative<RequestType>(msg);

                    if (!isType) {
                        []() { FAIL() << "Parsed request is on unexpected type"; } ();
                        return Err(getCannedError(CannedError::UnsupportedMessageType));
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
    getResponseOfFail(MessageType expectType) {
        _reader.limit(_writer.limit());

        return proc.parseMessageHeader(_reader)
                .then([expectType](MessageHeader&& header) {
                    return (header.type != expectType)
                ? Result<MessageHeader, Error>(Err(getCannedError(CannedError::UnsupportedMessageType)))
                : Result<MessageHeader, Error>(Ok(std::move(header)));
                })
                .then([this](MessageHeader&& header) {
                    return proc.parseResponse(header, _reader);
                })
                .then([this](ResponseMessage&& msg) -> ResponseType {
                    const bool isType = std::holds_alternative<ResponseType>(msg);

                    [isType]() {
                        ASSERT_TRUE(isType);
                    } ();

                    return std::get<ResponseType>(std::move(msg));
                })
                .mapError([](Error&& e) {
                    [&e]() {
                        FAIL() << e.toString();
                    } ();

                    return e;
                });
    }

protected:

    Protocol        proc;
    MemoryManager   _memManager {kMaxMesssageSize};
    MemoryResource  _memBuf{_memManager.allocate(kMaxMesssageSize)};
    ByteWriter      _writer{_memBuf};
    ByteReader      _reader{_memBuf};
};




TEST_F(P9Messages, createVersionRequest) {
    const auto testVersion = Protocol::PROTOCOL_VERSION;

    RequestBuilder(_writer, Protocol::NO_TAG)
            .version(testVersion)
            .build();

    getRequestOfFail<Request::Version>(MessageType::TVersion)
            .then([this, testVersion](Request::Version const& request) {
                EXPECT_EQ(proc.maxPossibleMessageSize(), request.msize);
                EXPECT_EQ(testVersion, request.version);
            });
}

TEST_F(P9Messages, createVersionRespose) {
    ResponseBuilder(_writer, Protocol::NO_TAG)
            .version("9Pe", 718)
            .build();

    getResponseOfFail<Response::Version>(MessageType::RVersion)
            .then([](Response::Version const& response) {
                ASSERT_EQ(718, response.msize);
                ASSERT_EQ("9Pe", response.version);
            });
}

TEST_F(P9Messages, parseVersionRespose) {
    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, headerSize() + sizeof(int32) + sizeof(int16) + 2, MessageType::RVersion, 1);
    _writer.writeLE(static_cast<int32>(512));
    _writer.writeLE(static_cast<int16>(2));
    _writer.write(StringLiteral("9P").view());
    _writer.flip();

    getResponseOfFail<Response::Version>(MessageType::RVersion)
            .then([](Response::Version const& response) {
                ASSERT_EQ(512, response.msize);
                ASSERT_EQ("9P", response.version);
            });
}


TEST_F(P9Messages, createAuthRequest) {
    RequestBuilder{_writer}
            .auth(312, "User mcUsers", "Somewhere near")
            .build();

    getRequestOfFail<Request::Auth>(MessageType::TAuth)
            .then([](Request::Auth&& request) {
                ASSERT_EQ(312, request.afid);
                ASSERT_EQ("User mcUsers", request.uname);
                ASSERT_EQ("Somewhere near", request.aname);
            });
}


TEST_F(P9Messages, createAuthRespose) {
    auto const qid = Qid {
            71,
            17,
            8187
    };

    ResponseBuilder(_writer, 1)
            .auth(qid)
            .build();

    getResponseOfFail<Response::Auth>(MessageType::RAuth)
            .then([qid](Response::Auth&& response) {
                ASSERT_EQ(qid, response.qid);
            });
}


TEST_F(P9Messages, parseAuthRespose) {
    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, headerSize() + 13, MessageType::RAuth, 1);

    _writer.writeLE(byte(13));     // QID.type
    _writer.writeLE(static_cast<uint32>(91));   // QID.version
    _writer.writeLE(static_cast<uint64>(441));  // QID.path
    _writer.flip();

    getResponseOfFail<Response::Auth>(MessageType::RAuth)
            .then([](Response::Auth&& response) {
                EXPECT_EQ(13, response.qid.type);
                EXPECT_EQ(91, response.qid.version);
                EXPECT_EQ(441, response.qid.path);
            });
}

// No such thing as error request!

TEST_F(P9Messages, createErrorRespose) {
    auto const testError = StringLiteral{"Something went right :)"};
    ResponseBuilder(_writer, 3)
            .error(testError)
            .build();

    getResponseOfFail<Response::Error>(MessageType::RError)
            .then([testError](Response::Error&& response) {
                ASSERT_EQ(testError, response.ename);
            });
}

TEST_F(P9Messages, parseErrorRespose) {
    const auto expectedErrorMessage = StringLiteral{"All good!"};

    styxe::Encoder{_writer}
            .header(MessageType::RError, 1, styxe::Encoder::protocolSize(expectedErrorMessage))
            .encode(expectedErrorMessage);
    _writer.flip();

    getResponseOfFail<Response::Error>(MessageType::RError)
            .then([expectedErrorMessage](Response::Error&& response) {
                EXPECT_EQ(expectedErrorMessage, response.ename);
            });
}


TEST_F(P9Messages, createFlushRequest) {
    RequestBuilder{_writer}
            .flush(7711)
            .build();

    getRequestOfFail<Request::Flush>(MessageType::TFlush)
            .then([](Request::Flush&& request) {
                ASSERT_EQ(7711, request.oldtag);
            });
}


TEST_F(P9Messages, createFlushResponse) {
    ResponseBuilder(_writer, 1)
            .flush()
            .build();

    getResponseOfFail<Response::Flush>(MessageType::RFlush);
}


TEST_F(P9Messages, parseFlushRespose) {
    writeHeader(_writer, headerSize(), MessageType::RFlush, 1);
    _writer.flip();

    getResponseOfFail<Response::Flush>(MessageType::RFlush);
}


TEST_F(P9Messages, createAttachRequest) {
    RequestBuilder{_writer}
            .attach(3310, 1841, "McFace", "close to u")
            .build();

    getRequestOfFail<Request::Attach>(MessageType::TAttach)
            .then([](Request::Attach&& request) {
                ASSERT_EQ(3310, request.fid);
                ASSERT_EQ(1841, request.afid);
                ASSERT_EQ("McFace", request.uname);
                ASSERT_EQ("close to u", request.aname);
            });
}

TEST_F(P9Messages, createAttachRespose) {
    auto const qid = Qid {
            91,
            3,
            7771
    };

    ResponseBuilder(_writer, 1)
            .attach(qid)
            .build();

    getResponseOfFail<Response::Attach>(MessageType::RAttach)
            .then([qid](Response::Attach&& response) {
                ASSERT_EQ(qid, response.qid);
            });
}

TEST_F(P9Messages, parseAttachRespose) {
    writeHeader(_writer, headerSize() + 13, MessageType::RAttach, 1);

    _writer.writeLE(byte(81));                      // QID.type
    _writer.writeLE(static_cast<uint32>(3));        // QID.version
    _writer.writeLE(static_cast<uint64>(1049));     // QID.path
    _writer.flip();

    getResponseOfFail<Response::Attach>(MessageType::RAttach)
            .then([](Response::Attach&& response) {
                EXPECT_EQ(81, response.qid.type);
                EXPECT_EQ(3, response.qid.version);
                EXPECT_EQ(1049, response.qid.path);
            });
}


TEST_F(P9Messages, createOpenRequest) {
    RequestBuilder{_writer}
            .open(517, OpenMode::RDWR)
            .build();

    getRequestOfFail<Request::Open>(MessageType::TOpen)
            .then([](Request::Open&& request) {
                ASSERT_EQ(517, request.fid);
                ASSERT_EQ(OpenMode::RDWR, request.mode);
            });
}


TEST_F(P9Messages, createOpenRespose) {
    auto const qid = Qid {
                                        8,
                                        13,
                                        323
                                    };

    ResponseBuilder(_writer, 1)
            .open(qid, 817)
            .build();

    getResponseOfFail<Response::Open>(MessageType::ROpen)
            .then([qid](Response::Open&& response) {
                ASSERT_EQ(qid, response.qid);
                ASSERT_EQ(817, response.iounit);
            });
}

TEST_F(P9Messages, parseOpenRespose) {
    auto const qid = Qid {
            8,
            71,
            4173
    };
    size_type const iounit = 998;

    writeHeader(_writer,
                headerSize() +
                styxe::Encoder::protocolSize(qid) +
                styxe::Encoder::protocolSize(iounit),
                MessageType::ROpen, 1);
    // qid
    encode9P(_writer, qid);
    // iounit
    _writer.writeLE(iounit);
    _writer.flip();

    getResponseOfFail<Response::Open>(MessageType::ROpen)
            .then([qid, iounit](Response::Open&& response) {
                EXPECT_EQ(qid, response.qid);
                EXPECT_EQ(iounit, response.iounit);
            });
}



TEST_F(P9Messages, createCreateRequest) {
    RequestBuilder{_writer}
            .create(1734, "mcFance", 11, OpenMode::EXEC)
            .build();

    getRequestOfFail<Request::Create>(MessageType::TCreate)
            .then([](Request::Create&& request) {
                ASSERT_EQ(1734, request.fid);
                ASSERT_EQ("mcFance", request.name);
                ASSERT_EQ(11, request.perm);
                ASSERT_EQ(OpenMode::EXEC, request.mode);
            });
}

TEST_F(P9Messages, createCreateRespose) {
    auto const qid = Qid {
            8,
            13,
            323
    };

    ResponseBuilder(_writer, 1)
            .create(qid, 718)
            .build();

    getResponseOfFail<Response::Create>(MessageType::RCreate)
            .then([qid](Response::Create&& response) {
                ASSERT_EQ(qid, response.qid);
                ASSERT_EQ(718, response.iounit);
            });
}

TEST_F(P9Messages, parseCreateRespose) {
    auto const qid = Qid {
            8,
            13,
            323
    };
    size_type const iounit = 778;

    writeHeader(_writer,
                headerSize() +
                styxe::Encoder::protocolSize(qid) +
                styxe::Encoder::protocolSize(iounit),
                MessageType::RCreate, 1);

    encode9P(_writer, qid);
    // iounit
    _writer.writeLE(iounit);
    _writer.flip();

    getResponseOfFail<Response::Create>(MessageType::RCreate)
            .then([qid](Response::Create&& response) {
                EXPECT_EQ(qid, response.qid);
                EXPECT_EQ(778, response.iounit);
            });
}


TEST_F(P9Messages, createReadRequest) {
    RequestBuilder{_writer}
            .read(7234, 18, 772)
            .build();

    getRequestOfFail<Request::Read>(MessageType::TRead)
            .then([](Request::Read&& request) {
                ASSERT_EQ(7234, request.fid);
                ASSERT_EQ(18, request.offset);
                ASSERT_EQ(772, request.count);
            });
}

TEST_F(P9Messages, createReadRespose) {
    const char content[] = "Good news everyone!";
    auto data = wrapMemory(content);
    ResponseBuilder(_writer, 1)
            .read(data)
            .build();

    getResponseOfFail<Response::Read>(MessageType::RRead)
            .then([data](Response::Read&& response) {
                ASSERT_EQ(data, response.data);
            });
}

TEST_F(P9Messages, parseReadRespose) {
    auto const messageData = StringLiteral{"This is a very important data d-_^b"};
    const uint32 dataLen = messageData.size();

    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, headerSize() + sizeof(uint32) + dataLen, MessageType::RRead, 1);
    // iounit
    _writer.writeLE(dataLen);
    _writer.write(messageData.view());
    _writer.flip();

    getResponseOfFail<Response::Read>(MessageType::RRead)
            .then([dataLen, messageData](Response::Read&& response) {
                EXPECT_EQ(dataLen, response.data.size());
                EXPECT_EQ(response.data, messageData.view());
            });
}


TEST_F(P9Messages, createWriteRequest) {
    const char messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

    RequestBuilder{_writer}
            .write(15927, 98, data)
            .build();

    getRequestOfFail<Request::Write>(MessageType::TWrite)
            .then([data](Request::Write&& request) {
                ASSERT_EQ(15927, request.fid);
                ASSERT_EQ(98, request.offset);
                ASSERT_EQ(data, request.data);
            });
}

TEST_F(P9Messages, createWriteRespose) {
    ResponseBuilder(_writer, 1)
            .write(71717)
            .build();

    getResponseOfFail<Response::Write>(MessageType::RWrite)
            .then([](Response::Write&& response) {
                ASSERT_EQ(71717, response.count);
            });
}

TEST_F(P9Messages, parseWriteRespose) {
    writeHeader(_writer, headerSize() + sizeof(uint32), MessageType::RWrite, 1);
    // iounit
    _writer.writeLE(static_cast<uint32>(81177));
    _writer.flip();

    getResponseOfFail<Response::Write>(MessageType::RWrite)
            .then([](Response::Write&& response) {
                EXPECT_EQ(81177, response.count);
            });
}



TEST_F(P9Messages, createClunkRequest) {
    RequestBuilder{_writer}
            .clunk(37509)
            .build();

    getRequestOfFail<Request::Clunk>(MessageType::TClunk)
            .then([](Request::Clunk&& request) {
                ASSERT_EQ(37509, request.fid);
            });
}

TEST_F(P9Messages, createClunkRespose) {
    ResponseBuilder(_writer, 1)
            .clunk()
            .build();

    getResponseOfFail<Response::Clunk>(MessageType::RClunk);
}

TEST_F(P9Messages, parseClunkRespose) {
    writeHeader(_writer, headerSize(), MessageType::RClunk, 1);
    _writer.flip();

    getResponseOfFail<Response::Clunk>(MessageType::RClunk);
}



TEST_F(P9Messages, createRemoveRequest) {
    RequestBuilder{_writer}
            .remove(54329)
            .build();

    getRequestOfFail<Request::Remove>(MessageType::TRemove)
            .then([](Request::Remove&& request) {
                ASSERT_EQ(54329, request.fid);
            });
}

TEST_F(P9Messages, createRemoveRespose) {
    ResponseBuilder(_writer, 1)
            .remove()
            .build();

    getResponseOfFail<Response::Remove>(MessageType::RRemove);
}

TEST_F(P9Messages, parseRemoveRespose) {
    writeHeader(_writer, headerSize(), MessageType::RRemove, 1);
    _writer.flip();

    getResponseOfFail<Response::Remove>(MessageType::RRemove);
}



TEST_F(P9Messages, createStatRequest) {
    RequestBuilder{_writer}
            .stat(7872)
            .build();

    getRequestOfFail<Request::StatRequest>(MessageType::TStat)
            .then([](Request::StatRequest&& request) {
                ASSERT_EQ(7872, request.fid);
            });
}

TEST_F(P9Messages, createStatRespose) {
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

    ResponseBuilder(_writer, 1)
            .stat(stat)
            .build();

    getResponseOfFail<Response::Stat>(MessageType::RStat)
            .then([stat](Response::Stat&& response) {
                ASSERT_EQ(stat, response.data);
            });
}

TEST_F(P9Messages, parseStatRespose) {
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
                headerSize() + sizeof(statResponse.dummySize) + styxe::Encoder::protocolSize(statResponse.data),
                MessageType::RStat,
                1);
    _writer.writeLE(statResponse.dummySize);
    encode9P(_writer, statResponse.data);
    _writer.flip();

    getResponseOfFail<Response::Stat>(MessageType::RStat)
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

    RequestBuilder{_writer}
            .writeStat(8193, stat)
            .build();

    getRequestOfFail<Request::WStat>(MessageType::TWStat)
            .then([stat](Request::WStat&& request) {
                ASSERT_EQ(8193, request.fid);
                ASSERT_EQ(stat, request.stat);
            });
}

TEST_F(P9Messages, createWStatRespose) {
    ResponseBuilder(_writer, 1)
            .wstat()
            .build();

    getResponseOfFail<Response::WStat>(MessageType::RWStat);
}

TEST_F(P9Messages, parseWStatRespose) {
    writeHeader(_writer, headerSize(), MessageType::RWStat, 1);
    _writer.flip();

    getResponseOfFail<Response::WStat>(MessageType::RWStat);
}




TEST_F(P9Messages, createWalkRequest) {
    auto const destPath = makePath("space", "knowhere");
    RequestBuilder{_writer}
            .walk(213, 124, destPath)
            .build();

    getRequestOfFail<Request::Walk>(MessageType::TWalk)
            .then([&destPath](Request::Walk&& request) {
                EXPECT_EQ(213, request.fid);
                EXPECT_EQ(124, request.newfid);
                EXPECT_EQ(destPath, request.path);
            });
}

TEST_F(P9Messages, createWalkEmptyPathRequest) {
    RequestBuilder{_writer}
            .walk(7374, 542, Path())
            .build();

    getRequestOfFail<Request::Walk>(MessageType::TWalk)
            .then([](Request::Walk&& request) {
                ASSERT_EQ(7374, request.fid);
                ASSERT_EQ(542, request.newfid);
                ASSERT_TRUE(request.path.empty());
            });
}


TEST_F(P9Messages, createWalkRespose) {
    auto qids = makeArray<Qid>(3);
    qids[2].path = 21;
    qids[2].version = 117;
    qids[2].type = 81;
    ResponseBuilder(_writer, 1)
            .walk(qids.view())
            .build();

    getResponseOfFail<Response::Walk>(MessageType::RWalk)
            .then([&qids](Response::Walk&& response) {
                ASSERT_EQ(qids.size(), response.nqids);
                ASSERT_EQ(qids[2], response.qids[2]);
            });
}

TEST_F(P9Messages, parseWalkRespose) {
    auto const qid = Qid {
            87,
            5481,
            17
    };

    // Set declared message size to be more then negotiated message size
    const auto headPosition = _writer.position();
    writeHeader(_writer, 0, MessageType::RWalk, 1);
    // nwqid
    _writer.writeLE(uint16(1));
    encode9P(_writer, qid);


    const auto totalSize = _writer.position();
    _writer.limit(totalSize);
    _writer.position(headPosition);
    _writer.writeLE(size_type(totalSize));

    getResponseOfFail<Response::Walk>(MessageType::RWalk)
            .then([](Response::Walk&& response) {
                EXPECT_EQ(1, response.nqids);
                EXPECT_EQ(87, response.qids[0].type);
                EXPECT_EQ(5481, response.qids[0].version);
                EXPECT_EQ(17, response.qids[0].path);
            });
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 9P2000.e
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class P9E_Messages :
        public P9Messages {
};

TEST_F(P9E_Messages, createSessionRequest) {
    const byte sessionKey[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    const auto data = wrapMemory(sessionKey);

    RequestBuilder{_writer}
            .session(data)
            .build();

    getRequestOfFail<Request::Session>(MessageType::TSession)
            .then([data](Request::Session&& request) {
                ASSERT_EQ(data, wrapMemory(request.key));
            });
}

TEST_F(P9E_Messages, createSessionRequest_NotEnoughData) {
    const byte sessionKey[5] = {8, 7, 6, 5, 4};

    ASSERT_THROW(RequestBuilder{_writer}
                 .session(wrapMemory(sessionKey)),
                 Solace::Exception);
}

TEST_F(P9E_Messages, parseSessionRequest_NotEnoughData) {
    const byte sessionKey[5] = {8, 7, 6, 5, 4};
    auto keyData = wrapMemory(sessionKey);

    // Set declared message size to be more then negotiated message size
    styxe::Encoder{_writer}
            .header(MessageType::TSession, 1, keyData.size());
    _writer.write(keyData);

    auto headerResult = proc.parseMessageHeader(_reader);
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(MessageType::TSession, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _reader);
    ASSERT_TRUE(message.isError());
}

TEST_F(P9E_Messages, createSessionRespose) {
    ResponseBuilder(_writer, 1)
            .session()
            .build();

    getResponseOfFail<Response::Session>(MessageType::RSession);
}

TEST_F(P9E_Messages, parseSessionRespose) {
    // Set declared message size to be more then negotiated message size
    styxe::Encoder{_writer}
            .header(MessageType::RSession, 1, 0);
    _writer.flip();

    getResponseOfFail<Response::Session>(MessageType::RSession);
}



TEST_F(P9E_Messages, createShortReadRequest) {
    const auto path = Path::parse("some/wierd/place").unwrap();
    RequestBuilder{_writer}
            .shortRead(32, path)
            .build();

    getRequestOfFail<Request::SRead>(MessageType::TSRead)
        .then([&path](Request::SRead&& request) {
            ASSERT_EQ(32, request.fid);
            ASSERT_EQ(path, request.path);
        });
}


TEST_F(P9E_Messages, createShortReadRespose) {
    const char messageData[] = "This was somewhat important data d^_-b";
    auto data = wrapMemory(messageData);
    ResponseBuilder(_writer, 1)
            .shortRead(data)
            .build();

    getResponseOfFail<Response::Read>(MessageType::RSRead)
            .then([data](Response::Read&& response) {
                EXPECT_EQ(data, response.data);
            });
}


TEST_F(P9E_Messages, parseShortReadRespose) {
    auto const messageData = StringLiteral{"This is a very important data d-_^b"};
    const uint32 dataLen = messageData.size();

    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, headerSize() + sizeof(uint32) + dataLen, MessageType::RSRead, 1);
    // iounit
    _writer.writeLE(dataLen);
    _writer.write(messageData.view());
    _writer.flip();

    getResponseOfFail<Response::Read>(MessageType::RSRead)
            .then([messageData](Response::Read&& response) {
                EXPECT_EQ(messageData.size(), response.data.size());
                EXPECT_EQ(messageData.view(), response.data);
            });
}


TEST_F(P9E_Messages, createShortWriteRequest) {
    const auto path = Path::parse("some/wierd/place").unwrap();
    const char messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

    RequestBuilder{_writer}
            .shortWrite(32, path, data)
            .build();

    getRequestOfFail<Request::SWrite>(MessageType::TSWrite)
        .then([&path, data](Request::SWrite&& request) {
            ASSERT_EQ(32, request.fid);
            ASSERT_EQ(path, request.path);
            ASSERT_EQ(data, request.data);
        });
}


TEST_F(P9E_Messages, createShortWriteRespose) {
    ResponseBuilder(_writer, 1)
            .shortWrite(100500)
            .build();

    getResponseOfFail<Response::Write>(MessageType::RSWrite)
            .then([](Response::Write&& response) {
                EXPECT_EQ(100500, response.count);
            });
}


TEST_F(P9E_Messages, parseShortWriteRespose) {
    styxe::Encoder{_writer}
            .header(MessageType::RSWrite, 1, sizeof(uint32))
            .encode(static_cast<uint32>(81177));
    _writer.flip();

    getResponseOfFail<Response::Write>(MessageType::RSWrite)
            .then([](Response::Write&& response) {
                EXPECT_EQ(81177, response.count);
            });
}
