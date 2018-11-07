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

#include "styxe/9p2000.hpp"


using namespace Solace;
using namespace styxe;


Result<void, Error>
Protocol::Decoder::read(uint8* dest) {
    return _src.readLE(*dest);
}


Result<void, Error>
Protocol::Decoder::read(uint16* dest) {
    return _src.readLE(*dest);
}

Result<void, Error>
Protocol::Decoder::read(uint32* dest) {
    return _src.readLE(*dest);
}

Result<void, Error>
Protocol::Decoder::read(uint64* dest) {
    return _src.readLE(*dest);
}

Result<void, Error>
Protocol::Decoder::read(StringView* dest) {
    uint16 dataSize = 0;

    return _src.readLE(dataSize)
            .then([&]() {
                StringView view(_src.viewRemaining().dataAs<const char>(), dataSize);
                return _src.advance(dataSize)
                        .then([dest, &view]() {
                            *dest = view;
                        });
            });
}

Result<void, Error>
Protocol::Decoder::read(Protocol::Qid* qid) {
    return read(&qid->type, &qid->version, &qid->path);
}


Result<void, Error>
Protocol::Decoder::read(Protocol::Stat* stat) {
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
Protocol::Decoder::read(MemoryView* data) {
    Protocol::size_type dataSize = 0;
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
Protocol::Decoder::read(MutableMemoryView* data) {
    return read(static_cast<MemoryView*>(data));
}


//Result<void, Error>
//readPathComponent(uint16 componentsCount) {
//}

Result<void, Error>
Protocol::Decoder::read(Path* path) {
    uint16 componentsCount = 0;

    return read(&componentsCount)
            .then([&]() -> Result<void, Error> {
                // FIXME: This is where PathBuilder can be handy.
                auto components = makeVector<String>(componentsCount);

                for (decltype (componentsCount) i = 0; i < componentsCount; ++i) {
                    StringView component;
                    auto result = read(&component);
                    if (!result) {  // Break on first error
                        return result;
                    }

                    // FIXME: Performance kick in the nuts!
                    components.emplace_back(makeString(component));
                }

                *path = makePath(components.toArray());

                return Ok();
            });

}

