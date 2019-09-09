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
 * @file: test/test_9PMessageBuilder.cpp
 *
 *******************************************************************************/
#include "styxe/responseWriter.hpp"  // Class being tested

#include <solace/exception.hpp>

#include <gtest/gtest.h>


using namespace Solace;
using namespace styxe;


class P9MessageBuilder : public ::testing::Test {
public:

	P9MessageBuilder()
		: _memManager{kMaxMesssageSize}
    {}


protected:

    void SetUp() override {
		_buffer = _memManager.allocate(kMaxMesssageSize).unwrap();
        _buffer.viewRemaining().fill(0xFE);
    }

    MemoryManager   _memManager;
    ByteWriter     _buffer;
};


TEST_F(P9MessageBuilder, dirListingMessage) {
	auto responseWriter = ResponseWriter{_buffer, 1}
            .read(MemoryView{});  // Prime read request 0 size data!

    DirListingWriter writer{_buffer, 4096, 0};

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

    for (auto& stat : testStats) {
        stat.size = DirListingWriter::sizeStat(stat);
    }

    for (auto const& stat : testStats) {
        if (!writer.encode(stat))
            break;
    }

    auto& buf = responseWriter.build();
    ByteReader reader{buf.viewRemaining()};

    Parser parser;
    auto maybeHeader = parser.parseMessageHeader(reader);
    ASSERT_TRUE(maybeHeader.isOk());

    auto maybeMessage = parser.parseResponse(maybeHeader.unwrap(), reader);
    ASSERT_TRUE(maybeMessage.isOk());

    auto& message = maybeMessage.unwrap();
    ASSERT_TRUE(std::holds_alternative<Response::Read>(message));

    auto read = std::get<Response::Read>(message);

    ASSERT_EQ(writer.bytesEncoded(), read.data.size());
}
