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

#include "gtest/gtest.h"


using namespace Solace;
using namespace styxe;


inline uint16 operator "" _us(unsigned long long value) {  // NOLINT(runtime/int)
    return static_cast<uint16>(value);
}

void encode9P(ByteBuffer& dest, const Protocol::Stat& stat) {
    dest << stat.size;
    dest << stat.type;
    dest << stat.dev;

    dest << stat.qid.type;
    dest << stat.qid.version;
    dest << stat.qid.path;

    dest << stat.mode;
    dest << stat.atime;
    dest << stat.mtime;
    dest << stat.length;

    dest << static_cast<uint16>(stat.name.size());
    dest.write(stat.name.view());

    dest << static_cast<uint16>(stat.uid.size());
    dest.write(stat.uid.view());

    dest << static_cast<uint16>(stat.gid.size());
    dest.write(stat.gid.view());

    dest << static_cast<uint16>(stat.muid.size());
    dest.write(stat.muid.view());
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
    ByteBuffer buffer(_mem.create(512));
    buffer << Protocol::size_type(4 + 1 + 2);
    buffer << static_cast<byte>(Protocol::MessageType::TVersion);
    buffer << Protocol::Tag(1);

    auto res = proc.parseMessageHeader(buffer.flip());
    ASSERT_TRUE(res.isOk());

    auto header = res.unwrap();
    ASSERT_EQ(4u + 1u + 2u, header.size);
    ASSERT_EQ(Protocol::MessageType::TVersion, header.type);
    ASSERT_EQ(1_us, header.tag);
}


TEST(P9_2000, parsingMessageHeaderWithUnknownMessageType) {
    MemoryManager _mem(1024);
    Protocol proc;

    // Form a normal message with no data:
    ByteBuffer buffer(_mem.create(512));
    buffer << Protocol::size_type(4 + 1 + 2);
    buffer << static_cast<byte>(-1);
    buffer << Protocol::Tag(1);

    ASSERT_TRUE(proc.parseMessageHeader(buffer.flip()).isError());
}

TEST(P9_2000, testParsingHeaderWithInsufficientData) {
    MemoryManager _mem(1024);
    Protocol proc;

    ByteBuffer buffer(_mem.create(512));
    // Only write one header field. Should be not enough data to read a header.
    buffer << Protocol::size_type(4 + 1 + 2);
    auto res = proc.parseMessageHeader(buffer.flip());

    ASSERT_TRUE(res.isError());
}


TEST(P9_2000, testParsingIllformedMessageHeader) {
    MemoryManager _mem(1024);

    ByteBuffer buffer(_mem.create(512));
    // Set declared message size less then header size.
    buffer << Protocol::size_type(1 + 2);
    buffer << static_cast<byte>(Protocol::MessageType::TVersion);
    buffer << Protocol::Tag(1);

    Protocol proc;
    ASSERT_TRUE(proc.parseMessageHeader(buffer.flip()).isError());
}

TEST(P9_2000, parsingIllFormedHeaderForMessagesLargerMTUShouldError) {
    MemoryManager _mem(1024);
    Protocol proc;

    ByteBuffer buffer(_mem.create(512));
    proc.maxNegotiatedMessageSize(20);
    // Set declared message size to be more then negotiated message size
    buffer << Protocol::size_type(proc.maxNegotiatedMessageSize() + 100);
    buffer << static_cast<byte>(Protocol::MessageType::TVersion);
    buffer << Protocol::Tag(1);

    ASSERT_TRUE(proc.parseMessageHeader(buffer.flip()).isError());
}


TEST(P9_2000, parseIncorrectlySizedSmallerResponse) {
    MemoryManager _mem(1024);
    Protocol proc;

    ByteBuffer buffer(_mem.create(512));
    // Set declared message size to be more then negotiated message size
    buffer << Protocol::size_type(proc.headerSize() + sizeof(int32));
    buffer << static_cast<byte>(Protocol::MessageType::RVersion);
    buffer << Protocol::Tag(1);
    buffer << byte(3);

    auto header = proc.parseMessageHeader(buffer.flip());
    ASSERT_TRUE(header.isOk());

    auto message = proc.parseResponse(header.unwrap(), buffer);
    ASSERT_TRUE(message.isError());
}

TEST(P9_2000, parseIncorrectlySizedLargerResponse) {
    MemoryManager _mem(1024);
    Protocol proc;

    ByteBuffer buffer(_mem.create(proc.headerSize() + sizeof(int32)*2));
    // Set declared message size to be more then negotiated message size
    buffer << Protocol::size_type(proc.headerSize() + sizeof(int32));
    buffer << static_cast<byte>(Protocol::MessageType::RVersion);
    buffer << Protocol::Tag(1);
    buffer << int64(999999);

    auto header = proc.parseMessageHeader(buffer.flip());
    ASSERT_TRUE(header.isOk());

    auto message = proc.parseResponse(header.unwrap(), buffer);
    ASSERT_TRUE(message.isError());
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 9P2000 Message parsing test suit
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class P9Messages : public ::testing::Test {
public:
    P9Messages() :
        _memManager(Protocol::MAX_MESSAGE_SIZE)
    {}


protected:

    // cppcheck-suppress unusedFunction
    void SetUp() override {
        _buffer = _memManager.create(Protocol::MAX_MESSAGE_SIZE);
        _buffer.viewRemaining().fill(0xFE);
    }


    MemoryManager   _memManager;
    ByteBuffer      _buffer;
};




TEST_F(P9Messages, createVersionRequest) {
    const auto testVersion = Protocol::PROTOCOL_VERSION;
    const auto versionStringLen = static_cast<uint32>(testVersion.size());

    Protocol proc;
    auto requestBuilder = Protocol::RequestBuilder(_buffer);
    requestBuilder.version(testVersion);

    ASSERT_EQ(proc.headerSize() + 4 + 2 + versionStringLen, _buffer.position());

    auto headerResult = proc.parseMessageHeader(requestBuilder.build());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TVersion, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(proc.maxPossibleMessageSize(), request.asVersion().msize);
    ASSERT_EQ(testVersion, request.asVersion().version);
}

TEST_F(P9Messages, createVersionRespose) {
    Protocol::ResponseBuilder(_buffer, Protocol::NO_TAG)
            .version("9Pe", 718);

    Protocol proc;
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RVersion, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    ASSERT_EQ(718, response.version.msize);
    ASSERT_EQ("9Pe", response.version.version);
}

TEST_F(P9Messages, parseVersionRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize() + sizeof(int32) + sizeof(int16) + 2;
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RVersion);
    _buffer << Protocol::Tag(1);
    _buffer << int32(512);
    _buffer << int16(2);
    _buffer.write(StringLiteral("9P").view());

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RVersion, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();

    ASSERT_EQ(512, response.version.msize);
    ASSERT_EQ("9P", response.version.version);
}


TEST_F(P9Messages, createAuthRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
        .auth(312, "User mcUsers", "Somewhere near");
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TAuth, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(312, request.asAuth().afid);
    ASSERT_EQ("User mcUsers", request.asAuth().uname);
    ASSERT_EQ("Somewhere near", request.asAuth().aname);
}


TEST_F(P9Messages, createAuthRespose) {
    Protocol proc;

    Protocol::Qid qid;
    qid.path = 8187;
    qid.type = 71;
    qid.version = 17;
    Protocol::ResponseBuilder(_buffer, 1)
            .auth(qid);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RAuth, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();

    ASSERT_EQ(qid, response.auth.qid);
}


TEST_F(P9Messages, parseAuthRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize() + 13;
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RAuth);
    _buffer << Protocol::Tag(1);

    _buffer << byte(13);     // QID.type
    _buffer << uint32(91);   // QID.version
    _buffer << uint64(441);  // QID.path

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RAuth, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(13, response.auth.qid.type);
    EXPECT_EQ(91, response.auth.qid.version);
    EXPECT_EQ(441, response.auth.qid.path);
}

// No such thing as error request!

TEST_F(P9Messages, createErrorRespose) {
    Protocol proc;

    StringView const testError = "Something went right :)";
    Protocol::ResponseBuilder(_buffer, 3)
            .error(testError);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RError, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    ASSERT_EQ(testError, response.error.ename);
}

TEST_F(P9Messages, parseErrorRespose) {
    Protocol proc;

    StringView const expectedErrorMessage{"All good!"};
    // Set declared message size to be more then negotiated message size
    Protocol::Encoder(_buffer)
            .header(Protocol::MessageType::RError, 1, Protocol::Encoder::protocolSize(expectedErrorMessage))
            .encode(expectedErrorMessage);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RError, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
//    EXPECT_STREQ(expectedErrorMessage, response.toString().c_str());
    EXPECT_EQ(expectedErrorMessage, response.error.ename);
}


TEST_F(P9Messages, createFlushRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .flush(7711);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TFlush, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(7711, request.asFlush().oldtag);
}


TEST_F(P9Messages, createFlushRespose) {
    Protocol proc;

    Protocol::ResponseBuilder(_buffer, 1)
            .flush();

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RFlush, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}


TEST_F(P9Messages, parseFlushRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize();
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RFlush);
    _buffer << Protocol::Tag(1);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RFlush, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}


TEST_F(P9Messages, createAttachRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .attach(3310, 1841, "McFace", "close to u");
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TAttach, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(3310, request.asAttach().fid);
    ASSERT_EQ(1841, request.asAttach().afid);
    ASSERT_EQ("McFace", request.asAttach().uname);
    ASSERT_EQ("close to u", request.asAttach().aname);
}

TEST_F(P9Messages, createAttachRespose) {
    Protocol proc;

    Protocol::Qid qid;
    qid.path = 7771;
    qid.type = 91;
    qid.version = 3;
    Protocol::ResponseBuilder(_buffer, 1)
            .attach(qid);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RAttach, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    ASSERT_EQ(qid, response.attach.qid);
}

TEST_F(P9Messages, parseAttachRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize() + 13;
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RAttach);
    _buffer << Protocol::Tag(1);

    _buffer << byte(81);     // QID.type
    _buffer << uint32(3);   // QID.version
    _buffer << uint64(1049);  // QID.path

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RAttach, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(81, response.attach.qid.type);
    EXPECT_EQ(3, response.attach.qid.version);
    EXPECT_EQ(1049, response.attach.qid.path);
}


TEST_F(P9Messages, createOpenRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .open(517, Protocol::OpenMode::RDWR);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TOpen, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(517, request.asOpen().fid);
    ASSERT_EQ(Protocol::OpenMode::RDWR, request.asOpen().mode);
}


TEST_F(P9Messages, createOpenRespose) {
    Protocol proc;

    Protocol::Qid qid;
    qid.path = 323;
    qid.type = 8;
    qid.version = 13;
    Protocol::ResponseBuilder(_buffer, 1)
            .open(qid, 817);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::ROpen, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();

    ASSERT_EQ(qid, response.open.qid);
    ASSERT_EQ(817, response.open.iounit);
}

TEST_F(P9Messages, parseOpenRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize() + 13 + sizeof(uint32);
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::ROpen);
    _buffer << Protocol::Tag(1);
    // qid
    _buffer << byte(71);     // QID.type
    _buffer << uint32(33);   // QID.version
    _buffer << uint64(4173);  // QID.path
    // iounit
    _buffer << uint32(998);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::ROpen, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(71, response.open.qid.type);
    EXPECT_EQ(33, response.open.qid.version);
    EXPECT_EQ(4173, response.open.qid.path);
    EXPECT_EQ(998, response.open.iounit);
}



TEST_F(P9Messages, createCreateRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .create(1734, "mcFance", 11, Protocol::OpenMode::EXEC);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TCreate, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(1734, request.asCreate().fid);
    ASSERT_EQ("mcFance", request.asCreate().name);
    ASSERT_EQ(11, request.asCreate().perm);
    ASSERT_EQ(Protocol::OpenMode::EXEC, request.asCreate().mode);
}

TEST_F(P9Messages, createCreateRespose) {
    Protocol proc;

    Protocol::Qid qid;
    qid.path = 323;
    qid.type = 8;
    qid.version = 13;
    Protocol::ResponseBuilder(_buffer, 1)
            .create(qid, 718);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RCreate, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();

    ASSERT_EQ(qid, response.create.qid);
    ASSERT_EQ(718, response.create.iounit);
}

TEST_F(P9Messages, parseCreateRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize() + 13 + sizeof(uint32);
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RCreate);
    _buffer << Protocol::Tag(1);
    // qid
    _buffer << byte(87);     // QID.type
    _buffer << uint32(5481);   // QID.version
    _buffer << uint64(17);  // QID.path
    // iounit
    _buffer << uint32(778);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RCreate, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(87, response.create.qid.type);
    EXPECT_EQ(5481, response.create.qid.version);
    EXPECT_EQ(17, response.create.qid.path);
    EXPECT_EQ(778, response.create.iounit);
}


TEST_F(P9Messages, createReadRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .read(7234, 18, 772);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TRead, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(7234, request.asRead().fid);
    ASSERT_EQ(18, request.asRead().offset);
    ASSERT_EQ(772, request.asRead().count);
}

TEST_F(P9Messages, createReadRespose) {
    Protocol proc;

    const char content[] = "Good news everyone!";
    auto data = wrapMemory(content);
    Protocol::ResponseBuilder(_buffer, 1)
            .read(data);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RRead, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();

    ASSERT_EQ(data, response.read.data);
}

TEST_F(P9Messages, parseReadRespose) {
    Protocol proc;

    StringLiteral messageData = "This is a very important data d-_^b";
    const uint32 dataLen = messageData.size();
    const auto messageSize = proc.headerSize() + sizeof(uint32) + dataLen;

    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RRead);
    _buffer << Protocol::Tag(1);
    // iounit
    _buffer << dataLen;
    _buffer.write(messageData.view());

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RRead, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(dataLen, response.read.data.size());
    EXPECT_EQ(response.read.data, messageData.view());
}


TEST_F(P9Messages, createWriteRequest) {
    Protocol proc;

    const char messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

    Protocol::RequestBuilder(_buffer)
            .write(15927, 98, data);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TWrite, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(15927, request.asWrite().fid);
    ASSERT_EQ(98, request.asWrite().offset);
    ASSERT_EQ(data, request.asWrite().data);
}

TEST_F(P9Messages, createWriteRespose) {
    MemoryManager _mem(1024);
    Protocol proc;

    Protocol::ResponseBuilder(_buffer, 1)
            .write(71717);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RWrite, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();

    ASSERT_EQ(71717, response.write.count);
}

TEST_F(P9Messages, parseWriteRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize() + sizeof(uint32);
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RWrite);
    _buffer << Protocol::Tag(1);
    // iounit
    _buffer << uint32(81177);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RWrite, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(81177, response.write.count);
}



TEST_F(P9Messages, createClunkRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .clunk(37509);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TClunk, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(37509, request.asClunk().fid);
}

TEST_F(P9Messages, createClunkRespose) {
    Protocol proc;
    Protocol::ResponseBuilder(_buffer, 1)
            .clunk();

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RClunk, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}

TEST_F(P9Messages, parseClunkRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize();
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RClunk);
    _buffer << Protocol::Tag(1);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RClunk, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}



TEST_F(P9Messages, createRemoveRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .remove(54329);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TRemove, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(54329, request.asRemove().fid);
}

TEST_F(P9Messages, createRemoveRespose) {
    Protocol proc;
    Protocol::ResponseBuilder(_buffer, 1)
            .remove();

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RRemove, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}

TEST_F(P9Messages, parseRemoveRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize();
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RRemove);
    _buffer << Protocol::Tag(1);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RRemove, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}



TEST_F(P9Messages, createStatRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .stat(7872);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TStat, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(7872, request.asStat().fid);
}

TEST_F(P9Messages, createStatRespose) {
    Protocol proc;

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

    Protocol::ResponseBuilder(_buffer, 1)
            .stat(stat);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RStat, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();

    ASSERT_EQ(stat, response.stat);
}

TEST_F(P9Messages, parseStatRespose) {
    Protocol proc;

    // Set declared message size to be more then negotiated message size
    const auto headPosition = _buffer.position();
    _buffer << Protocol::size_type(0);
    _buffer << static_cast<byte>(Protocol::MessageType::RStat);
    _buffer << Protocol::Tag(1);
    _buffer << uint16(1);

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

    encode9P(_buffer, stat);
    const auto totalSize = _buffer.position();
    _buffer.reset(headPosition);
    _buffer << Protocol::size_type(totalSize);
    _buffer.reset(totalSize);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RStat, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(stat, response.stat);
}


TEST_F(P9Messages, createWStatRequest) {
    Protocol proc;

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

    Protocol::RequestBuilder(_buffer)
            .writeStat(8193, stat);
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TWStat, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(8193, request.asWstat().fid);
    ASSERT_EQ(stat, request.asWstat().stat);
}

TEST_F(P9Messages, createWStatRespose) {
    Protocol proc;

    Protocol::ResponseBuilder(_buffer, 1)
            .wstat();

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RWStat, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}

TEST_F(P9Messages, parseWStatRespose) {
    Protocol proc;

    const auto messageSize = proc.headerSize();
    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RWStat);
    _buffer << Protocol::Tag(1);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RWStat, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}




TEST_F(P9Messages, createWalkRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .walk(213, 124, {"knowhere"});
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TWalk, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(213, request.asWalk().fid);
    ASSERT_EQ(124, request.asWalk().newfid);
    ASSERT_STREQ("knowhere", request.asWalk().path.toString().c_str());
}

TEST_F(P9Messages, createWalkEmptyPathRequest) {
    Protocol proc;

    Protocol::RequestBuilder(_buffer)
            .walk(7374, 542, Path());
    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TWalk, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(7374, request.asWalk().fid);
    ASSERT_EQ(542, request.asWalk().newfid);
    ASSERT_TRUE(request.asWalk().path.empty());
}


TEST_F(P9Messages, createWalkRespose) {
    Protocol proc;

    Array<Protocol::Qid> qids(3);
    qids[2].path = 21;
    qids[2].version = 117;
    qids[2].type = 81;
    Protocol::ResponseBuilder(_buffer, 1)
            .walk(qids);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RWalk, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();

    ASSERT_EQ(qids.size(), response.walk.nqids);
    ASSERT_EQ(qids[2], response.walk.qids[2]);
}

TEST_F(P9Messages, parseWalkRespose) {
    Protocol proc;

    // Set declared message size to be more then negotiated message size
    const auto headPosition = _buffer.position();
    _buffer << Protocol::size_type(0);
    _buffer << static_cast<byte>(Protocol::MessageType::RWalk);
    _buffer << Protocol::Tag(1);
    // nwqid
    _buffer << uint16(1);
    // qid
    _buffer << byte(87);     // QID.type
    _buffer << uint32(5481);   // QID.version
    _buffer << uint64(17);  // QID.path


    const auto totalSize = _buffer.position();
    _buffer.reset(headPosition);
    _buffer << Protocol::size_type(totalSize);
    _buffer.reset(totalSize);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RWalk, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(1, response.walk.nqids);
    EXPECT_EQ(87, response.walk.qids[0].type);
    EXPECT_EQ(5481, response.walk.qids[0].version);
    EXPECT_EQ(17, response.walk.qids[0].path);
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

    Protocol proc;
    Protocol::RequestBuilder(_buffer)
            .session(data);

    ASSERT_EQ(proc.headerSize() + 8, _buffer.position());

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TSession, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(data, wrapMemory(request.asSession().key));
}

TEST_F(P9E_Messages, createSessionRequest_NotEnoughData) {
    const byte sessionKey[5] = {8, 7, 6, 5, 4};

    ASSERT_THROW(Protocol::RequestBuilder(_buffer)
                 .session(wrapMemory(sessionKey)),
                 Solace::Exception);
}

TEST_F(P9E_Messages, parseSessionRequest_NotEnoughData) {
    const byte sessionKey[5] = {8, 7, 6, 5, 4};
    auto keyData = wrapMemory(sessionKey);

    // Set declared message size to be more then negotiated message size
    Protocol proc;
    Protocol::Encoder(_buffer)
            .header(Protocol::MessageType::TSession, 1, keyData.size());
    _buffer.write(keyData);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TSession, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isError());
}

TEST_F(P9E_Messages, createSessionRespose) {
    Protocol proc;

    Protocol::ResponseBuilder(_buffer, 1)
            .session();

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RSession, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}

TEST_F(P9E_Messages, parseSessionRespose) {
    Protocol proc;

    // Set declared message size to be more then negotiated message size
    Protocol::Encoder(_buffer)
            .header(Protocol::MessageType::RSession, 1);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RSession, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());
}



TEST_F(P9E_Messages, createShortReadRequest) {
    const auto path = Path::parse("some/wierd/place").unwrap();

    Protocol proc;
    Protocol::RequestBuilder(_buffer)
            .shortRead(32, path);

    ASSERT_EQ(proc.headerSize() + sizeof(Protocol::Fid) + sizeof (uint16)
              + 3*sizeof (uint16) + (4 + 5 + 5), _buffer.position());

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TSRead, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(32, request.asShortRead().fid);
    ASSERT_EQ(path, request.asShortRead().path);
}


TEST_F(P9E_Messages, createShortReadRespose) {
    Protocol proc;

    const char messageData[] = "This was somewhat important data d^_-b";
    auto data = wrapMemory(messageData);
    Protocol::ResponseBuilder(_buffer, 1)
            .shortRead(data);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RSRead, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(data, response.read.data);
}


TEST_F(P9E_Messages, parseShortReadRespose) {
    Protocol proc;

    StringLiteral messageData = "This is a very important data d-_^b";
    const uint32 dataLen = messageData.size();
    const auto messageSize = proc.headerSize() + sizeof(uint32) + dataLen;

    // Set declared message size to be more then negotiated message size
    _buffer << Protocol::size_type(messageSize);
    _buffer << static_cast<byte>(Protocol::MessageType::RSRead);
    _buffer << Protocol::Tag(1);
    // iounit
    _buffer << dataLen;
    _buffer.write(messageData.view());

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RSRead, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(dataLen, response.read.data.size());
    EXPECT_EQ(response.read.data, messageData.view());
}


TEST_F(P9E_Messages, createShortWriteRequest) {
    const auto path = Path::parse("some/wierd/place").unwrap();
    const char messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

    Protocol proc;
    Protocol::RequestBuilder(_buffer)
            .shortWrite(32, path, data);

    ASSERT_EQ(proc.headerSize() + sizeof(Protocol::Fid) + sizeof (uint16)
              + 3*sizeof (uint16) + (4 + 5 + 5) +
              sizeof(Protocol::size_type) + data.size(), _buffer.position());

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::TSWrite, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseRequest(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto request = message.moveResult();
    ASSERT_EQ(32, request.asShortWrite().fid);
    ASSERT_EQ(path, request.asShortWrite().path);
    ASSERT_EQ(data, request.asShortWrite().data);
}


TEST_F(P9E_Messages, createShortWriteRespose) {
    Protocol proc;

    Protocol::ResponseBuilder(_buffer, 1)
            .shortWrite(100500);

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RSWrite, header.type);

    // Make sure we can parse the message back.
    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(100500, response.write.count);
}


TEST_F(P9E_Messages, parseShortWriteRespose) {
    Protocol proc;

    // Set declared message size to be more then negotiated message size
    Protocol::Encoder(_buffer)
            .header(Protocol::MessageType::RSWrite, 1, sizeof(uint32))
            .encode(static_cast<uint32>(81177));

    auto headerResult = proc.parseMessageHeader(_buffer.flip());
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
    ASSERT_EQ(Protocol::MessageType::RSWrite, header.type);

    auto message = proc.parseResponse(header, _buffer);
    ASSERT_TRUE(message.isOk());

    auto response = message.moveResult();
    EXPECT_EQ(81177, response.write.count);
}
