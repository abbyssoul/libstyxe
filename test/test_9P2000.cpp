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

#include "gtest/gtest.h"


using namespace Solace;
using namespace styxe;


inline uint16 operator "" _us(unsigned long long value) {  // NOLINT(runtime/int)
    return static_cast<uint16>(value);
}

void encode9P(ByteWriter& dest, Protocol::Qid const& qid) {
    dest.writeLE(qid.type);
    dest.writeLE(qid.version);
    dest.writeLE(qid.path);
}

void encode9P(ByteWriter& dest, Protocol::Stat const& stat) {
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

void writeHeader(ByteWriter& writer, Protocol::size_type msgSize, Protocol::MessageType type, byte tag) {
    writer.writeLE(msgSize);
    writer.writeLE(static_cast<byte>(type));
    writer.writeLE(Protocol::Tag(tag));
}



TEST(P9_2000, testHeaderSize) {
    Protocol proc;

    ASSERT_EQ(4u + 1u + 2u, proc.headerSize());
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
    writeHeader(writer, 4 + 1 + 2, Protocol::MessageType::TVersion, 1);

    auto reader = ByteReader{memBuffer};
    auto res = proc.parseMessageHeader(reader);
    ASSERT_TRUE(res.isOk());

    auto header = res.unwrap();
    ASSERT_EQ(4u + 1u + 2u, header.messageSize);
    ASSERT_EQ(Protocol::MessageType::TVersion, header.type);
    ASSERT_EQ(1_us, header.tag);
}


TEST(P9_2000, parsingMessageHeaderWithUnknownMessageType) {
    Protocol proc;

    // Form a normal message with no data:
    byte memBuffer[512];
    auto writer = ByteWriter{wrapMemory(memBuffer)};

    writer.writeLE(Protocol::size_type(4 + 1 + 2));
    writer.writeLE(static_cast<byte>(-1));
    writer.writeLE(Protocol::Tag(1));

    auto reader = ByteReader{wrapMemory(memBuffer)};
    ASSERT_TRUE(proc.parseMessageHeader(reader).isError());
}

TEST(P9_2000, testParsingHeaderWithInsufficientData) {
    MemoryManager _mem(1024);
    Protocol proc;

    auto memBuffer = _mem.allocate(512);
    auto writer = ByteWriter{memBuffer};

    // Only write one header field. Should be not enough data to read a header.
    writer.writeLE(Protocol::size_type(4 + 1 + 2));

    auto reader = ByteReader{memBuffer};
    auto res = proc.parseMessageHeader(reader);

    ASSERT_TRUE(res.isError());
}


TEST(P9_2000, testParsingIllformedMessageHeader) {
    MemoryManager _mem(1024);

    auto memBuffer = _mem.allocate(512);
    auto writer = ByteWriter{memBuffer};
    // Set declared message size less then header size.
    writeHeader(writer, 1 + 2, Protocol::MessageType::TVersion, 1);

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
    writeHeader(writer, proc.maxNegotiatedMessageSize() + 100, Protocol::MessageType::TVersion, 1);

    auto reader = ByteReader{memBuffer};
    ASSERT_TRUE(proc.parseMessageHeader(reader).isError());
}


TEST(P9_2000, parseIncorrectlySizedSmallerResponse) {
    MemoryManager _mem(1024);
    Protocol proc;

    auto memBuffer = _mem.allocate(512);
    auto writer = ByteWriter{memBuffer};

    // Set declared message size to be more then negotiated message size
    writeHeader(writer, proc.headerSize() + sizeof(int32), Protocol::MessageType::RVersion, 1);
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

    auto memBuffer = _mem.allocate(proc.headerSize() + sizeof(int32)*2);
    auto writer = ByteWriter{memBuffer};

    // Set declared message size to be more then negotiated message size
    writeHeader(writer, proc.headerSize() + sizeof(int32), Protocol::MessageType::RVersion, 1);
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

    Result<Protocol::Request, Error>
    getRequestOfFail(Protocol::MessageType expectType) {
        _reader.limit(_writer.limit());

        return proc.parseMessageHeader(_reader)
                .then([expectType](Protocol::MessageHeader&& header) {
                    return (header.type != expectType)
                            ? Result<Protocol::MessageHeader, Error>(Err(getCannedError(CannedError::UnsupportedMessageType)))
                            : Result<Protocol::MessageHeader, Error>(Ok(std::move(header)));
                })
                .then([this](Protocol::MessageHeader&& header) {
                    return proc.parseRequest(header, _reader);
                })
                .mapError([](Error&& e) -> Error {
                    [&e]() {
                        FAIL() << e.toString();
                    } ();

                    return e;
                });
    }

    Result<Protocol::Response, Error>
    getResponseOfFail(Protocol::MessageType expectType) {
        _reader.limit(_writer.limit());

        return proc.parseMessageHeader(_reader)
                .then([expectType](Protocol::MessageHeader&& header) {
                    return (header.type != expectType)
                            ? Result<Protocol::MessageHeader, Error>(Err(getCannedError(CannedError::UnsupportedMessageType)))
                            : Result<Protocol::MessageHeader, Error>(Ok(std::move(header)));
                })
                .then([this](Protocol::MessageHeader&& header) {
                    return proc.parseResponse(header, _reader);
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
    MemoryManager   _memManager {Protocol::MAX_MESSAGE_SIZE};
    MemoryResource  _memBuf{_memManager.allocate(Protocol::MAX_MESSAGE_SIZE)};
    ByteWriter      _writer{_memBuf};
    ByteReader      _reader{_memBuf};
};




TEST_F(P9Messages, createVersionRequest) {
    const auto testVersion = Protocol::PROTOCOL_VERSION;

    Protocol::RequestBuilder(_writer)
            .version(testVersion)
            .build();

    getRequestOfFail(Protocol::MessageType::TVersion)
            .then([this, testVersion](Protocol::Request&& request) {
                ASSERT_EQ(proc.maxPossibleMessageSize(), request.asVersion().msize);
                ASSERT_EQ(testVersion, request.asVersion().version);
            });
}

TEST_F(P9Messages, createVersionRespose) {
    Protocol::ResponseBuilder(_writer, Protocol::NO_TAG)
            .version("9Pe", 718)
            .build();

    getResponseOfFail(Protocol::MessageType::RVersion)
            .then([](Protocol::Response&& response) {
                ASSERT_EQ(718, response.version.msize);
                ASSERT_EQ("9Pe", response.version.version);
            });
}

TEST_F(P9Messages, parseVersionRespose) {
    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, proc.headerSize() + sizeof(int32) + sizeof(int16) + 2, Protocol::MessageType::RVersion, 1);
    _writer.writeLE(static_cast<int32>(512));
    _writer.writeLE(static_cast<int16>(2));
    _writer.write(StringLiteral("9P").view());
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RVersion)
            .then([](Protocol::Response&& response) {
                ASSERT_EQ(512, response.version.msize);
                ASSERT_EQ("9P", response.version.version);
            });
}


TEST_F(P9Messages, createAuthRequest) {
    Protocol::RequestBuilder(_writer)
            .auth(312, "User mcUsers", "Somewhere near")
            .build();

    getRequestOfFail(Protocol::MessageType::TAuth)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(312, request.asAuth().afid);
                ASSERT_EQ("User mcUsers", request.asAuth().uname);
                ASSERT_EQ("Somewhere near", request.asAuth().aname);
            });
}


TEST_F(P9Messages, createAuthRespose) {
    auto const qid = Protocol::Qid {
            71,
            17,
            8187
    };

    Protocol::ResponseBuilder(_writer, 1)
            .auth(qid)
            .build();

    getResponseOfFail(Protocol::MessageType::RAuth)
            .then([qid](Protocol::Response&& response) {
                ASSERT_EQ(qid, response.auth.qid);
            });
}


TEST_F(P9Messages, parseAuthRespose) {
    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, proc.headerSize() + 13, Protocol::MessageType::RAuth, 1);

    _writer.writeLE(byte(13));     // QID.type
    _writer.writeLE(static_cast<uint32>(91));   // QID.version
    _writer.writeLE(static_cast<uint64>(441));  // QID.path
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RAuth)
            .then([](Protocol::Response&& response) {
                EXPECT_EQ(13, response.auth.qid.type);
                EXPECT_EQ(91, response.auth.qid.version);
                EXPECT_EQ(441, response.auth.qid.path);
            });
}

// No such thing as error request!

TEST_F(P9Messages, createErrorRespose) {
    auto const testError = StringLiteral{"Something went right :)"};
    Protocol::ResponseBuilder(_writer, 3)
            .error(testError)
            .build();

    getResponseOfFail(Protocol::MessageType::RError)
            .then([testError](Protocol::Response&& response) {
                ASSERT_EQ(testError, response.error.ename);
            });
}

TEST_F(P9Messages, parseErrorRespose) {
    const auto expectedErrorMessage = StringLiteral{"All good!"};

    Protocol::Encoder(_writer)
            .header(Protocol::MessageType::RError, 1, Protocol::Encoder::protocolSize(expectedErrorMessage))
            .encode(expectedErrorMessage);
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RError)
            .then([expectedErrorMessage](Protocol::Response&& response) {
                EXPECT_EQ(expectedErrorMessage, response.error.ename);
            });
}


TEST_F(P9Messages, createFlushRequest) {
    Protocol::RequestBuilder(_writer)
            .flush(7711)
            .build();

    getRequestOfFail(Protocol::MessageType::TFlush)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(7711, request.asFlush().oldtag);
            });
}


TEST_F(P9Messages, createFlushRespose) {
    Protocol::ResponseBuilder(_writer, 1)
            .flush()
            .build();

    getResponseOfFail(Protocol::MessageType::RFlush);
}


TEST_F(P9Messages, parseFlushRespose) {
    writeHeader(_writer, proc.headerSize(), Protocol::MessageType::RFlush, 1);
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RFlush);
}


TEST_F(P9Messages, createAttachRequest) {
    Protocol::RequestBuilder(_writer)
            .attach(3310, 1841, "McFace", "close to u")
            .build();

    getRequestOfFail(Protocol::MessageType::TAttach)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(3310, request.asAttach().fid);
                ASSERT_EQ(1841, request.asAttach().afid);
                ASSERT_EQ("McFace", request.asAttach().uname);
                ASSERT_EQ("close to u", request.asAttach().aname);
            });
}

TEST_F(P9Messages, createAttachRespose) {
    auto const qid = Protocol::Qid {
            91,
            3,
            7771
    };

    Protocol::ResponseBuilder(_writer, 1)
            .attach(qid)
            .build();

    getResponseOfFail(Protocol::MessageType::RAttach)
            .then([qid](Protocol::Response&& response) {
                ASSERT_EQ(qid, response.attach.qid);
            });
}

TEST_F(P9Messages, parseAttachRespose) {
    writeHeader(_writer, proc.headerSize() + 13, Protocol::MessageType::RAttach, 1);

    _writer.writeLE(byte(81));                      // QID.type
    _writer.writeLE(static_cast<uint32>(3));        // QID.version
    _writer.writeLE(static_cast<uint64>(1049));     // QID.path
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RAttach)
            .then([](Protocol::Response&& response) {
                EXPECT_EQ(81, response.attach.qid.type);
                EXPECT_EQ(3, response.attach.qid.version);
                EXPECT_EQ(1049, response.attach.qid.path);
            });
}


TEST_F(P9Messages, createOpenRequest) {
    Protocol::RequestBuilder(_writer)
            .open(517, Protocol::OpenMode::RDWR)
            .build();

    getRequestOfFail(Protocol::MessageType::TOpen)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(517, request.asOpen().fid);
                ASSERT_EQ(Protocol::OpenMode::RDWR, request.asOpen().mode);
            });
}


TEST_F(P9Messages, createOpenRespose) {
    auto const qid = Protocol::Qid {
                                        8,
                                        13,
                                        323
                                    };

    Protocol::ResponseBuilder(_writer, 1)
            .open(qid, 817)
            .build();

    getResponseOfFail(Protocol::MessageType::ROpen)
            .then([qid](Protocol::Response&& response) {
                ASSERT_EQ(qid, response.open.qid);
                ASSERT_EQ(817, response.open.iounit);
            });
}

TEST_F(P9Messages, parseOpenRespose) {
    auto const qid = Protocol::Qid {
            8,
            71,
            4173
    };
    Protocol::size_type const iounit = 998;

    writeHeader(_writer,
                proc.headerSize() +
                Protocol::Encoder::protocolSize(qid) +
                Protocol::Encoder::protocolSize(iounit),
                Protocol::MessageType::ROpen, 1);
    // qid
    encode9P(_writer, qid);
    // iounit
    _writer.writeLE(iounit);
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::ROpen)
            .then([qid, iounit](Protocol::Response&& response) {
                EXPECT_EQ(qid, response.open.qid);
                EXPECT_EQ(iounit, response.open.iounit);
            });
}



TEST_F(P9Messages, createCreateRequest) {
    Protocol::RequestBuilder(_writer)
            .create(1734, "mcFance", 11, Protocol::OpenMode::EXEC)
            .build();

    getRequestOfFail(Protocol::MessageType::TCreate)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(1734, request.asCreate().fid);
                ASSERT_EQ("mcFance", request.asCreate().name);
                ASSERT_EQ(11, request.asCreate().perm);
                ASSERT_EQ(Protocol::OpenMode::EXEC, request.asCreate().mode);
            });
}

TEST_F(P9Messages, createCreateRespose) {
    auto const qid = Protocol::Qid {
            8,
            13,
            323
    };

    Protocol::ResponseBuilder(_writer, 1)
            .create(qid, 718)
            .build();

    getResponseOfFail(Protocol::MessageType::RCreate)
            .then([qid](Protocol::Response&& response) {
                ASSERT_EQ(qid, response.create.qid);
                ASSERT_EQ(718, response.create.iounit);
            });
}

TEST_F(P9Messages, parseCreateRespose) {
    auto const qid = Protocol::Qid {
            8,
            13,
            323
    };
    Protocol::size_type const iounit = 778;

    writeHeader(_writer,
                proc.headerSize() +
                Protocol::Encoder::protocolSize(qid) +
                Protocol::Encoder::protocolSize(iounit),
                Protocol::MessageType::RCreate, 1);

    encode9P(_writer, qid);
    // iounit
    _writer.writeLE(iounit);
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RCreate)
            .then([qid](Protocol::Response&& response) {
                EXPECT_EQ(qid, response.create.qid);
                EXPECT_EQ(778, response.create.iounit);
            });
}


TEST_F(P9Messages, createReadRequest) {
    Protocol::RequestBuilder(_writer)
            .read(7234, 18, 772)
            .build();

    getRequestOfFail(Protocol::MessageType::TRead)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(7234, request.asRead().fid);
                ASSERT_EQ(18, request.asRead().offset);
                ASSERT_EQ(772, request.asRead().count);
            });
}

TEST_F(P9Messages, createReadRespose) {
    const char content[] = "Good news everyone!";
    auto data = wrapMemory(content);
    Protocol::ResponseBuilder(_writer, 1)
            .read(data)
            .build();

    getResponseOfFail(Protocol::MessageType::RRead)
            .then([data](Protocol::Response&& response) {
                ASSERT_EQ(data, response.read.data);
            });
}

TEST_F(P9Messages, parseReadRespose) {
    auto const messageData = StringLiteral{"This is a very important data d-_^b"};
    const uint32 dataLen = messageData.size();

    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, proc.headerSize() + sizeof(uint32) + dataLen, Protocol::MessageType::RRead, 1);
    // iounit
    _writer.writeLE(dataLen);
    _writer.write(messageData.view());
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RRead)
            .then([dataLen, messageData](Protocol::Response&& response) {
                EXPECT_EQ(dataLen, response.read.data.size());
                EXPECT_EQ(response.read.data, messageData.view());
            });
}


TEST_F(P9Messages, createWriteRequest) {
    const char messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

    Protocol::RequestBuilder(_writer)
            .write(15927, 98, data)
            .build();

    getRequestOfFail(Protocol::MessageType::TWrite)
            .then([data](Protocol::Request&& request) {
                ASSERT_EQ(15927, request.asWrite().fid);
                ASSERT_EQ(98, request.asWrite().offset);
                ASSERT_EQ(data, request.asWrite().data);
            });
}

TEST_F(P9Messages, createWriteRespose) {
    Protocol::ResponseBuilder(_writer, 1)
            .write(71717)
            .build();

    getResponseOfFail(Protocol::MessageType::RWrite)
            .then([](Protocol::Response&& response) {
                ASSERT_EQ(71717, response.write.count);
            });
}

TEST_F(P9Messages, parseWriteRespose) {
    writeHeader(_writer, proc.headerSize() + sizeof(uint32), Protocol::MessageType::RWrite, 1);
    // iounit
    _writer.writeLE(static_cast<uint32>(81177));
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RWrite)
            .then([](Protocol::Response&& response) {
                EXPECT_EQ(81177, response.write.count);
            });
}



TEST_F(P9Messages, createClunkRequest) {
    Protocol::RequestBuilder(_writer)
            .clunk(37509)
            .build();

    getRequestOfFail(Protocol::MessageType::TClunk)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(37509, request.asClunk().fid);
            });
}

TEST_F(P9Messages, createClunkRespose) {
    Protocol::ResponseBuilder(_writer, 1)
            .clunk()
            .build();

    getResponseOfFail(Protocol::MessageType::RClunk);
}

TEST_F(P9Messages, parseClunkRespose) {
    writeHeader(_writer, proc.headerSize(), Protocol::MessageType::RClunk, 1);
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RClunk);
}



TEST_F(P9Messages, createRemoveRequest) {
    Protocol::RequestBuilder(_writer)
            .remove(54329)
            .build();

    getRequestOfFail(Protocol::MessageType::TRemove)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(54329, request.asRemove().fid);
            });
}

TEST_F(P9Messages, createRemoveRespose) {
    Protocol::ResponseBuilder(_writer, 1)
            .remove()
            .build();

    getResponseOfFail(Protocol::MessageType::RRemove);
}

TEST_F(P9Messages, parseRemoveRespose) {
    writeHeader(_writer, proc.headerSize(), Protocol::MessageType::RRemove, 1);
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RRemove);
}



TEST_F(P9Messages, createStatRequest) {
    Protocol::RequestBuilder(_writer)
            .stat(7872)
            .build();

    getRequestOfFail(Protocol::MessageType::TStat)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(7872, request.asStat().fid);
            });
}

TEST_F(P9Messages, createStatRespose) {
    Protocol::Stat stat;
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

    Protocol::ResponseBuilder(_writer, 1)
            .stat(stat)
            .build();

    getResponseOfFail(Protocol::MessageType::RStat)
            .then([stat](Protocol::Response&& response) {
                ASSERT_EQ(stat, response.stat);
            });
}

TEST_F(P9Messages, parseStatRespose) {
    Protocol::Stat stat;
    stat.atime = 21;
    stat.dev = 8828;
    stat.gid = "Some user";
    stat.length = 818177;
    stat.mode = 111;
    stat.mtime = 17;
    stat.name = "File McFileface";
    stat.qid.path = 61;
    stat.qid.type = 15;
    stat.qid.version = 404;
    stat.size = 124;
    stat.type = 1;
    stat.uid = "User McUserface";

    const auto headPosition = _writer.position();
    writeHeader(_writer, 0, Protocol::MessageType::RStat, 1);
    _writer.writeLE(uint16(1));
    encode9P(_writer, stat);

    const auto totalSize = _writer.position();
    _writer.limit(totalSize);
    _writer.reset(headPosition);
    _writer.writeLE(Protocol::size_type(totalSize));

    getResponseOfFail(Protocol::MessageType::RStat)
            .then([stat](Protocol::Response&& response) {
                ASSERT_EQ(stat, response.stat);
            });
}


TEST_F(P9Messages, createWStatRequest) {
    Protocol::Stat stat;
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

    Protocol::RequestBuilder(_writer)
            .writeStat(8193, stat)
            .build();

    getRequestOfFail(Protocol::MessageType::TWStat)
            .then([stat](Protocol::Request&& request) {
                ASSERT_EQ(8193, request.asWstat().fid);
                ASSERT_EQ(stat, request.asWstat().stat);
            });
}

TEST_F(P9Messages, createWStatRespose) {
    Protocol::ResponseBuilder(_writer, 1)
            .wstat()
            .build();

    getResponseOfFail(Protocol::MessageType::RWStat);
}

TEST_F(P9Messages, parseWStatRespose) {
    writeHeader(_writer, proc.headerSize(), Protocol::MessageType::RWStat, 1);
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RWStat);
}




TEST_F(P9Messages, createWalkRequest) {
    auto const destPath = makePath("space", "knowhere");
    Protocol::RequestBuilder(_writer)
            .walk(213, 124, destPath)
            .build();

    getRequestOfFail(Protocol::MessageType::TWalk)
            .then([&destPath](Protocol::Request&& request) {
                EXPECT_EQ(213, request.asWalk().fid);
                EXPECT_EQ(124, request.asWalk().newfid);
                EXPECT_EQ(destPath, request.asWalk().path);
            });
}

TEST_F(P9Messages, createWalkEmptyPathRequest) {
    Protocol::RequestBuilder(_writer)
            .walk(7374, 542, Path())
            .build();

    getRequestOfFail(Protocol::MessageType::TWalk)
            .then([](Protocol::Request&& request) {
                ASSERT_EQ(7374, request.asWalk().fid);
                ASSERT_EQ(542, request.asWalk().newfid);
                ASSERT_TRUE(request.asWalk().path.empty());
            });
}


TEST_F(P9Messages, createWalkRespose) {
    auto qids = makeArray<Protocol::Qid>(3);
    qids[2].path = 21;
    qids[2].version = 117;
    qids[2].type = 81;
    Protocol::ResponseBuilder(_writer, 1)
            .walk(qids)
            .build();

    getResponseOfFail(Protocol::MessageType::RWalk)
            .then([&qids](Protocol::Response&& response) {
                ASSERT_EQ(qids.size(), response.walk.nqids);
                ASSERT_EQ(qids[2], response.walk.qids[2]);
            });
}

TEST_F(P9Messages, parseWalkRespose) {
    auto const qid = Protocol::Qid {
            87,
            5481,
            17
    };

    // Set declared message size to be more then negotiated message size
    const auto headPosition = _writer.position();
    writeHeader(_writer, 0, Protocol::MessageType::RWalk, 1);
    // nwqid
    _writer.writeLE(uint16(1));
    encode9P(_writer, qid);


    const auto totalSize = _writer.position();
    _writer.limit(totalSize);
    _writer.reset(headPosition);
    _writer.writeLE(Protocol::size_type(totalSize));

    getResponseOfFail(Protocol::MessageType::RWalk)
            .then([](Protocol::Response&& response) {
                EXPECT_EQ(1, response.walk.nqids);
                EXPECT_EQ(87, response.walk.qids[0].type);
                EXPECT_EQ(5481, response.walk.qids[0].version);
                EXPECT_EQ(17, response.walk.qids[0].path);
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

    Protocol::RequestBuilder(_writer)
            .session(data)
            .build();

    getRequestOfFail(Protocol::MessageType::TSession)
            .then([data](Protocol::Request&& request) {
                ASSERT_EQ(data, wrapMemory(request.asSession().key));
            });
}

TEST_F(P9E_Messages, createSessionRequest_NotEnoughData) {
    const byte sessionKey[5] = {8, 7, 6, 5, 4};

    ASSERT_THROW(Protocol::RequestBuilder(_writer)
                 .session(wrapMemory(sessionKey)),
                 Solace::Exception);
}

TEST_F(P9E_Messages, parseSessionRequest_NotEnoughData) {
    const byte sessionKey[5] = {8, 7, 6, 5, 4};
    auto keyData = wrapMemory(sessionKey);

    // Set declared message size to be more then negotiated message size
    Protocol::Encoder(_writer)
            .header(Protocol::MessageType::TSession, 1, keyData.size());
    _writer.write(keyData);

    auto headerResult = proc.parseMessageHeader(_reader);
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TSession, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _reader);
    ASSERT_TRUE(message.isError());
}

TEST_F(P9E_Messages, createSessionRespose) {
    Protocol::ResponseBuilder(_writer, 1)
            .session()
            .build();

    getResponseOfFail(Protocol::MessageType::RSession);
}

TEST_F(P9E_Messages, parseSessionRespose) {
    // Set declared message size to be more then negotiated message size
    Protocol::Encoder(_writer)
            .header(Protocol::MessageType::RSession, 1);
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RSession);
}



TEST_F(P9E_Messages, createShortReadRequest) {
    const auto path = Path::parse("some/wierd/place").unwrap();
    Protocol::RequestBuilder(_writer)
            .shortRead(32, path)
            .build();

    getRequestOfFail(Protocol::MessageType::TSRead)
        .then([&path](Protocol::Request&& request) {
            ASSERT_EQ(32, request.asShortRead().fid);
            ASSERT_EQ(path, request.asShortRead().path);
        });
}


TEST_F(P9E_Messages, createShortReadRespose) {
    const char messageData[] = "This was somewhat important data d^_-b";
    auto data = wrapMemory(messageData);
    Protocol::ResponseBuilder(_writer, 1)
            .shortRead(data)
            .build();

    getResponseOfFail(Protocol::MessageType::RSRead)
            .then([data](Protocol::Response&& response) {
                EXPECT_EQ(data, response.read.data);
            });
}


TEST_F(P9E_Messages, parseShortReadRespose) {
    auto const messageData = StringLiteral{"This is a very important data d-_^b"};
    const uint32 dataLen = messageData.size();

    // Set declared message size to be more then negotiated message size
    writeHeader(_writer, proc.headerSize() + sizeof(uint32) + dataLen, Protocol::MessageType::RSRead, 1);
    // iounit
    _writer.writeLE(dataLen);
    _writer.write(messageData.view());
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RSRead)
            .then([messageData](Protocol::Response&& response) {
                EXPECT_EQ(messageData.size(), response.read.data.size());
                EXPECT_EQ(messageData.view(), response.read.data);
            });
}


TEST_F(P9E_Messages, createShortWriteRequest) {
    const auto path = Path::parse("some/wierd/place").unwrap();
    const char messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

    Protocol::RequestBuilder(_writer)
            .shortWrite(32, path, data)
            .build();

    getRequestOfFail(Protocol::MessageType::TSWrite)
        .then([&path, data](Protocol::Request&& request) {
            ASSERT_EQ(32, request.asShortWrite().fid);
            ASSERT_EQ(path, request.asShortWrite().path);
            ASSERT_EQ(data, request.asShortWrite().data);
        });
}


TEST_F(P9E_Messages, createShortWriteRespose) {
    Protocol::ResponseBuilder(_writer, 1)
            .shortWrite(100500)
            .build();

    getResponseOfFail(Protocol::MessageType::RSWrite)
            .then([](Protocol::Response&& response) {
                EXPECT_EQ(100500, response.write.count);
            });
}


TEST_F(P9E_Messages, parseShortWriteRespose) {
    Protocol::Encoder(_writer)
            .header(Protocol::MessageType::RSWrite, 1, sizeof(uint32))
            .encode(static_cast<uint32>(81177));
    _writer.flip();

    getResponseOfFail(Protocol::MessageType::RSWrite)
            .then([](Protocol::Response&& response) {
                EXPECT_EQ(81177, response.write.count);
            });
}
