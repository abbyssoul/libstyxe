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
#include "styxe/encoder.hpp"


using namespace Solace;
using namespace styxe;



auto noPayloadMessage(ByteWriter& buffer,
                      MessageType type, Tag tag) {
    auto header = makeHeaderWithPayload(type, tag, 0);
    auto const pos = buffer.position();

    Encoder(buffer)
            .header(type, tag, 0);

    return TypedWriter{buffer, pos, header};
}


TypedWriter
ResponseBuilder::version(StringView version, size_type maxMessageSize) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(maxMessageSize) +
            encoder.protocolSize(version);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RVersion, Parser::NO_TAG, payloadSize);
    encoder.encode(header)
            .encode(maxMessageSize)
            .encode(version);

    return TypedWriter{_buffer, pos, header};
}

TypedWriter
ResponseBuilder::auth(Qid qid) {
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

TypedWriter
ResponseBuilder::error(StringView message) {
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

TypedWriter
ResponseBuilder::flush() {
    return noPayloadMessage(_buffer, MessageType::RFlush, _tag);
}


TypedWriter
ResponseBuilder::attach(Qid qid) {
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

TypedWriter
ResponseBuilder::walk(ArrayView<Qid> const& qids) {
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

TypedWriter
ResponseBuilder::open(Qid qid, size_type iounit) {
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


TypedWriter
ResponseBuilder::create(Qid qid, size_type iounit) {
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


TypedWriter
ResponseBuilder::read(MemoryView data) {
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


TypedWriter
ResponseBuilder::write(size_type count) {
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


TypedWriter
ResponseBuilder::clunk() {
    return noPayloadMessage(_buffer, MessageType::RClunk, _tag);
}


TypedWriter
ResponseBuilder::remove() {
    return noPayloadMessage(_buffer, MessageType::RRemove, _tag);
}


TypedWriter
ResponseBuilder::stat(const Stat& data) {
    Encoder encoder{_buffer};

    // Compute message size first:
    var_datum_size_type const statSize = encoder.protocolSize(data);  // FIXME: Deal with stat data size over 64k
    auto const payloadSize = narrow_cast<size_type>(sizeof(var_datum_size_type) + statSize);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RStat, _tag, payloadSize);
    encoder.encode(header)
            .encode(statSize)
            .encode(data);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
ResponseBuilder::wstat() {
    return noPayloadMessage(_buffer, MessageType::RWStat, _tag);
}


TypedWriter
ResponseBuilder::session() {
    return noPayloadMessage(_buffer, MessageType::RSession, _tag);
}



TypedWriter
ResponseBuilder::shortRead(MemoryView data) {
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


TypedWriter
ResponseBuilder::shortWrite(size_type count) {
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


var_datum_size_type
DirListingWriter::sizeStat(Stat const& stat) {
   return narrow_cast<var_datum_size_type>(Encoder::protocolSize(stat) - sizeof(stat.size));
}


bool DirListingWriter::encode(Stat const& stat) {
    const auto protoSize = Encoder::protocolSize(stat);
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
    Encoder{_dest}.encode(stat);

    return true;
}



ByteWriter&
TypedWriter::build() {
    auto const finalPos = _buffer.position();
    auto const messageSize = finalPos - _pos;  // Re-compute actual message size
    _buffer.position(_pos);  // Reset to the start position

    header.messageSize = narrow_cast<size_type>(messageSize);
    Encoder encoder{_buffer};
    encoder.encode(header);

    if (header.type == MessageType::RRead) {
        encoder.encode(narrow_cast<size_type>(finalPos - sizeof(size_type) - _buffer.position()));
    }

    _buffer.position(finalPos);

    return _buffer.flip();
}
