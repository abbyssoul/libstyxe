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
 * @file: test/test_9P2000e.cpp
 * Specific test 9P2000.e
 *******************************************************************************/
#include "styxe/messageWriter.hpp"
#include "styxe/messageParser.hpp"

#include "styxe/print.hpp"
#include <solace/output_utils.hpp>

#include <gtest/gtest.h>


using namespace Solace;
using namespace styxe;

class P9E_Messages
        : public ::testing::Test {
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

		auto _maybeParser = createParser(kMaxMesssageSize, _9P2000E::kProtocolVersion);
		if (!_maybeParser) {
			return _maybeParser.moveError();
		}
		auto& parser = _maybeParser.unwrap();

		auto const expectType = messageCode(RequestType{});
		return parser.parseMessageHeader(_reader)
                .then([expectType](MessageHeader&& header) {
                    return (header.type != expectType)
					? Result<MessageHeader, Error>{getCannedError(CannedError::UnsupportedMessageType)}
					: Result<MessageHeader, Error>{types::okTag, std::move(header)};
                })
				.then([&parser, this](MessageHeader&& header) {
					return parser.parseRequest(header, _reader);
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
						FAIL() << e;
                    } ();

                    return e;
                });
    }


    template<typename ResponseType>
    Result<ResponseType, Error>
	getResponseOrFail() {
        _reader.limit(_writer.limit());

		auto _maybeParser = createParser(kMaxMesssageSize, _9P2000E::kProtocolVersion);
		if (!_maybeParser) {
			return _maybeParser.moveError();
		}
		auto& parser = _maybeParser.unwrap();

		auto const expectType = messageCode(ResponseType{});
		return parser.parseMessageHeader(_reader)
                .then([expectType](MessageHeader&& header) {
                    return (header.type != expectType)
					? Result<MessageHeader, Error>{types::errTag, getCannedError(CannedError::UnsupportedMessageType)}
					: Result<MessageHeader, Error>{types::okTag, std::move(header)};
                })
				.then([&parser, this](MessageHeader&& header) {
					return parser.parseResponse(header, _reader);
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
                    [&e]() {
                        FAIL() << e.toString();
                    } ();

                    return e;
                });
    }

protected:

    MemoryManager   _memManager {kMaxMesssageSize};
	MemoryResource  _memBuf{_memManager.allocate(kMaxMesssageSize).unwrap()};
    ByteWriter      _writer{_memBuf};
    ByteReader      _reader{_memBuf};

	MessageWriter	_messageWriter{_writer};
};


TEST_F(P9E_Messages, createSessionRequest) {
	_messageWriter << _9P2000E::Request::Session{{8, 7, 6, 5, 4, 3, 2, 1}};
	_messageWriter.build();

	getRequestOrFail<_9P2000E::Request::Session>()
			.then([](_9P2000E::Request::Session&& request) {
				ASSERT_EQ(8, request.key[0]);
				ASSERT_EQ(4, request.key[4]);
				ASSERT_EQ(1, request.key[7]);
			});
}


TEST_F(P9E_Messages, parseSessionRequest_NotEnoughData) {
	byte const sessionKey[5] = {8, 7, 6, 5, 4};
    auto keyData = wrapMemory(sessionKey);

    // Set declared message size to be more then negotiated message size
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(messageCode(_9P2000E::Response::Session{}), 1, keyData.size());
    _writer.write(keyData);

	UnversionedParser parser{kMaxMesssageSize};
	auto headerResult = parser.parseMessageHeader(_reader);
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
	ASSERT_EQ(messageCode(_9P2000E::Response::Session{}), header.type);

    // Make sure we can parse the message back.
	auto message = parser.parseMessage(header, _reader);
    ASSERT_TRUE(message.isError());
}


TEST_F(P9E_Messages, createSessionRespose) {
	_messageWriter << _9P2000E::Response::Session{};
	_messageWriter.build();

	getResponseOrFail<_9P2000E::Response::Session>();
}


TEST_F(P9E_Messages, parseSessionRespose) {
    // Set declared message size to be more then negotiated message size
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(messageCode(_9P2000E::Response::Session{}), 1, 0);
    _writer.flip();

	getResponseOrFail<_9P2000E::Response::Session>();
}



TEST_F(P9E_Messages, createShortReadRequest) {
	MessageWriter writer{_writer};

	byte buffer[15 + 2*3];
	ByteWriter pathWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{pathWriter};
	encoder << StringView{"some"}
			<< StringView{"wierd"}
			<< StringView{"place"};

	WalkPath path{3, wrapMemory(buffer)};
	writer << _9P2000E::Request::ShortRead{32, path};
	writer.build();

	getRequestOrFail<_9P2000E::Request::ShortRead>()
		.then([](_9P2000E::Request::ShortRead&& request) {
            ASSERT_EQ(32, request.fid);
			ASSERT_EQ(3, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}


TEST_F(P9E_Messages, createShortReadRespose) {
	char const messageData[] = "This was somewhat important data d^_-b";
    auto data = wrapMemory(messageData);
	MessageWriter writer{_writer, 1};
	writer << _9P2000E::Response::ShortRead{data};
	writer.build();

	getResponseOrFail<_9P2000E::Response::ShortRead>()
			.then([data](_9P2000E::Response::ShortRead&& response) {
                EXPECT_EQ(data, response.data);
            });
}


TEST_F(P9E_Messages, parseShortReadRespose) {
    auto const messageData = StringLiteral{"This is a very important data d-_^b"};
    auto const dataView = messageData.view();

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(messageCode(_9P2000E::Response::ShortRead{}),
									 1,
									 narrow_cast<size_type>(sizeof(size_type) + dataView.size()))
			<< dataView;
    _writer.flip();


	getResponseOrFail<_9P2000E::Response::ShortRead>()
			.then([messageData](_9P2000E::Response::ShortRead&& response) {
                EXPECT_EQ(messageData.size(), response.data.size());
                EXPECT_EQ(messageData.view(), response.data);
            });
}


TEST_F(P9E_Messages, createShortWriteRequest) {
	char const messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);


	byte buffer[15 + 2*3];
	ByteWriter pathWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{pathWriter};
	encoder << StringView{"some"}
			<< StringView{"wierd"}
			<< StringView{"place"};

	WalkPath path{3, wrapMemory(buffer)};

	MessageWriter writer{_writer};
	writer << _9P2000E::Request::ShortWrite{32, path, data};
	writer.build();

	getRequestOrFail<_9P2000E::Request::ShortWrite>()
		.then([data](_9P2000E::Request::ShortWrite&& request) {
            ASSERT_EQ(32, request.fid);
            ASSERT_EQ(data, request.data);
			ASSERT_EQ(3, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}


TEST_F(P9E_Messages, createShortWriteRespose) {
	MessageWriter writer{_writer, 1};
	writer << _9P2000E::Response::ShortWrite{100500};
	writer.build();

	getResponseOrFail<_9P2000E::Response::ShortWrite>()
			.then([](_9P2000E::Response::ShortWrite&& response) {
                EXPECT_EQ(100500, response.count);
            });
}


TEST_F(P9E_Messages, parseShortWriteRespose) {
	styxe::Encoder encoder {_writer};
	encoder << makeHeaderWithPayload(messageCode(_9P2000E::Response::ShortWrite{}), 1, sizeof(uint32))
			<< (static_cast<uint32>(81177));
    _writer.flip();

	getResponseOrFail<_9P2000E::Response::ShortWrite>()
			.then([](_9P2000E::Response::ShortWrite&& response) {
                EXPECT_EQ(81177, response.count);
            });
}
