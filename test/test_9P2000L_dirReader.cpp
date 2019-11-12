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

	ASSERT_EQ(reader.being(), reader.end());
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

	auto i = reader.being();
	ASSERT_NE(i, reader.end());
	ASSERT_EQ(entry, *i);
	ASSERT_NE(i, reader.end());


	ASSERT_EQ(++i, reader.end());
}



TEST(P92000L, dirReader_shortBuffer) {
	char buffer[127];
	MutableMemoryView data = wrapMemory(buffer);
	ByteWriter dirStream{data};

	_9P2000L::DirEntry entries[] = {
		{randomQid(), 0, 31, StringView{"Awesome file"}},
		{randomQid(), 1, 32, StringView{"other file"}}
	};

	styxe::Encoder encoder{dirStream};
	encoder << entries[0] << entries[1];


	_9P2000L::DirEntryReader reader{dirStream.viewWritten()};  // .slice(0, dirStream.position() - 10)};

	auto i = reader.being();
	ASSERT_NE(i, reader.end());
	ASSERT_EQ(entries[0], *i);
	ASSERT_NE(i, reader.end());
	ASSERT_NE(++i, reader.end());
	ASSERT_EQ(entries[1], *i);
	ASSERT_EQ(++i, reader.end());
}
