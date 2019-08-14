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

TypedWriter
RequestBuilder::DataWriter::data(MemoryView data) {
	Encoder{buffer()}.encode(data);

	return *this;
}


RequestBuilder::PathWriter::PathWriter(ByteWriter& writer,
									   ByteWriter::size_type pos,
									   MessageHeader head) noexcept
	: TypedWriter{writer, pos, head}
	, _segmentsPos{writer.position()}
{
	Encoder{writer}
		.encode(_nSegments);
}

RequestBuilder::PathWriter&
RequestBuilder::PathWriter::path(StringView pathSegment) {
	_nSegments += 1;

	auto& writer = buffer();
	Encoder encoder(writer);

	auto const currentPos = writer.position();
	writer.position(_segmentsPos);
	encoder.encode(_nSegments);
	writer.position(currentPos);

	encoder.encode(pathSegment);

	return *this;
}

RequestBuilder::PathDataWriter::PathDataWriter(ByteWriter& writer,
											   ByteWriter::size_type pos,
											   MessageHeader head) noexcept
	: DataWriter{writer, pos, head}
	, _segmentsPos{writer.position()}
{
	Encoder{writer}
		.encode(_nSegments);
}


RequestBuilder::PathDataWriter&
RequestBuilder::PathDataWriter::path(StringView pathSegment) {
	_nSegments += 1;

	auto& writer = buffer();
	Encoder encoder(writer);

	auto const currentPos = writer.position();
	writer.position(_segmentsPos);
	encoder.encode(_nSegments);
	writer.position(currentPos);

	encoder.encode(pathSegment);

	return *this;
}




TypedWriter
RequestBuilder::version(StringView version, size_type maxMessageSize) const {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(maxMessageSize) +                // Negotiated message size field
            encode.protocolSize(version);   // Version string data

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TVersion, Parser::NO_TAG, payloadSize);
    encode.encode(header)
            .encode(maxMessageSize)
            .encode(version);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::auth(Fid afid, StringView userName, StringView attachName) const {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(afid) +                  // Proposed fid for authentication mechanism
            encode.protocolSize(userName) +       // User name
            encode.protocolSize(attachName);     // Root name where we want to attach to

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TAuth, _tag, payloadSize);
    encode.encode(header)
            .encode(afid)
            .encode(userName)
            .encode(attachName);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::attach(Fid fid, Fid afid, StringView userName, StringView attachName) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid) +                  // Proposed fid for the attached root
            encode.protocolSize(afid) +                  // Fid of the passed authentication
            encode.protocolSize(userName) +      // User name
            encode.protocolSize(attachName);     // Root name where we want to attach to

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TAttach, _tag, payloadSize);
    encode.encode(header)
            .encode(fid)
            .encode(afid)
            .encode(userName)
            .encode(attachName);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::clunk(Fid fid) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid);               // Fid to forget

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TClunk, _tag, payloadSize);
    encode.encode(header)
            .encode(fid);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::flush(Tag oldTransation) const {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(oldTransation);               // Transaction to forget

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TFlush, _tag, payloadSize);
    encode.encode(header)
            .encode(oldTransation);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::remove(Fid fid) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid);               // Fid to remove

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TRemove, _tag, payloadSize);
    encode.encode(header)
            .encode(fid);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::open(Fid fid, OpenMode mode) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid) +                // Fid of the file to open
            encode.protocolSize(mode.mode);                // Mode of the file to open

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TOpen, _tag, payloadSize);
    encode.encode(header)
            .encode(fid)
            .encode(mode.mode);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::create(Fid fid, StringView name, uint32 permissions, OpenMode mode) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(name) +
            encode.protocolSize(permissions) +
            encode.protocolSize(mode.mode);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TCreate, _tag, payloadSize);
    encode.encode(header)
            .encode(fid)
            .encode(name)
            .encode(permissions)
            .encode(mode.mode);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::read(Fid fid, uint64 offset, size_type count) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(offset) +
            encode.protocolSize(count);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TRead, _tag, payloadSize);
    encode.encode(header)
            .encode(fid)
            .encode(offset)
            .encode(count);

    return TypedWriter{_buffer, pos, header};
}


RequestBuilder::DataWriter
RequestBuilder::write(Fid fid, uint64 offset) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(offset) +
			encode.protocolSize(MemoryView{});

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TWrite, _tag, payloadSize);
    encode.encode(header)
            .encode(fid)
			.encode(offset);
//            .encode(data);

	return DataWriter{_buffer, pos, header};
}


RequestBuilder::PathWriter
RequestBuilder::walk(Fid fid, Fid nfid) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(nfid) +
			encode.protocolSize(WalkPath::size_type{0});

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TWalk, _tag, payloadSize);
    encode.encode(header)
            .encode(fid)
			.encode(nfid);

	return PathWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::stat(Fid fid) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TStat, _tag, payloadSize);
    encode.encode(header)
            .encode(fid);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::writeStat(Fid fid, Stat const& stat) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(stat);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TWStat, _tag, payloadSize);
    encode.encode(header)
            .encode(fid)
            .encode(stat);

    return TypedWriter{_buffer, pos, header};
}


TypedWriter
RequestBuilder::session(Solace::ArrayView<byte> key) {
	assertTrue(key.size() == 8);

    // Compute message size first:
    auto const payloadSize =
			8;  // Key size is fixed to be 8 bytes.
//            protocolSize(key);

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TSession, _tag, payloadSize);
    Encoder(_buffer)
            .encode(header);
//            .encode(key);

	_buffer.write(key.view());

    return TypedWriter{_buffer, pos, header};
}

RequestBuilder::PathWriter
RequestBuilder::shortRead(Fid rootFid) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(rootFid) +
			encode.protocolSize(WalkPath::size_type{0});

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TSRead, _tag, payloadSize);
    encode.encode(header)
			.encode(rootFid);

	return PathWriter{_buffer, pos, header};
}

RequestBuilder::PathDataWriter
RequestBuilder::shortWrite(Fid rootFid) {
    Encoder encode(_buffer);

    // Compute message size first:
    auto const payloadSize =
            encode.protocolSize(rootFid) +
			encode.protocolSize(WalkPath::size_type{0}) +
			encode.protocolSize(MemoryView{});

    auto const pos = _buffer.position();
    auto header = makeHeaderWithPayload(MessageType::TSWrite, _tag, payloadSize);
    encode.encode(header)
			.encode(rootFid);
//            .encode(path)
//            .encode(data);

	return PathDataWriter{_buffer, pos, header};
}

