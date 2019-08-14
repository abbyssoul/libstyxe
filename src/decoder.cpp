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

#include <styxe/decoder.hpp>


using namespace Solace;
using namespace styxe;


Result<void, Error>
Decoder::read(uint8* dest) {
	return _src.readLE(*dest);
}


Result<void, Error>
Decoder::read(uint16* dest) {
    return _src.readLE(*dest);
}

Result<void, Error>
Decoder::read(uint32* dest) {
    return _src.readLE(*dest);
}

Result<void, Error>
Decoder::read(uint64* dest) {
    return _src.readLE(*dest);
}

Result<void, Error>
Decoder::read(StringView* dest) {
    uint16 dataSize = 0;

    return _src.readLE(dataSize)
            .then([&]() {
				StringView view(_src.viewRemaining().dataAs<char const>(), dataSize);
                return _src.advance(dataSize)
                        .then([dest, &view]() {
                            *dest = view;
                        });
            });
}

Result<void, Error>
Decoder::read(Qid* qid) {
    return read(&qid->type, &qid->version, &qid->path);
}


Result<void, Error>
Decoder::read(Stat* stat) {
    return read(&stat->size,
                &stat->type,
                &stat->dev,
                &(stat->qid),
                &stat->mode,
                &stat->atime,
                &stat->mtime,
                &stat->length,
                &(stat->name),
                &(stat->uid),
                &(stat->gid),
                &(stat->muid));
}


Result<void, Error>
Decoder::read(MemoryView* data) {
    size_type dataSize = 0;
    // Read size of the following data.
    return read(&dataSize)
            .then([&]() {
                if (dataSize <= _src.remaining()) {
                    // Read the data. Note we only take a view into the actual message buffer.
                    *data = _src.viewRemaining().slice(0, dataSize);
                }

                return _src.advance(dataSize);
            });
}


Result<void, Error>
Decoder::read(MutableMemoryView* data) {
    return read(static_cast<MemoryView*>(data));
}


Result<void, Error>
Decoder::read(WalkPath* path) {
	WalkPath::size_type componentsCount = 0;

	return read(&componentsCount)
			.then([&]() {
				*path = WalkPath{componentsCount, _src.viewRemaining()};
				// Advance the byteReader:
				ByteReader::size_type skip = 0;
				for (auto segment : *path) {
					skip += sizeof(var_datum_size_type) + segment.size();
				}

				return _src.advance(skip);
			});
}
