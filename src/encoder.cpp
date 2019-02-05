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

#include <solace/utils.hpp>  // narrow_cast
#include <limits>



using namespace Solace;
using namespace styxe;



Protocol::size_type
Protocol::Encoder::protocolSize(const uint8& value) {
    return sizeof(value);
}

Protocol::size_type
Protocol::Encoder::protocolSize(const uint16& value) {
    return sizeof(value);
}

Protocol::size_type
Protocol::Encoder::protocolSize(const uint32& value) {
    return sizeof(value);
}

Protocol::size_type
Protocol::Encoder::protocolSize(const uint64& value) {
    return sizeof(value);
}

Protocol::size_type
Protocol::Encoder::protocolSize(const StringView& str) {
    return sizeof(uint16) +         // Space for string var size
            str.size();             // Space for the actual string bytes
}

Protocol::size_type
Protocol::Encoder::protocolSize(const Path& path) {
    assertIndexInRange(path.getComponentsCount(), 0,
                       static_cast<Path::size_type>(std::numeric_limits<uint16>::max()));

    Protocol::size_type payloadSize = 0;
    for (auto& segment : path) {
        payloadSize += protocolSize(segment.view());
    }

    return sizeof(uint16) +  // Var number of segments
            payloadSize;
}


Protocol::size_type
Protocol::Encoder::protocolSize(const Qid&) {
    static constexpr size_type kQidSize = sizeof(Qid::type) +
            sizeof(Qid::version) +
            sizeof(Qid::path);

    // Qid has a fixed size of 13 bytes, lets keep it that way
    static_assert(kQidSize == 13, "Incorrect Qid struct size");

    return kQidSize;
}


Protocol::size_type
Protocol::Encoder::protocolSize(const Stat& stat) {
    return  protocolSize(stat.size) +
            protocolSize(stat.type) +
            protocolSize(stat.dev) +
            protocolSize(stat.qid) +
            protocolSize(stat.mode) +
            protocolSize(stat.atime) +
            protocolSize(stat.mtime) +
            protocolSize(stat.length) +
            protocolSize(stat.name) +
            protocolSize(stat.uid) +
            protocolSize(stat.gid) +
            protocolSize(stat.muid);
}


Protocol::size_type
Protocol::Encoder::protocolSize(Solace::ArrayView<Qid> qids) {
    assertIndexInRange(qids.size(), 0,
                       static_cast<ArrayView<Qid>::size_type>(std::numeric_limits<uint16>::max()));

    Qid uselessQid;

    return sizeof(uint16) +  // Var number of elements
            qids.size() * protocolSize(uselessQid);
}


Protocol::size_type
Protocol::Encoder::protocolSize(const MemoryView& data) {
    assertIndexInRange(data.size(), 0,
                       static_cast<MemoryView::size_type>(std::numeric_limits<size_type>::max()));

    return sizeof(size_type) +  // Var number of elements
            narrow_cast<size_type>(data.size());
}


Protocol::Encoder&
Protocol::Encoder::header(Solace::byte customMessageType, Tag tag, size_type payloadSize) {
    return encode(Protocol::headerSize() + payloadSize)
            .encode(customMessageType)
            .encode(tag);
}



Protocol::Encoder&
Protocol::Encoder::encode(MessageHeader header) {
    return encode(header.messageSize)
            .encode(header.type)
            .encode(header.tag);
}


Protocol::Encoder&
Protocol::Encoder::encode(uint8 value) {
    _dest.writeLE(value);
    return (*this);
}

Protocol::Encoder&
Protocol::Encoder::encode(uint16 value) {
    _dest.writeLE(value);
    return (*this);
}

Protocol::Encoder&
Protocol::Encoder::encode(uint32 value) {
    _dest.writeLE(value);
    return (*this);
}

Protocol::Encoder&
Protocol::Encoder::encode(uint64 value) {
    _dest.writeLE(value);
    return (*this);
}

Protocol::Encoder&
Protocol::Encoder::encode(StringView str) {
    encode(str.size());
    _dest.write(str.view());

    return (*this);
}

Protocol::Encoder&
Protocol::Encoder::encode(Qid qid) {
    return encode(qid.type)
            .encode(qid.version)
            .encode(qid.path);
}

Protocol::Encoder&
Protocol::Encoder::encode(ArrayView<Protocol::Qid> qids) {
    encode(static_cast<uint16>(qids.size()));
    for (auto qid : qids) {
        encode(qid);
    }

    return (*this);
}

Protocol::Encoder&
Protocol::Encoder::encode(const Protocol::Stat& stat) {
    return encode(stat.size)
            .encode(stat.type)
            .encode(stat.dev)
            .encode(stat.qid)
            .encode(stat.mode)
            .encode(stat.atime)
            .encode(stat.mtime)
            .encode(stat.length)
            .encode(stat.name)
            .encode(stat.uid)
            .encode(stat.gid)
            .encode(stat.muid);
}


Protocol::Encoder&
Protocol::Encoder::encode(MemoryView data) {
    encode(static_cast<Protocol::size_type>(data.size()));
    _dest.write(data);

    return (*this);
}

Protocol::Encoder&
Protocol::Encoder::encode(const Path& path) {
    encode(static_cast<uint16>(path.getComponentsCount()));

    for (const auto& component : path) {
        encode(component.view());
    }

    return (*this);
}

