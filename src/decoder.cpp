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

#include "styxe/decoder.hpp"


using namespace Solace;
using namespace styxe;


Result<Decoder&, Error>
styxe::operator>> (Decoder& decoder, uint8& dest) {
	auto result = decoder.buffer().readLE(dest);
	if (!result) return result.moveError();

	return Result<Decoder&, Error>{types::okTag, decoder};
}


Result<Decoder&, Error>
styxe::operator>> (Decoder& decoder, uint16& dest) {
	auto result = decoder.buffer().readLE(dest);
	if (!result) return result.moveError();

	return Result<Decoder&, Error>{types::okTag, decoder};
}

Result<Decoder&, Error>
styxe::operator>> (Decoder& decoder, uint32& dest) {
	auto result = decoder.buffer().readLE(dest);
	if (!result) return result.moveError();

	return Result<Decoder&, Error>{types::okTag, decoder};
}

Result<Decoder&, Error>
styxe::operator>> (Decoder& decoder, uint64& dest) {
	auto result = decoder.buffer().readLE(dest);
	if (!result) return result.moveError();

	return Result<Decoder&, Error>{types::okTag, decoder};
}

Result<Decoder&, Error>
styxe::operator>> (Decoder& decoder, StringView& dest) {
	auto& buffer = decoder.buffer();

	uint16 dataSize = 0;
	auto result = buffer.readLE(dataSize)
			.then([&]() {
				StringView view{buffer.viewRemaining().dataAs<char const>(), dataSize};
				return buffer.advance(dataSize)
						.then([&dest, &view]() {
							dest = view;
                        });
            });

	if (!result) return result.moveError();

	return Result<Decoder&, Error>{types::okTag, decoder};
}


Result<Decoder&, Error>
styxe::operator>> (Decoder& decoder, MemoryView& data) {
	auto& buffer = decoder.buffer();
	styxe::size_type dataSize = 0;

    // Read size of the following data.
	auto result = buffer.readLE(dataSize)
			.then([&]() {
				data = buffer.viewRemaining().slice(0, dataSize);
				return buffer.advance(dataSize);
            });
	if (!result) return result.moveError();

	return Result<Decoder&, Error>{types::okTag, decoder};
}


Result<Decoder&, Error>
styxe::operator>> (Decoder& decoder, WalkPath& path) {
	auto& buffer = decoder.buffer();
	WalkPath::size_type componentsCount = 0;

	auto result = buffer.readLE(componentsCount)
			.then([&]() {
				path = WalkPath{componentsCount, buffer.viewRemaining()};
				// Advance the byteReader:
				ByteReader::size_type skip = 0;
				for (auto segment : path) {
					skip += sizeof(styxe::var_datum_size_type) + segment.size();
				}

				return buffer.advance(skip);
			});

	if (!result) return result.moveError();

	return Result<Decoder&, Error>{types::okTag, decoder};
}


