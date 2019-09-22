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

#include "styxe/requestWriter.hpp"
#include "styxe/encoder.hpp"


using namespace Solace;
using namespace styxe;

TypedWriter
RequestWriter::DataWriter::data(MemoryView data) {
	Encoder encoder{buffer()};
	encoder << data;

	return *this;
}


RequestWriter::PathWriter::PathWriter(ByteWriter& writer,
									  ByteWriter::size_type pos,
									  MessageHeader header) noexcept
	: TypedWriter{writer, pos, header}
	, _segmentsPos{writer.position()}
{
	Encoder encoder{writer};
	encoder << _nSegments;
}

RequestWriter::PathWriter&
RequestWriter::PathWriter::path(StringView pathSegment) {
	_nSegments += 1;

	auto& writer = buffer();
	Encoder encoder(writer);

	auto const currentPos = writer.position();
	writer.position(_segmentsPos);
	encoder << _nSegments;
	writer.position(currentPos);

	encoder << pathSegment;

	return *this;
}

RequestWriter::PathDataWriter::PathDataWriter(ByteWriter& writer,
											  ByteWriter::size_type pos,
											  MessageHeader head) noexcept
	: DataWriter{writer, pos, head}
	, _segmentsPos{writer.position()}
{
	Encoder encoder{writer};
	encoder << _nSegments;
}


RequestWriter::PathDataWriter&
RequestWriter::PathDataWriter::path(StringView pathSegment) {
	_nSegments += 1;

	auto& writer = buffer();
	Encoder encoder{writer};

	auto const currentPos = writer.position();
	writer.position(_segmentsPos);
	encoder << _nSegments;
	writer.position(currentPos);

	encoder << pathSegment;

	return *this;
}




TypedWriter
RequestWriter::version(StringView version, size_type maxMessageSize) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(maxMessageSize) +                // Negotiated message size field
			encoder.protocolSize(version);   // Version string data

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TVersion, Parser::NO_TAG, payloadSize);
	encoder << header
			<< maxMessageSize
			<< version;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::auth(Fid afid, StringView userName, StringView attachName) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(afid) +                  // Proposed fid for authentication mechanism
			encoder.protocolSize(userName) +       // User name
			encoder.protocolSize(attachName);     // Root name where we want to attach to

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TAuth, _tag, payloadSize);
	encoder << header
			<< afid
			<< userName
			<< attachName;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::attach(Fid fid, Fid afid, StringView userName, StringView attachName) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid) +                  // Proposed fid for the attached root
			encoder.protocolSize(afid) +                  // Fid of the passed authentication
			encoder.protocolSize(userName) +      // User name
			encoder.protocolSize(attachName);     // Root name where we want to attach to

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TAttach, _tag, payloadSize);
	encoder << header
			<< fid
			<< afid
			<< userName
			<< attachName;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::clunk(Fid fid) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid);               // Fid to forget

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TClunk, _tag, payloadSize);
	encoder << header
			<< fid;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::flush(Tag oldTransation) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(oldTransation);               // Transaction to forget

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TFlush, _tag, payloadSize);
	encoder << header
			<< oldTransation;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::remove(Fid fid) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid);               // Fid to remove

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TRemove, _tag, payloadSize);
	encoder << header
			<< fid;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::open(Fid fid, OpenMode mode) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid) +                // Fid of the file to open
			encoder.protocolSize(mode.mode);                // Mode of the file to open

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TOpen, _tag, payloadSize);
	encoder << header
			<< fid
			<< mode.mode;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::create(Fid fid, StringView name, uint32 permissions, OpenMode mode) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid) +
			encoder.protocolSize(name) +
			encoder.protocolSize(permissions) +
			encoder.protocolSize(mode.mode);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TCreate, _tag, payloadSize);
	encoder << header
			<< fid
			<< name
			<< permissions
			<< mode.mode;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::read(Fid fid, uint64 offset, size_type count) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid) +
			encoder.protocolSize(offset) +
			encoder.protocolSize(count);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TRead, _tag, payloadSize);
	encoder << header
			<< fid
			<< offset
			<< count;

    return TypedWriter{_buffer, pos, header};
}


RequestWriter::DataWriter
RequestWriter::write(Fid fid, uint64 offset) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid) +
			encoder.protocolSize(offset) +
			encoder.protocolSize(MemoryView{});

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TWrite, _tag, payloadSize);
	encoder << header
			<< fid
			<< offset;

	return DataWriter{_buffer, pos, header};
}


RequestWriter::PathWriter
RequestWriter::walk(Fid fid, Fid nfid) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid) +
			encoder.protocolSize(nfid) +
			encoder.protocolSize(WalkPath::size_type{0});

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TWalk, _tag, payloadSize);
	encoder << header
			<< fid
			<< nfid;

	return PathWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::stat(Fid fid) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TStat, _tag, payloadSize);
	encoder << header
			<< fid;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::writeStat(Fid fid, Stat const& stat) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(fid) +
			encoder.protocolSize(stat);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TWStat, _tag, payloadSize);
	encoder << header
			<< fid
			<< stat;

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestWriter::session(Solace::ArrayView<byte> key) {
	assertTrue(key.size() == 8);

    // Compute message size first:
    auto const payloadSize =
			8;  // Key size is fixed to be 8 bytes.
//            protocolSize(key);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TSession, _tag, payloadSize);
	Encoder encoder{_buffer};
	encoder << header;

//            << key);
	// Note: the key bytes written directly into the buffer because using encoder to write raw buffer results in
	// in size of the buffer written before the data. In case of Session message - we know key size to be 8 bytes.
	_buffer.write(key.view());

    return TypedWriter{_buffer, pos, header};
}

RequestWriter::PathWriter
RequestWriter::shortRead(Fid rootFid) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(rootFid) +
			encoder.protocolSize(WalkPath::size_type{0});

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TSRead, _tag, payloadSize);
	encoder << header
			<< rootFid;

	return PathWriter{_buffer, pos, header};
}

RequestWriter::PathDataWriter
RequestWriter::shortWrite(Fid rootFid) {
	Encoder encoder{_buffer};

    // Compute message size first:
    auto const payloadSize =
			encoder.protocolSize(rootFid) +
			encoder.protocolSize(WalkPath::size_type{0}) +
			encoder.protocolSize(MemoryView{});

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TSWrite, _tag, payloadSize);
	encoder << header
			<< rootFid;

	return PathDataWriter{_buffer, pos, header};
}

