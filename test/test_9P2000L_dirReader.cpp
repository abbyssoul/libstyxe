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
#include "styxe/9p2000L.hpp"

#include "testHarnes.hpp"

using namespace Solace;
using namespace styxe;


TEST(P92000L, emptyDirReader) {
	MemoryView data{};

	_9P2000L::DirEntryReader reader{data};

	ASSERT_EQ(begin(reader), end(reader));
}


TEST(P92000L, dirReader) {
	char buffer[127];
	MutableMemoryView data = wrapMemory(buffer);
	ByteWriter dirStream{data};

	_9P2000L::DirEntry entry {
		randomQid(),
				0,
				31,
				StringView{"Awesome file"}
	};

	styxe::Encoder encoder{dirStream};
	encoder << entry;


	_9P2000L::DirEntryReader reader{dirStream.viewWritten()};

	auto i = begin(reader);
	ASSERT_NE(i, end(reader));
	ASSERT_EQ(entry, *i);
	ASSERT_NE(i, end(reader));


	ASSERT_EQ(++i, end(reader));
}



TEST(P92000L, dirReader_multiple_enties) {
	char buffer[127];
	MutableMemoryView data = wrapMemory(buffer);
	ByteWriter dirStream{data};

	_9P2000L::DirEntry entries[] = {
		{randomQid(), 0, 31, StringView{"data"}},
		{randomQid(), 4, 31, StringView{"Awesome file"}},
		{randomQid(), 1, 32, StringView{"other file"}}
	};

	styxe::Encoder encoder{dirStream};
	for (auto const& e : entries) {
		encoder << e;
	}


	_9P2000L::DirEntryReader reader{dirStream.viewWritten()};
	size_t index = 0;
	for (auto const& ent : reader) {
		ASSERT_EQ(entries[index], ent);
		index += 1;
	}
}


TEST(P92000L, dirReader_incomplete_buffer_1) {
	char buffer[127];
	MutableMemoryView data = wrapMemory(buffer);
	ByteWriter dirStream{data};

	_9P2000L::DirEntry entries[] = {
		{randomQid(), 0, 31, StringView{"data"}},
	};

	styxe::Encoder encoder{dirStream};
	for (auto const& e : entries) {
		encoder << e;
	}

	_9P2000L::DirEntryReader reader{dirStream.viewWritten() .slice(0, dirStream.position() - 10)};
	size_t index = 0;
	for (auto ent : reader) {
		ASSERT_EQ(entries[index], ent);
		index += 1;
	}

	ASSERT_EQ(0, index);
}


TEST(P92000L, dirReader_incomplete_buffer_2) {
	char buffer[127];
	MutableMemoryView data = wrapMemory(buffer);

	_9P2000L::DirEntry const entries[] = {
		{randomQid(), 0, 31, StringView{"data"}},
		{randomQid(), 4, 31, StringView{"Awesome file"}},
		{randomQid(), 1, 32, StringView{"other file"}}
	};

	ByteWriter dirStream{data};
	styxe::Encoder encoder{dirStream};
	for (auto const& e : entries) {
		encoder << e;
	}

	_9P2000L::DirEntryReader reader{dirStream.viewWritten() .slice(0, dirStream.position() - 10)};
	size_t index = 0;
	for (auto ent : reader) {
		ASSERT_EQ(entries[index], ent);
		index += 1;
	}

	ASSERT_EQ(2, index);
}
