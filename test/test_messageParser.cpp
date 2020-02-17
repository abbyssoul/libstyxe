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
 * @file: test/test_messageParser.cpp
 * @author: soultaker
 *
 *******************************************************************************/
#include "styxe/messageParser.hpp"

#include <solace/output_utils.hpp>

#include <gtest/gtest.h>


using namespace Solace;
using namespace styxe;


TEST(P9, testHeaderSize) {
	ASSERT_EQ(4u + 1u + 2u, headerSize());
}


TEST(P9, canParsingValidMessageHeader) {
	// Form a normal message with no data:
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{byteStream};
	encoder << makeHeaderWithPayload(asByte(MessageType::TVersion), 1, 0);

	auto reader = ByteReader{byteStream.viewWritten()};
	auto res = parseMessageHeader(reader);
	ASSERT_TRUE(res.isOk());

	auto header = res.unwrap();
	ASSERT_EQ(4u + 1u + 2u, header.messageSize);
	ASSERT_EQ(asByte(MessageType::TVersion), header.type);
	ASSERT_EQ(1, header.tag);
}


TEST(P9, parsingMessageHeaderWithUnknownMessageType) {
	// Form a normal message with no data:
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{byteStream};
	encoder << makeHeaderWithPayload(255, 1, 0);

	auto reader = ByteReader{byteStream.viewWritten()};
	ASSERT_TRUE(parseMessageHeader(reader).isOk());
}


TEST(P9, failParsingHeaderWithInsufficientData) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};

	// Only write one header field. Should be not enough data to read a header.
	byteStream.writeLE(headerSize());  // type and tag are not written deliberately

	auto reader = ByteReader{byteStream.viewWritten()};
	auto maybeHeader = parseMessageHeader(reader);
	ASSERT_TRUE(maybeHeader.isError());
}


TEST(P9, testParsingIllformedMessageHeader) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};
	// Set declared message size less then header size.
	byteStream.writeLE(byte{3});
	byteStream.writeLE(asByte(MessageType::TVersion));
	byteStream.writeLE(Tag{1});

	auto reader = ByteReader{byteStream.viewWritten()};
	ASSERT_TRUE(parseMessageHeader(reader).isError());
}


TEST(P9, parsingIllFormedHeaderForMessagesLargerMTUShouldError) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};

	styxe::Encoder encoder{byteStream};
	// Set declared message size to be more then negotiated message size
	encoder << makeHeaderWithPayload(asByte(MessageType::TVersion), 1, 100);

	auto reader = ByteReader{byteStream.viewWritten()};
	auto maybeHeader = parseMessageHeader(reader);
	ASSERT_TRUE(maybeHeader.isOk());

	EXPECT_FALSE(validateHeader(*maybeHeader, 16, 128));
}



TEST(P9, parseIncorrectlySizedSmallerResponse) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};

	// Set declared message size to be more then negotiated message size
	styxe::Encoder encoder{byteStream};
	encoder << makeHeaderWithPayload(asByte(MessageType::RVersion), 1, sizeof(int32))
			<< byte{3};  // The payload is just 1 byte and not 4 as declared by the header.

	auto reader = ByteReader{byteStream.viewWritten()};
	auto header = parseMessageHeader(reader);
	ASSERT_TRUE(header.isOk());

	auto message = parseVersionRequest(*header, reader, 12);
	ASSERT_TRUE(message.isError());
}


TEST(P9, parseIncorrectlySizedLargerResponse) {
	byte buffer[16];
	auto byteStream = ByteWriter{wrapMemory(buffer)};

	// Set declared message size to be more then negotiated message size
	styxe::Encoder encoder{byteStream};
	encoder << makeHeaderWithPayload(asByte(MessageType::RVersion), 1, sizeof(int32))
			<< uint64{999999};  // The payload is 8 bytes and not 4 as declared by the header.

	auto reader = ByteReader{byteStream.viewWritten()};
	auto header = parseMessageHeader(reader);
	ASSERT_TRUE(header.isOk());

	auto message = parseVersionRequest(*header, reader, 12);
	ASSERT_TRUE(message.isError());
}


TEST(P9, creatingUnsupportedVersionParserShouldFail) {
	ASSERT_TRUE(createRequestParser("Fancy", 128).isError());
	ASSERT_TRUE(createResponseParser("Style", 64).isError());
}
