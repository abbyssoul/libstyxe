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

#include "styxe/responseWriter.hpp"
#include "styxe/encoder.hpp"


using namespace Solace;
using namespace styxe;



auto noPayloadMessage(ByteWriter& buffer,
                      MessageType type, Tag tag) {
    auto header = makeHeaderWithPayload(type, tag, 0);
    auto const pos = buffer.position();

	Encoder encoder{buffer};
	encoder << header;

    return TypedWriter{buffer, pos, header};
}


TypedWriter
ResponseWriter::version(StringView version, size_type maxMessageSize) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(maxMessageSize) +
            encoder.protocolSize(version);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RVersion, Parser::NO_TAG, payloadSize);
	encoder << header
			<< maxMessageSize
			<< version;

    return TypedWriter{_buffer, pos, header};
}

TypedWriter
ResponseWriter::auth(Qid qid) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qid);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RAuth, _tag, payloadSize);
	encoder << header
			<< qid;

    return TypedWriter{_buffer, pos, header};
}

TypedWriter
ResponseWriter::error(StringView message) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(message);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RError, _tag, payloadSize);
	encoder << header
			<< message;

    return TypedWriter{_buffer, pos, header};
}

TypedWriter
ResponseWriter::flush() {
    return noPayloadMessage(_buffer, MessageType::RFlush, _tag);
}


TypedWriter
ResponseWriter::attach(Qid qid) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qid);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RAttach, _tag, payloadSize);
	encoder << header
			<< qid;

    return TypedWriter{_buffer, pos, header};
}

TypedWriter
ResponseWriter::walk(Solace::ArrayView<Qid> qids) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qids);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RWalk, _tag, payloadSize);
	encoder << header
			<< qids;

    return TypedWriter{_buffer, pos, header};
}

TypedWriter
ResponseWriter::open(Qid qid, size_type iounit) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qid) +
            encoder.protocolSize(iounit);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::ROpen, _tag, payloadSize);
	encoder << header
			<< qid
			<< iounit;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
ResponseWriter::create(Qid qid, size_type iounit) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(qid) +
            encoder.protocolSize(iounit);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RCreate, _tag, payloadSize);
	encoder << header
			<< qid
			<< iounit;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
ResponseWriter::read(MemoryView data) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(data);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RRead, _tag, payloadSize);
	encoder << header
			<< data;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
ResponseWriter::write(size_type count) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(count);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RWrite, _tag, payloadSize);
	encoder << header
			<< count;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
ResponseWriter::clunk() {
    return noPayloadMessage(_buffer, MessageType::RClunk, _tag);
}


TypedWriter
ResponseWriter::remove() {
    return noPayloadMessage(_buffer, MessageType::RRemove, _tag);
}


TypedWriter
ResponseWriter::stat(Stat const& data) {
    Encoder encoder{_buffer};

    // Compute message size first:
    var_datum_size_type const statSize = encoder.protocolSize(data);  // FIXME: Deal with stat data size over 64k
    auto const payloadSize = narrow_cast<size_type>(sizeof(var_datum_size_type) + statSize);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RStat, _tag, payloadSize);
	encoder << header
			<< statSize
			<< data;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
ResponseWriter::wstat() {
    return noPayloadMessage(_buffer, MessageType::RWStat, _tag);
}


TypedWriter
ResponseWriter::session() {
    return noPayloadMessage(_buffer, MessageType::RSession, _tag);
}



TypedWriter
ResponseWriter::shortRead(MemoryView data) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(data);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RSRead, _tag, payloadSize);
	encoder << header
			<< data;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
ResponseWriter::shortWrite(size_type count) {
    Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
            encoder.protocolSize(count);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::RSWrite, _tag, payloadSize);
	encoder << header
			<< count;

    return TypedWriter{_buffer, pos, header};
}


var_datum_size_type
DirListingWriter::sizeStat(Stat const& stat) {
   return narrow_cast<var_datum_size_type>(Encoder::protocolSize(stat) - sizeof(stat.size));
}


bool DirListingWriter::encode(Stat const& stat) {
	auto const protoSize = Encoder::protocolSize(stat);
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
	Encoder encoder{_dest};
	encoder << stat;

    return true;
}



ByteWriter&
TypedWriter::build() {
    auto const finalPos = _buffer.position();
    auto const messageSize = finalPos - _pos;  // Re-compute actual message size
    _buffer.position(_pos);  // Reset to the start position

    _header.messageSize = narrow_cast<size_type>(messageSize);
    Encoder encoder{_buffer};
	encoder << _header;

	if (_header.type == MessageType::RRead || _header.type == MessageType::RSRead) {
		auto const dataSize = narrow_cast<size_type>(finalPos - _buffer.position() - sizeof(size_type));
		encoder << dataSize;
    }

    _buffer.position(finalPos);

    return _buffer.flip();
}
