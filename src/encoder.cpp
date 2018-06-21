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



P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const uint8& value) {
    return sizeof(value);
}

P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const uint16& value) {
    return sizeof(value);
}

P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const uint32& value) {
    return sizeof(value);
}

P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const uint64& value) {
    return sizeof(value);
}

P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const StringView& str) {
    return sizeof(uint16) +         // Space for string var size
            str.size();             // Space for the actual string bytes
}

P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const Path& path) {
    P9Protocol::size_type payloadSize = 0;
    for (auto& segment : path) {
        payloadSize += protocolSize(segment.view());
    }

    return sizeof(uint16) +  // Var number of segments
            payloadSize;
}


P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const Qid&) {
    static constexpr size_type kQidSize = sizeof(Qid::type) +
            sizeof(Qid::version) +
            sizeof(Qid::path);

    static_assert(kQidSize == 13, "Incorrect Qid struct size");

    return kQidSize;
}


P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const Stat& stat) {
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


P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const Array<Qid>& qids) {
    Qid uselessQid;

    return sizeof(uint16) +  // Var number of elements
            qids.size() * protocolSize(uselessQid);
}


P9Protocol::size_type
P9Protocol::Encoder::protocolSize(const ImmutableMemoryView& data) {
    return sizeof(size_type) +  // Var number of elements
            data.size();
}


/**
 * P9 protocol helper class to encode data into a protocol message format.
 */
P9Protocol::Encoder&
P9Protocol::Encoder::header(P9Protocol::MessageType type, P9Protocol::Tag tag, P9Protocol::size_type payloadSize) {
    return encode(P9Protocol::headerSize() + payloadSize)
            .encode(static_cast<byte>(type))
            .encode(tag);
}


P9Protocol::Encoder&
P9Protocol::Encoder::encode(uint8 value) {
    _dest.writeLE(value);
    return (*this);
}

P9Protocol::Encoder&
P9Protocol::Encoder::encode(uint16 value) {
    _dest.writeLE(value);
    return (*this);
}

P9Protocol::Encoder&
P9Protocol::Encoder::encode(uint32 value) {
    _dest.writeLE(value);
    return (*this);
}

P9Protocol::Encoder&
P9Protocol::Encoder::encode(uint64 value) {
    _dest.writeLE(value);
    return (*this);
}

P9Protocol::Encoder&
P9Protocol::Encoder::encode(const StringView& str) {
    encode(str.size());
    _dest.write(str.view());

    return (*this);
}

P9Protocol::Encoder&
P9Protocol::Encoder::encode(const P9Protocol::Qid& qid) {
    return encode(qid.type)
            .encode(qid.version)
            .encode(qid.path);
}

P9Protocol::Encoder&
P9Protocol::Encoder::encode(const Array<P9Protocol::Qid>& qids) {
    encode(static_cast<uint16>(qids.size()));
    for (auto qid : qids) {
        encode(qid);
    }

    return (*this);
}

P9Protocol::Encoder&
P9Protocol::Encoder::encode(const P9Protocol::Stat& stat) {
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

P9Protocol::Encoder&
P9Protocol::Encoder::encode(const ImmutableMemoryView& data) {
    encode(static_cast<P9Protocol::size_type>(data.size()));
    _dest.write(data);

    return (*this);
}

P9Protocol::Encoder&
P9Protocol::Encoder::encode(const Path& path) {
    encode(static_cast<uint16>(path.getComponentsCount()));

    for (const auto& component : path) {
        encode(component.view());
    }

    return (*this);
}

