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

#include "testHarnes.hpp"


using namespace Solace;
using namespace styxe;


auto const kTestedVestion = _9P2000E::kProtocolVersion;

namespace  {

struct P92000e_Responses : public TestHarnes {

    template<typename ResponseType>
	styxe::Result<ResponseType>
	getResponseOrFail() {
		ByteReader reader{_writer.viewWritten()};

		auto maybeParser = createResponseParser(kTestedVestion, kMaxMessageSize);
		if (!maybeParser) {
			logFailure(maybeParser.getError());
			return maybeParser.moveError();
		}
		auto& parser = maybeParser.unwrap();

		auto constexpr expectType = messageCodeOf<ResponseType>();
		return parseMessageHeader(reader)
				.then([](MessageHeader&& header) {
                    return (header.type != expectType)
					? styxe::Result<MessageHeader>{types::errTag, getCannedError(CannedError::UnsupportedMessageType)}
					: styxe::Result<MessageHeader>{types::okTag, std::move(header)};
                })
				.then([&parser, &reader](MessageHeader&& header) {
					return parser.parseResponse(header, reader);
                })
				.then([](ResponseMessage&& msg) -> styxe::Result<ResponseType> {
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

protected:

	ResponseWriter	_responseWriter{_writer};
};



struct P92000e_Requests : public TestHarnes {

	template<typename RequestType>
	styxe::Result<RequestType>
	getRequestOrFail() {
		ByteReader reader{_writer.viewWritten()};

		auto maybeParser = createRequestParser(kTestedVestion, kMaxMessageSize);
		if (!maybeParser) {
			logFailure(maybeParser.getError());
			return maybeParser.moveError();
		}
		auto& parser = maybeParser.unwrap();

		auto constexpr expectType = messageCodeOf<RequestType>();
		return parseMessageHeader(reader)
				.then([](MessageHeader&& header) {
					return (header.type != expectType)
					? styxe::Result<MessageHeader>{getCannedError(CannedError::UnsupportedMessageType)}
					: styxe::Result<MessageHeader>{types::okTag, std::move(header)};
				})
				.then([&parser, &reader](MessageHeader&& header) {
					return parser.parseRequest(header, reader);
				})
				.then([](RequestMessage&& msg) -> styxe::Result<RequestType> {
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

protected:

	RequestWriter	_requestWriter{_writer};
};

}  // namespace


TEST_F(P92000e_Requests, createSessionRequest) {
	_requestWriter << _9P2000E::Request::Session{{8, 7, 6, 5, 4, 3, 2, 1}};

	getRequestOrFail<_9P2000E::Request::Session>()
			.then([](_9P2000E::Request::Session&& request) {
				ASSERT_EQ(8, request.key[0]);
				ASSERT_EQ(4, request.key[4]);
				ASSERT_EQ(1, request.key[7]);
			});
}


TEST_F(P92000e_Requests, parseSessionRequest_NotEnoughData) {
	byte const sessionKey[5] = {8, 7, 6, 5, 4};
    auto keyData = wrapMemory(sessionKey);

    // Set declared message size to be more then negotiated message size
	_requestWriter.encoder() << makeHeaderWithPayload(messageCodeOf<_9P2000E::Response::Session>(), 1, keyData.size());
    _writer.write(keyData);

	ByteReader reader{_writer.viewWritten()};
	auto headerResult = parseMessageHeader(reader);
    ASSERT_TRUE(headerResult.isOk());

    auto header = headerResult.unwrap();
	ASSERT_EQ(messageCodeOf<_9P2000E::Response::Session>(), header.type);

    // Make sure we can parse the message back.
	auto message = parseVersionRequest(header, reader, kMaxMessageSize);
    ASSERT_TRUE(message.isError());
}


TEST_F(P92000e_Responses, createSessionResponse) {
	_responseWriter << _9P2000E::Response::Session{};

	getResponseOrFail<_9P2000E::Response::Session>();
}


TEST_F(P92000e_Responses, parseSessionResponse) {
    // Set declared message size to be more then negotiated message size
	_responseWriter.encoder() << makeHeaderWithPayload(messageCodeOf<_9P2000E::Response::Session>(), 1, 0);

	getResponseOrFail<_9P2000E::Response::Session>();
}



TEST_F(P92000e_Requests, createShortReadRequest) {
	byte buffer[15 + 2*3];
	ByteWriter pathWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{pathWriter};
	encoder << StringView{"some"}
			<< StringView{"wierd"}
			<< StringView{"place"};

	_requestWriter << _9P2000E::Request::ShortRead{32, WalkPath{3, wrapMemory(buffer)}};

	getRequestOrFail<_9P2000E::Request::ShortRead>()
		.then([](_9P2000E::Request::ShortRead&& request) {
			ASSERT_EQ(32U, request.fid);
			ASSERT_EQ(3U, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}



TEST_F(P92000e_Requests, createPartialShortReadRequest) {
	_requestWriter << _9P2000E::Request::Partial::ShortRead{32}
				   << StringView{"some"}
				   << StringView{"wierd"}
				   << StringView{"place"};

	getRequestOrFail<_9P2000E::Request::ShortRead>()
		.then([](_9P2000E::Request::ShortRead&& request) {
			ASSERT_EQ(32U, request.fid);
			ASSERT_EQ(3U, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}


TEST_F(P92000e_Responses, createShortReadResponse) {
	char const messageData[] = "This was somewhat important data d^_-b";
    auto data = wrapMemory(messageData);
	_responseWriter << _9P2000E::Response::ShortRead{data};

	getResponseOrFail<_9P2000E::Response::ShortRead>()
			.then([data](_9P2000E::Response::ShortRead&& response) {
                EXPECT_EQ(data, response.data);
            });
}


TEST_F(P92000e_Responses, parseShortReadResponse) {
    auto const messageData = StringLiteral{"This is a very important data d-_^b"};
    auto const dataView = messageData.view();


	_responseWriter.encoder() << makeHeaderWithPayload(messageCodeOf<_9P2000E::Response::ShortRead>(),
													   1,
													   narrow_cast<size_type>(sizeof(size_type) + dataView.size()))
							  << dataView;

	getResponseOrFail<_9P2000E::Response::ShortRead>()
			.then([messageData](_9P2000E::Response::ShortRead&& response) {
                EXPECT_EQ(messageData.size(), response.data.size());
                EXPECT_EQ(messageData.view(), response.data);
            });
}


TEST_F(P92000e_Requests, createShortWriteRequest) {
	char const messageData[] = "This is a very important data d-_^b";
    auto data = wrapMemory(messageData);

	byte buffer[15 + 2*3];
	ByteWriter pathWriter{wrapMemory(buffer)};
	styxe::Encoder encoder{pathWriter};
	encoder << StringView{"some"}
			<< StringView{"wierd"}
			<< StringView{"place"};

	_requestWriter << _9P2000E::Request::ShortWrite{32, WalkPath{3, wrapMemory(buffer)}, data};
	getRequestOrFail<_9P2000E::Request::ShortWrite>()
		.then([data](_9P2000E::Request::ShortWrite&& request) {
			ASSERT_EQ(32U, request.fid);
            ASSERT_EQ(data, request.data);
			ASSERT_EQ(3U, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}


TEST_F(P92000e_Requests, createPartialShortWriteRequest) {
	char const messageData[] = "This is a very important data d-_^b";
	auto data = wrapMemory(messageData);

	_requestWriter << _9P2000E::Request::Partial::ShortWrite{32}
				   << StringView{"some"}
				   << StringView{"wierd"}
				   << StringView{"place"}
				   << data;


	getRequestOrFail<_9P2000E::Request::ShortWrite>()
		.then([data](_9P2000E::Request::ShortWrite&& request) {
			ASSERT_EQ(32U, request.fid);
			ASSERT_EQ(data, request.data);
			ASSERT_EQ(3U, request.path.size());
			ASSERT_EQ("some", *request.path.begin());
		});
}



TEST_F(P92000e_Responses, createShortWriteResponse) {
	_responseWriter << _9P2000E::Response::ShortWrite{100500};

	getResponseOrFail<_9P2000E::Response::ShortWrite>()
			.then([](_9P2000E::Response::ShortWrite&& response) {
				EXPECT_EQ(100500U, response.count);
            });
}


TEST_F(P92000e_Responses, parseShortWriteResponse) {
	_responseWriter.encoder() << makeHeaderWithPayload(messageCodeOf<_9P2000E::Response::ShortWrite>(), 1, sizeof(uint32))
							  << (static_cast<uint32>(81177));

	getResponseOrFail<_9P2000E::Response::ShortWrite>()
			.then([](_9P2000E::Response::ShortWrite&& response) {
				EXPECT_EQ(81177U, response.count);
            });
}
