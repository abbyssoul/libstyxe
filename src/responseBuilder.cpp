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

#include <solace/exception.hpp>


using namespace Solace;
using namespace styxe;


auto noPayloadMessage(ByteWriter& buffer,
                      Protocol::MessageType type, Protocol::Tag tag) {
    auto header = Protocol::makeHeaderWithPayload(type, tag, 0);
    auto const pos = buffer.position();

    Protocol::Encoder(buffer)
            .header(type, tag, 0);

    return Protocol::TypedWriter{buffer, pos, header};
}


Protocol::TypedWriter
Protocol::ResponseBuilder::version(StringView version, size_type maxMessageSize) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(maxMessageSize) +
            encoder.protocolSize(version);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RVersion, NO_TAG, payloadSize);
    encoder.encode(header)
            .encode(maxMessageSize)
            .encode(version);

    return TypedWriter{_buffer, pos, header};
}

Protocol::TypedWriter
Protocol::ResponseBuilder::auth(Qid qid) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qid);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RAuth, _tag, payloadSize);
    encoder.encode(header)
            .encode(qid);

    return TypedWriter{_buffer, pos, header};
}

Protocol::TypedWriter
Protocol::ResponseBuilder::error(StringView message) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(message);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RError, _tag, payloadSize);
    encoder.encode(header)
            .encode(message);

    return TypedWriter{_buffer, pos, header};
}

Protocol::TypedWriter
Protocol::ResponseBuilder::flush() {
    return noPayloadMessage(_buffer, MessageType::RFlush, _tag);
}


Protocol::TypedWriter
Protocol::ResponseBuilder::attach(Qid qid) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qid);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RAttach, _tag, payloadSize);
    encoder.encode(header)
            .encode(qid);

    return TypedWriter{_buffer, pos, header};
}

Protocol::TypedWriter
Protocol::ResponseBuilder::walk(Solace::ArrayView<Qid> const& qids) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qids);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RWalk, _tag, payloadSize);
    encoder.encode(header)
            .encode(qids);

    return TypedWriter{_buffer, pos, header};
}

Protocol::TypedWriter
Protocol::ResponseBuilder::open(Qid qid, size_type iounit) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qid) +
            encoder.protocolSize(iounit);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::ROpen, _tag, payloadSize);
    encoder.encode(header)
            .encode(qid)
            .encode(iounit);

    return TypedWriter{_buffer, pos, header};
}


Protocol::TypedWriter
Protocol::ResponseBuilder::create(Qid qid, size_type iounit) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qid) +
            encoder.protocolSize(iounit);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RCreate, _tag, payloadSize);
    encoder.encode(header)
            .encode(qid)
            .encode(iounit);

    return TypedWriter{_buffer, pos, header};
}


Protocol::TypedWriter
Protocol::ResponseBuilder::read(MemoryView data) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(data);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RRead, _tag, payloadSize);
    encoder.encode(header)
            .encode(data);

    return TypedWriter{_buffer, pos, header};
}


Protocol::TypedWriter
Protocol::ResponseBuilder::write(size_type count) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(count);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RWrite, _tag, payloadSize);
    encoder.encode(header)
            .encode(count);

    return TypedWriter{_buffer, pos, header};
}


Protocol::TypedWriter
Protocol::ResponseBuilder::clunk() {
    return noPayloadMessage(_buffer, MessageType::RClunk, _tag);
}


Protocol::TypedWriter
Protocol::ResponseBuilder::remove() {
    return noPayloadMessage(_buffer, MessageType::RRemove, _tag);
}


Protocol::TypedWriter
Protocol::ResponseBuilder::stat(const Stat& data) {
    Encoder encoder{_buffer};

    // Compute message size first:
    uint16 const statSize = encoder.protocolSize(data);  // FIXME: Deal with stat data size over 64k
    auto const payloadSize = narrow_cast<size_type>(sizeof(uint16) + statSize);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RStat, _tag, payloadSize);
    encoder.encode(header)
            .encode(statSize) //static_cast<uint16>(payloadSize))
            .encode(data);

    return TypedWriter{_buffer, pos, header};
}


Protocol::TypedWriter
Protocol::ResponseBuilder::wstat() {
    return noPayloadMessage(_buffer, MessageType::RWStat, _tag);
}


Protocol::TypedWriter
Protocol::ResponseBuilder::session() {
    return noPayloadMessage(_buffer, MessageType::RSession, _tag);
}



Protocol::TypedWriter
Protocol::ResponseBuilder::shortRead(MemoryView data) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(data);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RSRead, _tag, payloadSize);
    encoder.encode(header)
            .encode(data);

    return TypedWriter{_buffer, pos, header};
}


Protocol::TypedWriter
Protocol::ResponseBuilder::shortWrite(size_type count) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(count);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RSWrite, _tag, payloadSize);
    encoder.encode(header)
            .encode(count);

    return TypedWriter{_buffer, pos, header};
}



bool DirListingWriter::encode(Protocol::Stat const& stat) {
    const auto protoSize = _encoder.protocolSize(stat);
    // Keep count of how many data we have traversed.
    _bytesTraversed += protoSize;
    if (_bytesTraversed <= offset) {  // Client is only interested in data pass the offset.
        return true;
    }

    // Keep track of much data will end up in a buffer to prevent overflow.
    _bytesEncoded += protoSize;
    if (_bytesEncoded > count) {
        return false;
    }

    // Only encode the data if we have some room left, as specified by 'count' arg.
    _encoder.encode(stat);

    return true;
}
