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


using namespace Solace;
using namespace styxe;


auto const kTestedVestion = _9P2000L::kProtocolVersion;


namespace  {

struct P92000L_Responses : public TestHarnes {

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



struct P92000L_Requests : public TestHarnes {

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


TEST_F(P92000L_Requests, statFS) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::StatFS{727};

	getRequestOrFail<_9P2000L::Request::StatFS>()
			.then([](_9P2000L::Request::StatFS&& request) {
				ASSERT_EQ(727U, request.fid);
			});
}

TEST_F(P92000L_Requests, open) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::LOpen{3310, 1841};

	getRequestOrFail<_9P2000L::Request::LOpen>()
			.then([](_9P2000L::Request::LOpen&& request) {
				ASSERT_EQ(3310U, request.fid);
				ASSERT_EQ(1841U, request.flags);
			});
}


TEST_F(P92000L_Requests, create) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::LCreate{1734, "mcFance", 11, 8881919, 999888};

	getRequestOrFail<_9P2000L::Request::LCreate>()
			.then([](_9P2000L::Request::LCreate&& request) {
				ASSERT_EQ(1734U, request.fid);
				ASSERT_EQ("mcFance", request.name);
				ASSERT_EQ(11U, request.flags);
				ASSERT_EQ(8881919U, request.mode);
				ASSERT_EQ(999888U, request.gid);
			});
}


TEST_F(P92000L_Requests, symlink) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::Symlink{8193, "one-file", "other-name", 3319};

	getRequestOrFail<_9P2000L::Request::Symlink>()
			.then([](_9P2000L::Request::Symlink&& request) {
				ASSERT_EQ(8193U, request.fid);
				ASSERT_EQ("one-file", request.name);
				ASSERT_EQ("other-name", request.symtgt);
				ASSERT_EQ(3319U, request.gid);
			});
}


TEST_F(P92000L_Requests, mkNode) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::MkNode{6523, "one-file", 3319, 119, 8282, 9911};

	getRequestOrFail<_9P2000L::Request::MkNode>()
			.then([](_9P2000L::Request::MkNode&& request) {
				ASSERT_EQ(6523U, request.dfid);
				ASSERT_EQ("one-file", request.name);
				ASSERT_EQ(3319U, request.mode);
				ASSERT_EQ(119U, request.major);
				ASSERT_EQ(8282U, request.minor);
				ASSERT_EQ(9911U, request.gid);
			});
}


TEST_F(P92000L_Requests, rename) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::Rename{8193, 434, "one-file"};

	getRequestOrFail<_9P2000L::Request::Rename>()
			.then([](_9P2000L::Request::Rename&& request) {
				ASSERT_EQ(8193U, request.fid);
				ASSERT_EQ(434U, request.dfid);
				ASSERT_EQ("one-file", request.name);
			});
}



TEST_F(P92000L_Requests, readLink) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::ReadLink{8193};

	getRequestOrFail<_9P2000L::Request::ReadLink>()
			.then([](_9P2000L::Request::ReadLink&& request) {
				ASSERT_EQ(8193U, request.fid);
			});
}


TEST_F(P92000L_Requests, getAttr) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::GetAttr{8193, 71641};

	getRequestOrFail<_9P2000L::Request::GetAttr>()
			.then([](_9P2000L::Request::GetAttr&& request) {
				ASSERT_EQ(8193U, request.fid);
				ASSERT_EQ(71641U, request.request_mask);
			});
}


TEST_F(P92000L_Requests, setAttr) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::SetAttr{
			  5324,
			  8182773,
			  54643,
			  394732,
			  721632,
			  76,
			  8593993,
			  123,
			  936483264,
			  1232
		};

	getRequestOrFail<_9P2000L::Request::SetAttr>()
			.then([](_9P2000L::Request::SetAttr&& request) {
				ASSERT_EQ(5324U, request.fid);
				ASSERT_EQ(8182773U, request.valid);
				ASSERT_EQ(54643U, request.mode);
				ASSERT_EQ(394732U, request.uid);
				ASSERT_EQ(721632U, request.gid);
				ASSERT_EQ(76U, request.size);

				ASSERT_EQ(8593993U, request.atime_sec);
				ASSERT_EQ(123U, request.atime_nsec);
				ASSERT_EQ(936483264U, request.mtime_sec);
				ASSERT_EQ(1232U, request.mtime_nsec);
			});
}


TEST_F(P92000L_Requests, xAttrWalk) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::XAttrWalk{8193, 732, "one-file"};

	getRequestOrFail<_9P2000L::Request::XAttrWalk>()
			.then([](_9P2000L::Request::XAttrWalk&& request) {
				ASSERT_EQ(8193U, request.fid);
				ASSERT_EQ(732U, request.newfid);
				ASSERT_EQ("one-file", request.name);
			});
}


TEST_F(P92000L_Requests, xAttrCreate) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::XAttrCreate{8193, "one-file", 3319, 9172};

	getRequestOrFail<_9P2000L::Request::XAttrCreate>()
			.then([](_9P2000L::Request::XAttrCreate&& request) {
				ASSERT_EQ(8193U, request.fid);
				ASSERT_EQ("one-file", request.name);
				ASSERT_EQ(3319U, request.attr_size);
				ASSERT_EQ(9172U, request.flags);
			});
}


TEST_F(P92000L_Requests, readDir) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::ReadDir{8193, 71632, 2132};

	getRequestOrFail<_9P2000L::Request::ReadDir>()
			.then([](_9P2000L::Request::ReadDir&& request) {
				ASSERT_EQ(8193U, request.fid);
				ASSERT_EQ(71632U, request.offset);
				ASSERT_EQ(2132U, request.count);
			});
}


TEST_F(P92000L_Requests, fSync) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::FSync{8193};

	getRequestOrFail<_9P2000L::Request::FSync>()
			.then([](_9P2000L::Request::FSync&& request) {
				ASSERT_EQ(8193U, request.fid);
			});
}



TEST_F(P92000L_Requests, lock) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::Lock{8193,
			  123,
			  9818732,
			  87123,
			  123,
			  98372498,
			  "client_id"};

	getRequestOrFail<_9P2000L::Request::Lock>()
			.then([](_9P2000L::Request::Lock&& request) {
				ASSERT_EQ(8193U, request.fid);
				ASSERT_EQ(123U, request.type);
				ASSERT_EQ(9818732U, request.flags);
				ASSERT_EQ(87123U, request.start);
				ASSERT_EQ(123U, request.length);
				ASSERT_EQ(98372498U, request.proc_id);
				ASSERT_EQ("client_id", request.client_id);
			});
}


TEST_F(P92000L_Requests, getLock) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::GetLock{8193,
			  123,
			  87123,
			  123,
			  98372498,
			  "client_id"};

	getRequestOrFail<_9P2000L::Request::GetLock>()
			.then([](_9P2000L::Request::GetLock&& request) {
				ASSERT_EQ(8193U, request.fid);
				ASSERT_EQ(123U, request.type);
				ASSERT_EQ(87123U, request.start);
				ASSERT_EQ(123U, request.length);
				ASSERT_EQ(98372498U, request.proc_id);
				ASSERT_EQ("client_id", request.client_id);
			});
}



TEST_F(P92000L_Requests, link) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::Link{9818732, 87123, "named"};

	getRequestOrFail<_9P2000L::Request::Link>()
			.then([](_9P2000L::Request::Link&& request) {
				ASSERT_EQ(9818732U, request.dfid);
				ASSERT_EQ(87123U, request.fid);
				ASSERT_EQ("named", request.name);
			});
}


TEST_F(P92000L_Requests, mkDir) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::MkDir{9818732, "dirid", 87123, 99911};

	getRequestOrFail<_9P2000L::Request::MkDir>()
			.then([](_9P2000L::Request::MkDir&& request) {
				ASSERT_EQ(9818732U, request.dfid);
				ASSERT_EQ("dirid", request.name);
				ASSERT_EQ(87123U, request.mode);
				ASSERT_EQ(99911U, request.gid);
			});
}


TEST_F(P92000L_Requests, renameAt) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::RenameAt{9818732, "badname", 87123, "bettername"};

	getRequestOrFail<_9P2000L::Request::RenameAt>()
			.then([](_9P2000L::Request::RenameAt&& request) {
				ASSERT_EQ(9818732U, request.olddirfid);
				ASSERT_EQ("badname", request.oldname);
				ASSERT_EQ(87123U, request.newdirfid);
				ASSERT_EQ("bettername", request.newname);
			});
}


TEST_F(P92000L_Requests, unlinkAt) {
	RequestWriter writer{_writer};
	writer << _9P2000L::Request::UnlinkAt{9818732, "badname", 77187123};

	getRequestOrFail<_9P2000L::Request::UnlinkAt>()
			.then([](_9P2000L::Request::UnlinkAt&& request) {
				ASSERT_EQ(9818732U, request.dfid);
				ASSERT_EQ("badname", request.name);
				ASSERT_EQ(77187123U, request.flags);
			});
}

//----------------------------------------------------------------------------------------------------------------------
// Responses
//----------------------------------------------------------------------------------------------------------------------

TEST_F(P92000L_Responses, lError) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::LError{9912};

	getResponseOrFail<_9P2000L::Response::LError>()
			.then([] (_9P2000L::Response::LError const& response) {
				ASSERT_EQ(9912U, response.ecode);
			});
}


TEST_F(P92000L_Responses, StatFS) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::StatFS {
			  8216372,
			  6662,
			  28713,
			  21312,
			  213213,
			  12321,
			  4354,
			  79824397543957,
			  5431};

	getResponseOrFail<_9P2000L::Response::StatFS>()
			.then([] (_9P2000L::Response::StatFS&& response) {
				ASSERT_EQ(8216372U, response.type);
				ASSERT_EQ(6662U, response.bsize);
				ASSERT_EQ(28713U, response.blocks);
				ASSERT_EQ(21312U, response.bfree);
				ASSERT_EQ(213213U, response.bavail);
				ASSERT_EQ(12321U, response.files);
				ASSERT_EQ(4354U, response.ffree);
				ASSERT_EQ(79824397543957U, response.fsid);
				ASSERT_EQ(5431U, response.namelen);
			});
}


TEST_F(P92000L_Responses, open) {
	auto qid = randomQid();
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::LOpen{{qid, 8732874}};

	getResponseOrFail<_9P2000L::Response::LOpen>()
			.then([qid] (_9P2000L::Response::LOpen const& response) {
				ASSERT_EQ(qid, response.qid);
				ASSERT_EQ(8732874U, response.iounit);
			});
}

TEST_F(P92000L_Responses, create) {
	auto qid = randomQid();
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::LCreate{{qid, 3123}};

	getResponseOrFail<_9P2000L::Response::LCreate>()
			.then([qid] (_9P2000L::Response::LCreate const& response) {
				ASSERT_EQ(qid, response.qid);
				ASSERT_EQ(3123U, response.iounit);
			});
}


TEST_F(P92000L_Responses, symlink) {
	auto qid = randomQid();
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::Symlink{qid};

	getResponseOrFail<_9P2000L::Response::Symlink>()
			.then([qid] (_9P2000L::Response::Symlink const& response) {
				ASSERT_EQ(qid, response.qid);
			});
}


TEST_F(P92000L_Responses, mkNode) {
	auto qid = randomQid();
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::MkNode{qid};

	getResponseOrFail<_9P2000L::Response::MkNode>()
			.then([qid] (_9P2000L::Response::MkNode const& response) {
				ASSERT_EQ(qid, response.qid);
			});
}


TEST_F(P92000L_Responses, rename) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::Rename{};

	ASSERT_TRUE(getResponseOrFail<_9P2000L::Response::Rename>());
}


TEST_F(P92000L_Responses, readLink) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::ReadLink{"SomeName"};

	getResponseOrFail<_9P2000L::Response::ReadLink>()
			.then([] (_9P2000L::Response::ReadLink const& response) {
				ASSERT_EQ("SomeName", response.target);
			});
}


TEST_F(P92000L_Responses, getAttr) {
	auto qid = randomQid();
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::GetAttr{
			  qid,
			  123,
			  654,
			  234,
			  435,
			  12734,

			  234141,
			  312,
			  435,
			  6345,

			  12341,
			  452,

			  4,
			  145,
			  23452435,
			  5132,

			  1324,
			  134,
			  1234,
			  7645};

	getResponseOrFail<_9P2000L::Response::GetAttr>()
			.then([qid] (_9P2000L::Response::GetAttr const& response) {
				ASSERT_EQ(qid, response.qid);

				ASSERT_EQ(123U, response.valid);
				ASSERT_EQ(654U, response.mode);
				ASSERT_EQ(234U, response.uid);
				ASSERT_EQ(435U, response.gid);
				ASSERT_EQ(12734U, response.size);

				ASSERT_EQ(234141U, response.atime_sec);
				ASSERT_EQ(312U, response.atime_nsec);
				ASSERT_EQ(435U, response.mtime_sec);
				ASSERT_EQ(6345U, response.mtime_nsec);

				ASSERT_EQ(12341U, response.ctime_sec);
				ASSERT_EQ(452U, response.ctime_nsec);

				ASSERT_EQ(4U, response.nlink);
				ASSERT_EQ(145U, response.rdev);
				ASSERT_EQ(23452435U, response.blksize);
				ASSERT_EQ(5132U, response.blocks);

				ASSERT_EQ(1324U, response.btime_sec);
				ASSERT_EQ(134U, response.btime_nsec);
				ASSERT_EQ(1234U, response.gen);
				ASSERT_EQ(7645U, response.data_version);
			});
}


TEST_F(P92000L_Responses, setAttr) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::SetAttr{};

	ASSERT_TRUE(getResponseOrFail<_9P2000L::Response::SetAttr>());
}



TEST_F(P92000L_Responses, xAttrWalk) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::XAttrWalk{98912};

	getResponseOrFail<_9P2000L::Response::XAttrWalk>()
			.then([] (_9P2000L::Response::XAttrWalk const& response) {
				ASSERT_EQ(98912U, response.size);
			});
}


TEST_F(P92000L_Responses, xAttrCreate) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::XAttrCreate{};

	ASSERT_TRUE(getResponseOrFail<_9P2000L::Response::XAttrCreate>());
}


TEST_F(P92000L_Responses, readDir) {
	char buffer[127];
	MutableMemoryView data = wrapMemory(buffer);
	ByteWriter dirStream{data};
	_9P2000L::DirEntry entry{
		randomQid(),
				0,
				31,
				StringView{"Awesome file"}
	};

	styxe::Encoder encoder{dirStream};
	encoder << entry;

	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::ReadDir{data};

	getResponseOrFail<_9P2000L::Response::ReadDir>()
			.then([data] (_9P2000L::Response::ReadDir const& response) {
				ASSERT_EQ(data, response.data);
			});
}


TEST_F(P92000L_Responses, fSync) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::FSync{};

	ASSERT_TRUE(getResponseOrFail<_9P2000L::Response::FSync>());
}


TEST_F(P92000L_Responses, lock) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::Lock{162};

	getResponseOrFail<_9P2000L::Response::Lock>()
			.then([] (_9P2000L::Response::Lock const& response) {
				ASSERT_EQ(162, response.status);
			});
}


TEST_F(P92000L_Responses, getLock) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::GetLock{87, 129836, 2132, 1231, "Boo!"};

	getResponseOrFail<_9P2000L::Response::GetLock>()
			.then([] (_9P2000L::Response::GetLock const& response) {
				ASSERT_EQ(87, response.type);
				ASSERT_EQ(129836U, response.start);
				ASSERT_EQ(2132U, response.length);
				ASSERT_EQ(1231U, response.proc_id);
				ASSERT_EQ("Boo!", response.client_id);
			});
}


TEST_F(P92000L_Responses, link) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::Link{};

	ASSERT_TRUE(getResponseOrFail<_9P2000L::Response::Link>());
}


TEST_F(P92000L_Responses, MkDir) {
	auto qid = randomQid();
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::MkDir{qid};

	getResponseOrFail<_9P2000L::Response::MkDir>()
			.then([qid] (_9P2000L::Response::MkDir const& response) {
				ASSERT_EQ(qid, response.qid);
			});
}


TEST_F(P92000L_Responses, renameAt) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::RenameAt{};

	ASSERT_TRUE(getResponseOrFail<_9P2000L::Response::RenameAt>());
}


TEST_F(P92000L_Responses, unlinkAt) {
	ResponseWriter writer{_writer, 3};
	writer << _9P2000L::Response::UnlinkAt{};

	ASSERT_TRUE(getResponseOrFail<_9P2000L::Response::UnlinkAt>());
}


//----------------------------------------------------------------------------------------------------------------------
// Responses manual check
//----------------------------------------------------------------------------------------------------------------------
TEST_F(P92000L_Responses, parseLError) {
	styxe::Encoder encoder{_writer};
	encoder << makeHeaderWithPayload(asByte(_9P2000L::MessageType::Rlerror), 1, 4)
			<< uint32{9913};

	getResponseOrFail<_9P2000L::Response::LError>()
			.then([](_9P2000L::Response::LError const& response) {
				EXPECT_EQ(9913U, response.ecode);
			});
}
