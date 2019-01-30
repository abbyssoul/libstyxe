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


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::updatePayloadSize(size_type payloadSize) {
    _payloadSize = payloadSize;
    _buffer.reset(_initialPosition);

    Encoder(_buffer)
            .header(type(), _tag, _payloadSize);

    _buffer.advance(_payloadSize);

    return (*this);
}



Protocol::ResponseBuilder&
Protocol::ResponseBuilder::updatePayloadSize() {
    const auto dataLoad = _buffer.position() - _initialPosition;
    if (dataLoad < headerSize()) {
        Solace::raise<IOException>("Message header has not been written.");
    }

    const auto newPayloadSize = dataLoad - headerSize();

    return updatePayloadSize(newPayloadSize);
}


ByteWriter&
Protocol::ResponseBuilder::build(bool recalcPayloadSize) {
    if (type() < MessageType::_beginSupportedMessageCode ||
        type() > MessageType::_endSupportedMessageCode) {
        Solace::raise<IOException>("Unexpected message type");
    }

    if (recalcPayloadSize) {
        updatePayloadSize(_buffer.position() - headerSize());
    }

    return _buffer.flip();
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::version(StringView version, size_type maxMessageSize) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RVersion;
    _payloadSize =
            encode.protocolSize(maxMessageSize) +
            encode.protocolSize(version);

    _tag = NO_TAG;
    encode.header(type(), NO_TAG, _payloadSize)
            .encode(maxMessageSize)
            .encode(version);

    return (*this);
}

Protocol::ResponseBuilder&
Protocol::ResponseBuilder::auth(const Qid& qid) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RAuth;
    _payloadSize =
            encode.protocolSize(qid);

    encode.header(type(), _tag, _payloadSize)
            .encode(qid);

    return (*this);
}

Protocol::ResponseBuilder&
Protocol::ResponseBuilder::error(StringView message) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RError;
    _payloadSize =
            encode.protocolSize(message);

    encode.header(type(), _tag, _payloadSize)
            .encode(message);

    return (*this);
}

Protocol::ResponseBuilder&
Protocol::ResponseBuilder::flush() {
    buffer().reset(_initialPosition);
    _type = MessageType::RFlush;
    _payloadSize = 0;

    Encoder(buffer())
            .header(type(), _tag);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::attach(const Qid& qid) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RAttach;
    _payloadSize =
            encode.protocolSize(qid);

    encode.header(type(), _tag, _payloadSize)
            .encode(qid);

    return (*this);
}

Protocol::ResponseBuilder&
Protocol::ResponseBuilder::walk(Solace::ArrayView<Qid> const& qids) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    _type = MessageType::RWalk;
    // Compute message size first:
    _payloadSize =
            encode.protocolSize(qids);

    encode.header(type(), _tag, _payloadSize)
            .encode(qids);

    return (*this);
}

Protocol::ResponseBuilder&
Protocol::ResponseBuilder::open(const Qid& qid, size_type iounit) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    _type = MessageType::ROpen;
    // Compute message size first:
    _payloadSize =
            encode.protocolSize(qid) +
            encode.protocolSize(iounit);

    encode.header(type(), _tag, _payloadSize)
            .encode(qid)
            .encode(iounit);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::create(const Qid& qid, size_type iounit) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RCreate;
    _payloadSize =
            encode.protocolSize(qid) +
            encode.protocolSize(iounit);

    encode.header(type(), _tag, _payloadSize)
            .encode(qid)
            .encode(iounit);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::read(const MemoryView& data) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RRead;
    _payloadSize =
            encode.protocolSize(data);

    encode.header(type(), _tag, _payloadSize)
            .encode(data);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::write(size_type count) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RWrite;
    _payloadSize =
            encode.protocolSize(count);

    encode.header(type(), _tag, _payloadSize)
            .encode(count);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::clunk() {
    buffer().reset(_initialPosition);
    _type = MessageType::RClunk;
    _payloadSize = 0;

    Encoder(buffer())
            .header(type(), _tag);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::remove() {
    buffer().reset(_initialPosition);
    _type = MessageType::RRemove;
    _payloadSize = 0;

    Encoder(buffer())
            .header(type(), _tag);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::stat(const Stat& data) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RStat;
    _payloadSize =
            sizeof(uint16) +
            encode.protocolSize(data);

    encode.header(type(), _tag, _payloadSize)
            .encode(static_cast<uint16>(_payloadSize))
            .encode(data);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::wstat() {
    buffer().reset(_initialPosition);
    _type = MessageType::RWStat;
    _payloadSize = 0;

    Encoder(buffer())
            .header(type(), _tag);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::session() {
    buffer().reset(_initialPosition);
    _type = MessageType::RSession;
    _payloadSize = 0;

    Encoder(buffer())
            .header(type(), _tag);

    return (*this);
}



Protocol::ResponseBuilder&
Protocol::ResponseBuilder::shortRead(const MemoryView& data) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RSRead;
    _payloadSize =
            encode.protocolSize(data);

    encode.header(type(), _tag, _payloadSize)
            .encode(data);

    return (*this);
}


Protocol::ResponseBuilder&
Protocol::ResponseBuilder::shortWrite(size_type count) {
    buffer().reset(_initialPosition);
    Encoder encode(buffer());

    // Compute message size first:
    _type = MessageType::RSWrite;
    _payloadSize =
            encode.protocolSize(count);

    encode.header(type(), _tag, _payloadSize)
            .encode(count);

    return (*this);
}
