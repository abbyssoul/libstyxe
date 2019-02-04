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

#include <gtest/gtest.h>


using namespace Solace;
using namespace styxe;


class P9MessageBuilder : public ::testing::Test {
public:
    P9MessageBuilder() :
        _memManager(Protocol::MAX_MESSAGE_SIZE)
    {}


protected:

    void SetUp() override {
        _buffer = _memManager.allocate(Protocol::MAX_MESSAGE_SIZE);
        _buffer.viewRemaining().fill(0xFE);
    }

    MemoryManager   _memManager;
    ByteWriter     _buffer;
};


TEST_F(P9MessageBuilder, messageChanging) {
    MemoryView emptyBuffer;
    Protocol::ResponseBuilder builder(_buffer, Protocol::NO_TAG);

    builder.read(emptyBuffer);
    ASSERT_EQ(Protocol::headerSize() + 4 + 0, _buffer.position());
    ASSERT_EQ(Protocol::MessageType::RRead, builder.type());

    // Change message type
    const char* message = "Nothing to read";
    const auto payloadSize = strlen(message) + 2;
    builder.error(StringView{message});

    ASSERT_EQ(payloadSize, builder.payloadSize());
    ASSERT_EQ(Protocol::headerSize() + payloadSize, _buffer.position());
    ASSERT_EQ(Protocol::MessageType::RError, builder.type());
}
