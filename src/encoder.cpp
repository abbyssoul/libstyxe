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


//Encoder&
//Encoder::header(Solace::byte customMessageType, Tag tag, size_type payloadSize) {
//    return encode(headerSize() + payloadSize)
//            .encode(customMessageType)
//            .encode(tag);
//}


Encoder&
styxe::operator<< (Encoder& encoder, uint8 value) {
	encoder.buffer().writeLE(value);
	return encoder;
}

Encoder& styxe::operator<< (Encoder& encoder, uint16 value) {
	encoder.buffer().writeLE(value);
	return encoder;
}

Encoder& styxe::operator<< (Encoder& encoder, uint32 value) {
	encoder.buffer().writeLE(value);
	return encoder;
}

Encoder& styxe::operator<< (Encoder& encoder, uint64 value) {
	encoder.buffer().writeLE(value);
	return encoder;
}

Encoder& styxe::operator<< (Encoder& encoder, StringView str) {
	encoder << str.size();
	encoder.buffer().write(str.view());

	return encoder;
}


Encoder& styxe::operator<< (Encoder& encoder, MemoryView data) {
	encoder << narrow_cast<size_type>(data.size());
	encoder.buffer().write(data);

	return encoder;
}
