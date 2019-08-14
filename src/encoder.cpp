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

#include <styxe/encoder.hpp>


#include <solace/utils.hpp>  // narrow_cast
#include <limits>



using namespace Solace;
using namespace styxe;



size_type
Encoder::protocolSize(uint8 const& value) noexcept {
    return sizeof(value);
}

size_type
Encoder::protocolSize(uint16 const& value) noexcept {
    return sizeof(value);
}

size_type
Encoder::protocolSize(uint32 const& value) noexcept {
    return sizeof(value);
}

size_type
Encoder::protocolSize(uint64 const& value) noexcept {
    return sizeof(value);
}

size_type
Encoder::protocolSize(StringView const& str) noexcept {
    return sizeof(var_datum_size_type) +         // Space for string var size
            str.size();             // Space for the actual string bytes
}

//size_type
//Encoder::protocolSize(Path const& path) noexcept {
//    assertIndexInRange(path.getComponentsCount(), 0,
//                       static_cast<Path::size_type>(std::numeric_limits<uint16>::max()));

//    size_type payloadSize = 0;
//    for (auto& segment : path) {
//        payloadSize += protocolSize(segment.view());
//    }

//    return sizeof(var_datum_size_type) +  // Var number of segments
//            payloadSize;
//}


size_type
Encoder::protocolSize(Qid const&) noexcept {
    static constexpr size_type kQidSize = sizeof(Qid::type) + sizeof(Qid::version) + sizeof(Qid::path);

    // Qid has a fixed size of 13 bytes, lets keep it that way
    static_assert(kQidSize == 13, "Incorrect Qid struct size");

    return kQidSize;
}


size_type
Encoder::protocolSize(Stat const& stat) noexcept {
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


size_type
Encoder::protocolSize(Solace::ArrayView<Qid> qids) noexcept {
    assertIndexInRange(qids.size(), 0,
                       static_cast<ArrayView<Qid>::size_type>(std::numeric_limits<uint16>::max()));

    Qid uselessQid;

    return sizeof(var_datum_size_type) +  // Var number of elements
            qids.size() * protocolSize(uselessQid);
}


size_type
Encoder::protocolSize(MemoryView const& data) noexcept {
    assertIndexInRange(data.size(), 0,
                       static_cast<MemoryView::size_type>(std::numeric_limits<size_type>::max()));

    return sizeof(size_type) +  // Var number of elements
            narrow_cast<size_type>(data.size());
}


Encoder&
Encoder::header(Solace::byte customMessageType, Tag tag, size_type payloadSize) {
    return encode(headerSize() + payloadSize)
            .encode(customMessageType)
            .encode(tag);
}


Encoder&
Encoder::encode(MessageHeader header) {
    return encode(header.messageSize)
            .encode(header.type)
            .encode(header.tag);
}


Encoder&
Encoder::encode(uint8 value) {
    _dest.writeLE(value);
    return (*this);
}

Encoder&
Encoder::encode(uint16 value) {
    _dest.writeLE(value);
    return (*this);
}

Encoder&
Encoder::encode(uint32 value) {
    _dest.writeLE(value);
    return (*this);
}

Encoder&
Encoder::encode(uint64 value) {
    _dest.writeLE(value);
    return (*this);
}

Encoder&
Encoder::encode(StringView str) {
    encode(str.size());
    _dest.write(str.view());

    return (*this);
}

Encoder&
Encoder::encode(Qid qid) {
    return encode(qid.type)
            .encode(qid.version)
            .encode(qid.path);
}

Encoder&
Encoder::encode(ArrayView<Qid> qids) {
    // Encode variable datum size first:
    encode(narrow_cast<var_datum_size_type>(qids.size()));

    // Datum
    for (auto const& qid : qids) {
        encode(qid);
    }

    return (*this);
}

Encoder&
Encoder::encode(Stat const& stat) {
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


Encoder&
Encoder::encode(MemoryView data) {
    encode(narrow_cast<size_type>(data.size()));
    _dest.write(data);

    return (*this);
}

//Encoder&
//Encoder::encode(Path const& path) {
//    // Encode variable datum size first:
//    encode(narrow_cast<var_datum_size_type>(path.getComponentsCount()));

//    // Datum
//    for (auto const& component : path) {
//        encode(component.view());
//    }

//    return (*this);
//}

