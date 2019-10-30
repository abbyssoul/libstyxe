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

#include <solace/output_utils.hpp>

#include <gtest/gtest.h>


using namespace Solace;
using namespace styxe;

class P9E_Responses
        : public ::testing::Test {
protected:

    // cppcheck-suppress unusedFunction
	void SetUp() override {
		_memBuf.view().fill(0xFE);

		_writer.rewind();
		_reader.rewind();
	}

    template<typename ResponseType>
    Result<ResponseType, Error>
	getResponseOrFail() {
        _reader.limit(_writer.limit());

		auto _maybeParser = createParser(kMaxMessageSize, _9P2000E::kProtocolVersion);
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

	MemoryManager   _memManager {kMaxMessageSize};
	MemoryResource  _memBuf{_memManager.allocate(kMaxMessageSize).unwrap()};
    ByteWriter      _writer{_memBuf};
    ByteReader      _reader{_memBuf};

	ResponseWriter	_responseWriter{_writer};
};



class P9E_Requests
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

		auto _maybeParser = createParser(kMaxMessageSize, _9P2000E::kProtocolVersion);
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

protected:

	MemoryManager   _memManager {kMaxMessageSize};
	MemoryResource  _memBuf{_memManager.allocate(kMaxMessageSize).unwrap()};
	ByteWriter      _writer{_memBuf};
	ByteReader      _reader{_memBuf};

	RequestWriter	_requestWriter{_writer};
};


TEST_F(P9E_Requests, createSessionRequest) {
	_requestWriter << _9P2000E::Request::Session{{8, 7, 6, 5, 4, 3, 2, 1}};
	_requestWriter.build();

	getRequestOrFail<_9P2000E::Request::Session>()
			.then([](_9P2000E::Request::Session&& request) {
				ASSERT_EQ(8, request.key[0]);
				ASSERT_EQ(4, request.key[4]);
				ASSERT_EQ(1, request.key[7]);
			});
}


TEST_F(P9E_Requests, parseSessionRequest_NotEnoughData) {
	byte const sessionKey[5] = {8, 7, 6, 5, 4};
    auto keyData = wrapMemory(sessionKey);

    // Set declared message size to be more then negotiated message size
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(messageCode(_9P2000E::Response::Session{}), 1, keyData.size());
    _writer.write(keyData);

	UnversionedParser parser{kMaxMessageSize};
	auto headerResult = parser.parseMessageHeader(_reader);
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
	ASSERT_EQ(messageCode(_9P2000E::Response::Session{}), header.type);

    // Make sure we can parse the message back.
	auto message = parser.parseMessage(header, _reader);
    ASSERT_TRUE(message.isError());
}


TEST_F(P9E_Responses, createSessionResponse) {
	_responseWriter << _9P2000E::Response::Session{};
	_responseWriter.build();

	getResponseOrFail<_9P2000E::Response::Session>();
}


TEST_F(P9E_Responses, parseSessionResponse) {
    // Set declared message size to be more then negotiated message size
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(messageCode(_9P2000E::Response::Session{}), 1, 0);
    _writer.flip();

	getResponseOrFail<_9P2000E::Response::Session>();
}



TEST_F(P9E_Requests, createShortReadRequest) {
	byte buffer[15 + 2*3];
	ByteWriter pathWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{pathWriter};
	encoder << StringView{"some"}
			<< StringView{"wierd"}
			<< StringView{"place"};

	WalkPath path{3, wrapMemory(buffer)};
	_requestWriter << _9P2000E::Request::ShortRead{32, path};
	_requestWriter.build();

	getRequestOrFail<_9P2000E::Request::ShortRead>()
		.then([](_9P2000E::Request::ShortRead&& request) {
            ASSERT_EQ(32, request.fid);
			ASSERT_EQ(3, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}


TEST_F(P9E_Responses, createShortReadResponse) {
	char const messageData[] = "This was somewhat important data d^_-b";
    auto data = wrapMemory(messageData);
	_responseWriter << _9P2000E::Response::ShortRead{data};
	_responseWriter.build();

	getResponseOrFail<_9P2000E::Response::ShortRead>()
			.then([data](_9P2000E::Response::ShortRead&& response) {
                EXPECT_EQ(data, response.data);
            });
}


TEST_F(P9E_Responses, parseShortReadResponse) {
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


TEST_F(P9E_Requests, createShortWriteRequest) {
	char const messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);


	byte buffer[15 + 2*3];
	ByteWriter pathWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{pathWriter};
	encoder << StringView{"some"}
			<< StringView{"wierd"}
			<< StringView{"place"};

	_requestWriter << _9P2000E::Request::ShortWrite{32, WalkPath{3, wrapMemory(buffer)}, data};
	_requestWriter.build();

	getRequestOrFail<_9P2000E::Request::ShortWrite>()
		.then([data](_9P2000E::Request::ShortWrite&& request) {
            ASSERT_EQ(32, request.fid);
            ASSERT_EQ(data, request.data);
			ASSERT_EQ(3, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}


TEST_F(P9E_Responses, createShortWriteResponse) {
	_responseWriter << _9P2000E::Response::ShortWrite{100500};
	_responseWriter.build();

	getResponseOrFail<_9P2000E::Response::ShortWrite>()
			.then([](_9P2000E::Response::ShortWrite&& response) {
                EXPECT_EQ(100500, response.count);
            });
}


TEST_F(P9E_Responses, parseShortWriteResponse) {
	styxe::Encoder encoder {_writer};
	encoder << makeHeaderWithPayload(messageCode(_9P2000E::Response::ShortWrite{}), 1, sizeof(uint32))
			<< (static_cast<uint32>(81177));
    _writer.flip();

	getResponseOrFail<_9P2000E::Response::ShortWrite>()
			.then([](_9P2000E::Response::ShortWrite&& response) {
                EXPECT_EQ(81177, response.count);
            });
}
