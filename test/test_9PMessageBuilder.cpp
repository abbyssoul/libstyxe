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
 * @file: test/test_9PMessageBuilder.cpp
 *
 *******************************************************************************/
#include "styxe/9p2000.hpp"  // Class being tested

#include <solace/exception.hpp>

#include "gtest/gtest.h"


using namespace Solace;
using namespace styxe;


class P9MessageBuilder : public ::testing::Test {
public:
    P9MessageBuilder() :
        _memManager(Protocol::MAX_MESSAGE_SIZE)
    {}


protected:

    void SetUp() override {
        _buffer = _memManager.create(Protocol::MAX_MESSAGE_SIZE);
        _buffer.viewRemaining().fill(0xFE);
    }

    MemoryManager   _memManager;
    ByteBuffer      _buffer;
};



TEST_F(P9MessageBuilder, payloadResizing) {
    ImmutableMemoryView emptyBuffer;
    Protocol::ResponseBuilder builder(_buffer, Protocol::NO_TAG);

    builder.read(emptyBuffer);
    ASSERT_EQ(4, builder.payloadSize());
    ASSERT_EQ(Protocol::headerSize() + 4 + 0, _buffer.position());

    // Write extra data:
    const byte extraData[] = {1, 3, 2, 45, 18};
    builder.buffer().write(extraData, 5);

    builder.updatePayloadSize();
    ASSERT_EQ(5 + 4, builder.payloadSize());
    ASSERT_EQ(Protocol::headerSize() + 4 + 5, _buffer.position());

    builder.updatePayloadSize(3);
    ASSERT_EQ(3, builder.payloadSize());
    ASSERT_EQ(Protocol::headerSize() + 3, _buffer.position());
}



TEST_F(P9MessageBuilder, messageChanging) {
    ImmutableMemoryView emptyBuffer;
    Protocol::ResponseBuilder builder(_buffer, Protocol::NO_TAG);

    builder.read(emptyBuffer);
    ASSERT_EQ(Protocol::headerSize() + 4 + 0, _buffer.position());
    ASSERT_EQ(Protocol::MessageType::RRead, builder.type());

    // Change message type
    const char* message = "Nothing to read";
    const auto payloadSize = strlen(message) + 2;
    builder.error(message);

    ASSERT_EQ(payloadSize, builder.payloadSize());
    ASSERT_EQ(Protocol::headerSize() + payloadSize, _buffer.position());
    ASSERT_EQ(Protocol::MessageType::RError, builder.type());
}
