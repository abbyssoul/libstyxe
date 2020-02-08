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
 * @file: test/test_P9DirListingWriter.cpp
 *
 *******************************************************************************/
#include "styxe/messageWriter.hpp"  // Class being tested
#include "styxe/messageParser.hpp"
#include "styxe/9p2000.hpp"

#include <solace/memoryManager.hpp>
#include <gtest/gtest.h>


using namespace Solace;
using namespace styxe;


class P9DirListingWriter : public ::testing::Test {
public:

	P9DirListingWriter()
		: _memManager{kMaxMessageSize}
    {}


protected:

    void SetUp() override {
		_buffer = _memManager.allocate(kMaxMessageSize).unwrap();
        _buffer.viewRemaining().fill(0xFE);
    }

    MemoryManager   _memManager;
    ByteWriter     _buffer;
};


TEST_F(P9DirListingWriter, directoryReadResponse) {
    Stat testStats[] = {
        {
            0,
            1,
            2,
            {2, 0, 64},
            01000644,
            0,
            0,
            4096,
            StringLiteral{"Root"},
            StringLiteral{"User"},
            StringLiteral{"Glanda"},
            StringLiteral{"User"}
        }
    };

	// Fix directory stat structs sizes
    for (auto& stat : testStats) {
        stat.size = DirListingWriter::sizeStat(stat);
    }

	auto responseWriter = ResponseWriter{_buffer, 1};
	auto dirWriter = DirListingWriter{responseWriter, 4096};
	for (auto const& stat : testStats) {
		if (!dirWriter.encode(stat)) {
            break;
		}
    }
	ASSERT_EQ(dirWriter.bytesEncoded(), protocolSize(testStats[0]));

	// Check response
	auto maybeParser = createResponseParser(kProtocolVersion, 128);
	ASSERT_TRUE(maybeParser.isOk());

	auto& parser = *maybeParser;

	ByteReader reader{_buffer.viewWritten()};
	auto headerParser = UnversionedParser{kMaxMessageSize};

	auto maybeHeader = headerParser.parseMessageHeader(reader);
    ASSERT_TRUE(maybeHeader.isOk());

	auto maybeMessage = parser.parseResponse(*maybeHeader, reader);
    ASSERT_TRUE(maybeMessage.isOk());

    auto& message = maybeMessage.unwrap();
    ASSERT_TRUE(std::holds_alternative<Response::Read>(message));

	auto read = std::get<Response::Read>(message);
	ASSERT_EQ(dirWriter.bytesEncoded(), read.data.size());
}


TEST_F(P9DirListingWriter, emptyDirectoryReadResponseOk) {
	auto responseWriter = ResponseWriter{_buffer, 1};
	auto dirWriter = DirListingWriter{responseWriter, 4096};
	ASSERT_EQ(dirWriter.bytesEncoded(), 0U);

	// Check response
	auto maybeParser = createResponseParser(kProtocolVersion, 128);
	ASSERT_TRUE(maybeParser.isOk());

	auto& parser = *maybeParser;
	ByteReader reader{_buffer.viewWritten()};
	auto headerParser = UnversionedParser{kMaxMessageSize};

	auto maybeHeader = headerParser.parseMessageHeader(reader);
	ASSERT_TRUE(maybeHeader.isOk());

	auto maybeMessage = parser.parseResponse(*maybeHeader, reader);
	ASSERT_TRUE(maybeMessage.isOk());

	auto& message = maybeMessage.unwrap();
	ASSERT_TRUE(std::holds_alternative<Response::Read>(message));

	auto read = std::get<Response::Read>(message);
	ASSERT_EQ(dirWriter.bytesEncoded(), read.data.size());
}
