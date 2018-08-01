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


ByteWriter&
Protocol::RequestBuilder::build() {
    if (type() < MessageType::_beginSupportedMessageCode ||
        type() > MessageType::_endSupportedMessageCode) {
        Solace::raise<IOException>("Unexpected message type");
    }

    return _buffer.flip();
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::version(StringView version, size_type maxMessageSize) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(maxMessageSize) +                // Negotiated message size field
            encode.protocolSize(version);   // Version string data

    // Note: we ignore what ever tag value been passed and use NO_TAG value as per protocol.
    _tag = NO_TAG;
    _type = MessageType::TVersion;
    encode.header(type(), NO_TAG, _payloadSize)
            .encode(maxMessageSize)
            .encode(version);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::auth(Fid afid, StringView userName, StringView attachName) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(afid) +                  // Proposed fid for authentication mechanism
            encode.protocolSize(userName) +       // User name
            encode.protocolSize(attachName);     // Root name where we want to attach to

    _type = MessageType::TAuth;
    encode.header(type(), _tag, _payloadSize)
            .encode(afid)
            .encode(userName)
            .encode(attachName);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::attach(Fid fid, Fid afid, StringView userName, StringView attachName) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid) +                  // Proposed fid for the attached root
            encode.protocolSize(afid) +                  // Fid of the passed authentication
            encode.protocolSize(userName) +      // User name
            encode.protocolSize(attachName);     // Root name where we want to attach to

    _type = MessageType::TAttach;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid)
            .encode(afid)
            .encode(userName)
            .encode(attachName);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::clunk(Fid fid) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid);               // Fid to forget

    _type = MessageType::TClunk;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::flush(Tag oldTransation) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(oldTransation);               // Transaction to forget

    _type = MessageType::TFlush;
    encode.header(type(), _tag, _payloadSize)
            .encode(oldTransation);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::remove(Fid fid) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid);               // Fid to remove

    _type = MessageType::TRemove;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::open(Fid fid, Protocol::OpenMode mode) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid) +                // Fid of the file to open
            encode.protocolSize(static_cast<byte>(mode));                // Mode of the file to open

    _type = MessageType::TOpen;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid)
            .encode(static_cast<byte>(mode));

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::create(Fid fid, StringView name, uint32 permissions, Protocol::OpenMode mode) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(name) +
            encode.protocolSize(permissions) +
            encode.protocolSize(static_cast<byte>(mode));

    _type = MessageType::TCreate;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid)
            .encode(name)
            .encode(permissions)
            .encode(static_cast<byte>(mode));

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::read(Fid fid, uint64 offset, size_type count) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(offset) +
            encode.protocolSize(count);

    _type = MessageType::TRead;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid)
            .encode(offset)
            .encode(count);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::write(Fid fid, uint64 offset, MemoryView data) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(offset) +
            encode.protocolSize(data);

    _type = MessageType::TWrite;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid)
            .encode(offset)
            .encode(data);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::walk(Fid fid, Fid nfid, Path const& path) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(nfid) +
            encode.protocolSize(path);

    _type = MessageType::TWalk;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid)
            .encode(nfid)
            .encode(path);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::stat(Fid fid) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid);

    _type = MessageType::TStat;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::writeStat(Fid fid, Stat const& stat) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(fid) +
            encode.protocolSize(stat);

    _type = MessageType::TWStat;
    encode.header(type(), _tag, _payloadSize)
            .encode(fid)
            .encode(stat);

    return (*this);
}


Protocol::RequestBuilder&
Protocol::RequestBuilder::session(MemoryView key) {
    assertIndexInRange(key.size(), 8, 9);

    // Compute message size first:
    _payloadSize =
            8;  // Key size is fixed to be 8 bites.
//            protocolSize(key);

    _type = MessageType::TSession;
    Encoder(buffer())
            .header(type(), _tag, _payloadSize);
//            .encode(key);

    buffer().write(key, 8);

    return (*this);
}

Protocol::RequestBuilder&
Protocol::RequestBuilder::shortRead(Fid rootFid, Path const& path) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(rootFid) +
            encode.protocolSize(path);

    _type = MessageType::TSRead;
    encode.header(type(), _tag, _payloadSize)
            .encode(rootFid)
            .encode(path);

    return (*this);
}

Protocol::RequestBuilder&
Protocol::RequestBuilder::shortWrite(Fid rootFid, Path const& path, MemoryView data) {
    Encoder encode(buffer());

    // Compute message size first:
    _payloadSize =
            encode.protocolSize(rootFid) +
            encode.protocolSize(path) +
            encode.protocolSize(data);

    _type = MessageType::TSWrite;
    encode.header(type(), _tag, _payloadSize)
            .encode(rootFid)
            .encode(path)
            .encode(data);

    return (*this);
}

