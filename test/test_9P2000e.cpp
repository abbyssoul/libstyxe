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
#include "styxe/9p2000.hpp"  // Class being tested
#include "styxe/encoder.hpp"

#include <solace/exception.hpp>
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
    getRequestOrFail(MessageType expectType) {
        _reader.limit(_writer.limit());

        return proc.parseMessageHeader(_reader)
                .then([expectType](MessageHeader&& header) {
                    return (header.type != expectType)
					? Result<MessageHeader, Error>{getCannedError(CannedError::UnsupportedMessageType)}
					: Result<MessageHeader, Error>{types::OkTag{}, std::move(header)};
                })
                .then([this](MessageHeader&& header) {
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
						FAIL() << e;
                    } ();

                    return e;
                });
    }


    template<typename ResponseType>
    Result<ResponseType, Error>
    getResponseOrFail(MessageType expectType) {
        _reader.limit(_writer.limit());

        return proc.parseMessageHeader(_reader)
                .then([expectType](MessageHeader&& header) {
                    return (header.type != expectType)
				? Result<MessageHeader, Error>{getCannedError(CannedError::UnsupportedMessageType)}
				: Result<MessageHeader, Error>{types::OkTag{}, std::move(header)};
                })
                .then([this](MessageHeader&& header) {
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
                    [&e]() {
                        FAIL() << e.toString();
                    } ();

                    return e;
                });
    }

protected:

    Parser          proc;
    MemoryManager   _memManager {kMaxMesssageSize};
    MemoryResource  _memBuf{_memManager.allocate(kMaxMesssageSize)};
    ByteWriter      _writer{_memBuf};
    ByteReader      _reader{_memBuf};
};


TEST_F(P9E_Messages, createSessionRequest) {
	byte sessionKey[8] = {8, 7, 6, 5, 4, 3, 2, 1};
	auto const data = arrayView(sessionKey);

    RequestBuilder{_writer}
            .session(data)
            .build();

    getRequestOrFail<Request_9P2000E::Session>(MessageType::TSession)
            .then([data](Request_9P2000E::Session&& request) {
                ASSERT_EQ(data, wrapMemory(request.key));
            });
}

TEST_F(P9E_Messages, createSessionRequest_NotEnoughData) {
	byte sessionKey[5] = {8, 7, 6, 5, 4};

    ASSERT_THROW(RequestBuilder{_writer}
				 .session(arrayView(sessionKey)),
                 Solace::Exception);
}


TEST_F(P9E_Messages, parseSessionRequest_NotEnoughData) {
	byte const sessionKey[5] = {8, 7, 6, 5, 4};
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

    getResponseOrFail<Response_9P2000E::Session>(MessageType::RSession);
}

TEST_F(P9E_Messages, parseSessionRespose) {
    // Set declared message size to be more then negotiated message size
    styxe::Encoder{_writer}
            .header(MessageType::RSession, 1, 0);
    _writer.flip();

    getResponseOrFail<Response_9P2000E::Session>(MessageType::RSession);
}



TEST_F(P9E_Messages, createShortReadRequest) {
    RequestBuilder{_writer}
			.shortRead(32)
			.path("some")
			.path("wierd")
			.path("place")
            .build();

    getRequestOrFail<Request_9P2000E::SRead>(MessageType::TSRead)
		.then([](Request_9P2000E::SRead&& request) {
            ASSERT_EQ(32, request.fid);
			ASSERT_EQ(3, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}


TEST_F(P9E_Messages, createShortReadRespose) {
	char const messageData[] = "This was somewhat important data d^_-b";
    auto data = wrapMemory(messageData);
    ResponseBuilder(_writer, 1)
            .shortRead(data)
            .build();

    getResponseOrFail<Response::Read>(MessageType::RSRead)
            .then([data](Response::Read&& response) {
                EXPECT_EQ(data, response.data);
            });
}


TEST_F(P9E_Messages, parseShortReadRespose) {
    auto const messageData = StringLiteral{"This is a very important data d-_^b"};
    auto const dataView = messageData.view();

    styxe::Encoder{_writer}
            .header(MessageType::RSRead, 1, sizeof(size_type) + dataView.size())
            .encode(dataView);
    _writer.flip();


    getResponseOrFail<Response::Read>(MessageType::RSRead)
            .then([messageData](Response::Read&& response) {
                EXPECT_EQ(messageData.size(), response.data.size());
                EXPECT_EQ(messageData.view(), response.data);
            });
}


TEST_F(P9E_Messages, createShortWriteRequest) {
	char const messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

    RequestBuilder{_writer}
			.shortWrite(32)
			.path("some")
			.path("wierd")
			.path("place")
			.data(data)
			.build();

    getRequestOrFail<Request_9P2000E::SWrite>(MessageType::TSWrite)
		.then([data](Request_9P2000E::SWrite&& request) {
            ASSERT_EQ(32, request.fid);
            ASSERT_EQ(data, request.data);
			ASSERT_EQ(3, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}


TEST_F(P9E_Messages, createShortWriteRespose) {
    ResponseBuilder(_writer, 1)
            .shortWrite(100500)
            .build();

    getResponseOrFail<Response::Write>(MessageType::RSWrite)
            .then([](Response::Write&& response) {
                EXPECT_EQ(100500, response.count);
            });
}


TEST_F(P9E_Messages, parseShortWriteRespose) {
    styxe::Encoder{_writer}
            .header(MessageType::RSWrite, 1, sizeof(uint32))
            .encode(static_cast<uint32>(81177));
    _writer.flip();

    getResponseOrFail<Response::Write>(MessageType::RSWrite)
            .then([](Response::Write&& response) {
                EXPECT_EQ(81177, response.count);
            });
}
