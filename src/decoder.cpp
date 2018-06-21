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


P9Protocol::Decoder&
P9Protocol::Decoder::read(uint8* dest) {
    _src.readLE(*dest);
    return *this;
}


P9Protocol::Decoder&
P9Protocol::Decoder::read(uint16* dest) {
    _src.readLE(*dest);
    return *this;
}

P9Protocol::Decoder&
P9Protocol::Decoder::read(uint32* dest) {
    _src.readLE(*dest);
    return *this;
}

P9Protocol::Decoder&
P9Protocol::Decoder::read(uint64* dest) {
    _src.readLE(*dest);
    return *this;
}

P9Protocol::Decoder&
P9Protocol::Decoder::read(StringView* dest) {
    uint16 dataSize = 0;
    _src.readLE(dataSize);

    StringView view(_src.viewRemaining().dataAs<const char>(), dataSize);
    _src.advance(dataSize);
    *dest = view;

    return *this;
}

P9Protocol::Decoder&
P9Protocol::Decoder::read(P9Protocol::Qid* qid) {
    return read(&qid->type)
            .read(&qid->version)
            .read(&qid->path);
}


P9Protocol::Decoder&
P9Protocol::Decoder::read(P9Protocol::Stat* stat) {
    return read(&stat->size)
            .read(&stat->type)
            .read(&stat->dev)
            .read(&(stat->qid))
            .read(&stat->mode)
            .read(&stat->atime)
            .read(&stat->mtime)
            .read(&stat->length)
            .read(&(stat->name))
            .read(&(stat->uid))
            .read(&(stat->gid))
            .read(&(stat->muid));
}

P9Protocol::Decoder&
P9Protocol::Decoder::read(ImmutableMemoryView* data) {
    P9Protocol::size_type dataSize = 0;
    // Read size of the following data.
    read(&dataSize);

    // Read the data. Note we only take a view into the actual message buffer.
    *data = _src.viewRemaining().slice(0, dataSize);
    _src.advance(dataSize);

    return (*this);
}

P9Protocol::Decoder&
P9Protocol::Decoder::read(Path* path) {
    uint16 componentsCount = 0;
    read(&componentsCount);

    // FIXME: This is where PathBuilder will be handy.
    std::vector<String> components;
    components.reserve(componentsCount);

    for (uint16 i = 0; i < componentsCount; ++i) {
        StringView component;
        read(&component);

        // FIXME: Performance kick in the nuts!
        components.emplace_back(component);
    }

    *path = Path(std::move(components));
    return (*this);
}

