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
 * @file: test/test_9P2000u.cpp
 * Specific test 9P2000.e
 *******************************************************************************/
#include "styxe/messageWriter.hpp"
#include "styxe/messageParser.hpp"

#include "testHarnes.hpp"
#include <random>

using namespace Solace;
using namespace styxe;


auto const kTestedVestion = _9P2000U::kProtocolVersion;



_9P2000U::StatEx randomStat() {
	std::random_device rd;
	std::default_random_engine randomGen{rd()};


	_9P2000U::StatEx stat;
	stat.atime = randomGen();
	stat.dev = randomGen();
	stat.gid = "Other user";
	stat.length = randomGen();
	stat.mode = 111;
	stat.mtime = randomGen();
	stat.name = "la-la McFile";
	stat.qid.path = 61;
	stat.qid.type = 15;
	stat.qid.version = 404;
	stat.type = 1;
	stat.uid = "Userface McUse";
	stat.n_uid = randomGen();
	stat.n_gid = randomGen();
	stat.n_muid = randomGen();

	stat.size = DirListingWriter::sizeStat(stat);

	return stat;
}



struct P92000u_Responses : public TestHarnes {

	template<typename ResponseType>
	Result<ResponseType, Error>
	getResponseOrFail() {
		ByteReader reader{_writer.viewWritten()};

		auto maybeParser = createResponseParser(kTestedVestion, kMaxMessageSize);
		if (!maybeParser) {
			logFailure(maybeParser.getError());
			return maybeParser.moveError();
		}
		auto& parser = maybeParser.unwrap();

		auto constexpr expectType = messageCodeOf<ResponseType>();
		auto headerParser = UnversionedParser{kMaxMessageSize};
		return headerParser.parseMessageHeader(reader)
				.then([](MessageHeader&& header) {
					return (header.type != expectType)
					? Result<MessageHeader, Error>{types::errTag, getCannedError(CannedError::UnsupportedMessageType)}
					: Result<MessageHeader, Error>{types::okTag, std::move(header)};
				})
				.then([&parser, &reader](MessageHeader&& header) {
					return parser.parseResponse(header, reader);
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

protected:
	ResponseWriter	_responseWriter{_writer};
};



struct P92000u_Requests : public TestHarnes {

	template<typename RequestType>
	Result<RequestType, Error>
	getRequestOrFail() {
		ByteReader reader{_writer.viewWritten()};

		auto maybeParser = createRequestParser(kTestedVestion, kMaxMessageSize);
		if (!maybeParser) {
			logFailure(maybeParser.getError());
			return maybeParser.moveError();
		}
		auto& parser = maybeParser.unwrap();

		auto constexpr expectType = messageCodeOf<RequestType>();
		auto headerParser = UnversionedParser{kMaxMessageSize};
		return headerParser.parseMessageHeader(reader)
				.then([](MessageHeader&& header) {
					return (header.type != expectType)
					? Result<MessageHeader, Error>{getCannedError(CannedError::UnsupportedMessageType)}
					: Result<MessageHeader, Error>{types::okTag, std::move(header)};
				})
				.then([&parser, &reader](MessageHeader&& header) {
					return parser.parseRequest(header, reader);
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

protected:

	RequestWriter	_requestWriter{_writer};
};


TEST_F(P92000u_Requests, createSessionAuth) {
	RequestWriter writer{_writer};
	writer << _9P2000U::Request::Auth{312, "User mcUsers", "Somewhere near", 7762};

	getRequestOrFail<_9P2000U::Request::Auth>()
			.then([](_9P2000U::Request::Auth&& request) {
				ASSERT_EQ(312, request.afid);
				ASSERT_EQ("User mcUsers", request.uname);
				ASSERT_EQ("Somewhere near", request.aname);
				ASSERT_EQ(7762, request.n_uname);
			});
}

TEST_F(P92000u_Requests, createAttachRequest) {
	RequestWriter writer{_writer};
	writer << _9P2000U::Request::Attach{3310, 1841, "McFace", "close to u", 6277};

	getRequestOrFail<_9P2000U::Request::Attach>()
			.then([](_9P2000U::Request::Attach&& request) {
				ASSERT_EQ(3310, request.fid);
				ASSERT_EQ(1841, request.afid);
				ASSERT_EQ("McFace", request.uname);
				ASSERT_EQ("close to u", request.aname);
				ASSERT_EQ(6277, request.n_uname);
			});
}


TEST_F(P92000u_Requests, createCreateRequest) {
	RequestWriter writer{_writer};
	writer << _9P2000U::Request::Create{1734, "mcFance", 11, OpenMode::EXEC, "Extra ext"};

	getRequestOrFail<_9P2000U::Request::Create>()
			.then([](_9P2000U::Request::Create&& request) {
				ASSERT_EQ(1734, request.fid);
				ASSERT_EQ("mcFance", request.name);
				ASSERT_EQ(11, request.perm);
				ASSERT_EQ(OpenMode::EXEC, request.mode);
				ASSERT_EQ("Extra ext", request.extension);
			});
}


TEST_F(P92000u_Requests, createWStatRequest) {
	_9P2000U::StatEx stat = randomStat();

	RequestWriter writer{_writer};
	writer << _9P2000U::Request::WStat{8193, stat};

	getRequestOrFail<_9P2000U::Request::WStat>()
			.then([stat](_9P2000U::Request::WStat&& request) {
				ASSERT_EQ(8193, request.fid);
				ASSERT_EQ(stat, request.stat);
			});
}


TEST_F(P92000u_Responses, createErrorResponse) {
	auto const testError = StringLiteral{"Something went right :)"};
	ResponseWriter writer{_writer, 3};
	writer << _9P2000U::Response::Error{testError, 9912};

	getResponseOrFail<_9P2000U::Response::Error>()
			.then([testError](_9P2000U::Response::Error&& response) {
				ASSERT_EQ(testError, response.ename);
				ASSERT_EQ(9912, response.errcode);
			});
}

TEST_F(P92000u_Responses, parseErrorResponse) {
	auto const expectedErrorMessage = StringLiteral{"All good!"};

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RError), 1, styxe::protocolSize(expectedErrorMessage) + 4)
			<< static_cast<StringView>(expectedErrorMessage)
			<< uint32{9913};

	getResponseOrFail<_9P2000U::Response::Error>()
			.then([expectedErrorMessage](_9P2000U::Response::Error&& response) {
				EXPECT_EQ(expectedErrorMessage, response.ename);
				EXPECT_EQ(9913, response.errcode);
			});
}


TEST_F(P92000u_Responses, createStatResponse) {
	_9P2000U::StatEx stat = randomStat();

	ResponseWriter writer{_writer, 1};
	writer << _9P2000U::Response::Stat{stat.size, stat};

	getResponseOrFail<_9P2000U::Response::Stat>()
			.then([stat](_9P2000U::Response::Stat&& response) {
				ASSERT_EQ(stat, response.data);
			});
}

TEST_F(P92000u_Responses, parseStatResponse) {
	_9P2000U::Response::Stat statResponse;
	statResponse.data = randomStat();
	statResponse.dummySize = protocolSize(statResponse.data);

	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(MessageType::RStat), 1,
									 sizeof(statResponse.dummySize) + protocolSize(statResponse.data))
			<< statResponse.dummySize
			<< statResponse.data;

	getResponseOrFail<_9P2000U::Response::Stat>()
			.then([statResponse](_9P2000U::Response::Stat&& response) {
				ASSERT_EQ(statResponse.dummySize, response.dummySize);
				ASSERT_EQ(statResponse.data, response.data);
			});
}
