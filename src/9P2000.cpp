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
#include "styxe/version.hpp"

#include <solace/assert.hpp>
#include <algorithm>  // std::min


using namespace Solace;
using namespace styxe;


static const Version   kLibVersion{STYXE_VERSION_MAJOR, STYXE_VERSION_MINOR, STYXE_VERSION_BUILD};

const size_type         styxe::kMaxMesssageSize = 8*1024;      // 8k should be enough for everyone, am I right?

const StringLiteral     Protocol::PROTOCOL_VERSION = "9P2000.e";  // By default we want to talk via 9P2000.e proc
const StringLiteral     Protocol::UNKNOWN_PROTOCOL_VERSION = "unknown";
const Tag               Protocol::NO_TAG = static_cast<Tag>(~0);
const Fid               Protocol::NOFID = static_cast<Fid>(~0);


AtomValue const
styxe::kProtocolErrorCatergory = atom("9p2000");


#define CANNE(id, msg) \
    Error(kProtocolErrorCatergory, static_cast<uint16>(id), msg)


static Error const kCannedErrors[] = {
    CANNE(CannedError::IllFormedHeader, "Ill-formed message header. Not enough data to read a header"),
    CANNE(CannedError::IllFormedHeader_FrameTooShort, "Ill-formed message: Declared frame size less than header"),
    CANNE(CannedError::IllFormedHeader_TooBig, "Ill-formed message: Declared frame size greater than negotiated one"),
    CANNE(CannedError::UnsupportedMessageType, "Ill-formed message: Unsupported message type"),

    CANNE(CannedError::NotEnoughData, "Ill-formed message: Declared frame size larger than message data received"),
    CANNE(CannedError::MoreThenExpectedData, "Ill-formed message: Declared frame size less than message data received"),
};


Error
styxe::getCannedError(CannedError errorId) noexcept {
    return kCannedErrors[static_cast<int>(errorId)];
}

Version const& styxe::getVersion() noexcept {
    return kLibVersion;
}


struct OkRespose {
    Result<ResponseMessage, Error>
    operator() () { return Result<ResponseMessage, Error>(types::OkTag{}, std::move(fcall)); }

    ResponseMessage fcall;

    template<typename T>
    OkRespose(T&& f)
        : fcall{std::forward<T>(f)}
    {}
};

struct OkRequest {
    Result<RequestMessage, Error>
    operator() () { return Result<RequestMessage, Error>(types::OkTag{}, std::move(fcall)); }

    RequestMessage fcall;

    template<typename T>
    OkRequest(T&& f)
        : fcall{std::forward<T>(f)}
    {}
};


template<typename T>
Result<ResponseMessage, Error>
parseNoDataResponse() {
    return Result<ResponseMessage, Error>(types::OkTag{}, T{});
}


Result<ResponseMessage, Error>
parseErrorResponse(ByteReader& data) {
    Response::Error fcall;

    return Decoder{data}
            .read(&fcall.ename)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseVersionResponse(ByteReader& data) {
    Response::Version fcall;

    return Decoder{data}
            .read(&fcall.msize, &fcall.version)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseAuthResponse(ByteReader& data) {
    Response::Auth fcall;

    return Decoder{data}
            .read(&fcall.qid)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseAttachResponse(ByteReader& data) {
    Response::Attach fcall;

    return Decoder{data}
            .read(&fcall.qid)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseOpenResponse(ByteReader& data) {
    Response::Open fcall;

    return Decoder{data}
            .read(&fcall.qid, &fcall.iounit)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseCreateResponse(ByteReader& data) {
    Response::Create fcall;

    return Decoder{data}
            .read(&fcall.qid, &fcall.iounit)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseReadResponse(ByteReader& data) {
    Response::Read fcall;

    return Decoder{data}
            .read(&fcall.data)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseWriteResponse(ByteReader& data) {
    Response::Write fcall;

    return Decoder{data}
            .read(&fcall.count)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseStatResponse(ByteReader& data) {
    Response::Stat fcall;

    return Decoder{data}
            .read(&fcall.dummySize, &fcall.data)
            .then(OkRespose(std::move(fcall)));
}


Result<ResponseMessage, Error>
parseWalkResponse(ByteReader& data) {
    Response::Walk fcall;

    Decoder decoder{data};

    // FIXME: Non-sense!
    return decoder.read(&fcall.nqids)
            .then([&decoder, &fcall]() -> Result<void, Error> {
                for (decltype(fcall.nqids) i = 0; i < fcall.nqids; ++i) {
                    auto r = decoder.read(&fcall.qids[i]);
                    if (!r) {
                        return Err<Error>(r.moveError());
                    }
                }

                return Ok();
            })
            .then(OkRespose(std::move(fcall)));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Request parser
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Result<RequestMessage, Error>
parseVersionRequest(ByteReader& data) {
    auto msg = Request::Version{};
    return Decoder{data}
            .read(&msg.msize, &msg.version)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseAuthRequest(ByteReader& data) {
    auto msg = Request::Auth{};
    return Decoder{data}
            .read(&msg.afid, &msg.uname, &msg.aname)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseFlushRequest(ByteReader& data) {
    auto msg = Request::Flush{};
    return Decoder{data}
            .read(&msg.oldtag)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseAttachRequest(ByteReader& data) {
    auto msg = Request::Attach{};
    return Decoder{data}
            .read(&msg.fid, &msg.afid, &msg.uname, &msg.aname)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseWalkRequest(ByteReader& data) {
    auto msg = Request::Walk{};
    return Decoder{data}
            .read(&msg.fid, &msg.newfid, &msg.path)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseOpenRequest(ByteReader& data) {
    byte openMode;

    auto msg = Request::Open{};
    return Decoder{data}
            .read(&msg.fid, &openMode)
            .then([&msg, &openMode]() { msg.mode = static_cast<OpenMode>(openMode); })
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseCreateRequest(ByteReader& data) {
    byte openMode;

    auto msg = Request::Create{};
    return Decoder{data}
            .read(&msg.fid, &msg.name, &msg.perm, &openMode)
            .then([&msg, &openMode]() { msg.mode = static_cast<OpenMode>(openMode); })
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseReadRequest(ByteReader& data) {
    auto msg = Request::Read{};
    return Decoder{data}
            .read(&msg.fid, &msg.offset, &msg.count)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseWriteRequest(ByteReader& data) {
    auto msg = Request::Write{};
    return Decoder{data}
            .read(&msg.fid, &msg.offset, &msg.data)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseClunkRequest(ByteReader& data) {
    auto msg = Request::Clunk{};
    return Decoder{data}
            .read(&msg.fid)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseRemoveRequest(ByteReader& data) {
    auto msg = Request::Remove{};
    return Decoder{data}
            .read(&msg.fid)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseStatRequest(ByteReader& data) {
    auto msg = Request::StatRequest{};
    return Decoder{data}
            .read(&msg.fid)
            .then(OkRequest(std::move(msg)));
}


Result<RequestMessage, Error>
parseWStatRequest(ByteReader& data) {
    auto msg = Request::WStat{};
    return Decoder{data}
            .read(&msg.fid, &msg.stat)
            .then(OkRequest(std::move(msg)));
}



Result<RequestMessage, Error>
parseSessionRequest(ByteReader& data) {
    auto msg = Request::Session{};
    return Decoder{data}
            .read(&(msg.key[0]), &(msg.key[1]), &(msg.key[2]), &(msg.key[3]),
                  &(msg.key[4]), &(msg.key[5]), &(msg.key[6]), &(msg.key[7]))
            .then(OkRequest(std::move(msg)));
}

Result<RequestMessage, Error>
parseShortReadRequest(ByteReader& data) {
    auto msg = Request::SRead{};
    return Decoder{data}
            .read(&msg.fid, &msg.path)
            .then(OkRequest(std::move(msg)));
}

Result<RequestMessage, Error>
parseShortWriteRequest(ByteReader& data) {
    auto msg = Request::SWrite{};
    return Decoder{data}
            .read(&msg.fid, &msg.path, &msg.data)
            .then(OkRequest(std::move(msg)));
}



Result<MessageHeader, Error>
Protocol::parseMessageHeader(ByteReader& src) const {
    auto const mandatoryHeaderSize = headerSize();
    auto const dataAvailable = src.remaining();

    // Check that we have enough data to read mandatory message header
    if (dataAvailable < mandatoryHeaderSize) {
        return Err(getCannedError(CannedError::IllFormedHeader));
    }

    Decoder decoder{src};
    MessageHeader header;

    decoder.read(&header.messageSize);

    // Sanity checks:
    if (header.messageSize < mandatoryHeaderSize) {
        return Err(getCannedError(CannedError::IllFormedHeader_FrameTooShort));
    }

    // It is a serious error if we got a message of a size bigger than negotiated one.
    if (header.messageSize > maxNegotiatedMessageSize()) {
        return Err(getCannedError(CannedError::IllFormedHeader_TooBig));
    }

    // Read message type:
    byte messageBytecode;
    decoder.read(&messageBytecode);
    // don't want any funny messages.
    header.type = static_cast<MessageType>(messageBytecode);
    if (header.type < MessageType::_beginSupportedMessageCode ||
        header.type >= MessageType::_endSupportedMessageCode) {
        return Err(getCannedError(CannedError::UnsupportedMessageType));
    }

    // Read message tag. Tags are provided by the client and can not be checked by the message parser.
    // Unless we are provided with the expected tag...
    decoder.read(&header.tag);

    return Ok(header);
}


Result<ResponseMessage, Error>
Protocol::parseResponse(MessageHeader const& header, ByteReader& data) const {
    auto const expectedData = header.payloadSize();

    // Message data sanity check
    // Just paranoid about huge messages exciding frame size getting through.
    if (header.messageSize > maxNegotiatedMessageSize()) {
        return Err(getCannedError(CannedError::IllFormedHeader_TooBig));
    }

    // Make sure we have been given enough data to read a message as requested in the message size.
    if (expectedData > data.remaining()) {
        return Err(getCannedError(CannedError::NotEnoughData));
    }

    // Make sure there is no extra data in the buffer.
    if (expectedData < data.remaining()) {
        return Err(getCannedError(CannedError::MoreThenExpectedData));
    }

    switch (header.type) {
    case MessageType::RError:   return parseErrorResponse(data);
    case MessageType::RVersion: return parseVersionResponse(data);
    case MessageType::RAuth:    return parseAuthResponse(data);
    case MessageType::RAttach:  return parseAttachResponse(data);
    case MessageType::RWalk:    return parseWalkResponse(data);
    case MessageType::ROpen:    return parseOpenResponse(data);
    case MessageType::RCreate:  return parseCreateResponse(data);
    case MessageType::RSRead:  // Note: RRead is re-used here for RSRead
    case MessageType::RRead:    return parseReadResponse(data);
    case MessageType::RSWrite:  // Note: RWrite is re-used here for RSWrite
    case MessageType::RWrite:   return parseWriteResponse(data);
    case MessageType::RStat:    return parseStatResponse(data);

    // Responses with no data use common procedure:
    case MessageType::RFlush:   return parseNoDataResponse<Response::Flush>();
    case MessageType::RClunk:   return parseNoDataResponse<Response::Clunk>();
    case MessageType::RRemove:  return parseNoDataResponse<Response::Remove>();
    case MessageType::RWStat:   return parseNoDataResponse<Response::WStat>();
    case MessageType::RSession: return parseNoDataResponse<Response::Session>();


    default:
        return Err(getCannedError(CannedError::UnsupportedMessageType));
    }
}

Result<RequestMessage, Solace::Error>
Protocol::parseRequest(MessageHeader const& header, ByteReader& data) const {
    const auto expectedData = header.payloadSize();

    // Message data sanity check
    // Just paranoid about huge messages exciding frame size getting through.
    if (header.messageSize > maxNegotiatedMessageSize()) {
        return Err(getCannedError(CannedError::IllFormedHeader_TooBig));
    }

    // Make sure we have been given enough data to read a message as requested in the message size.
    if (expectedData > data.remaining()) {
        return Err(getCannedError(CannedError::NotEnoughData));
    }

    // Make sure there is no extra unexpected data in the buffer.
    if (expectedData < data.remaining()) {
        return Err(getCannedError(CannedError::MoreThenExpectedData));
    }

    switch (header.type) {
    case MessageType::TVersion: return parseVersionRequest(data);
    case MessageType::TAuth:    return parseAuthRequest(data);
    case MessageType::TFlush:   return parseFlushRequest(data);
    case MessageType::TAttach:  return parseAttachRequest(data);
    case MessageType::TWalk:    return parseWalkRequest(data);
    case MessageType::TOpen:    return parseOpenRequest(data);
    case MessageType::TCreate:  return parseCreateRequest(data);
    case MessageType::TRead:    return parseReadRequest(data);
    case MessageType::TWrite:   return parseWriteRequest(data);
    case MessageType::TClunk:   return parseClunkRequest(data);
    case MessageType::TRemove:  return parseRemoveRequest(data);
    case MessageType::TStat:    return parseStatRequest(data);
    case MessageType::TWStat:   return parseWStatRequest(data);
    /* 9P2000.e extension messages */
    case MessageType::TSession: return parseSessionRequest(data);
    case MessageType::TSRead:   return parseShortReadRequest(data);
    case MessageType::TSWrite:  return parseShortWriteRequest(data);

    default:
        return Err(getCannedError(CannedError::UnsupportedMessageType));
    }
}

size_type
Protocol::maxNegotiatedMessageSize(size_type newMessageSize) {
    Solace::assertIndexInRange(newMessageSize, 0, maxPossibleMessageSize() + 1);
    _maxNegotiatedMessageSize = std::min(newMessageSize, maxPossibleMessageSize());

    return _maxNegotiatedMessageSize;
}


Protocol::Protocol(size_type maxMassageSize, StringView version) :
    _maxMassageSize(maxMassageSize),
    _maxNegotiatedMessageSize(maxMassageSize),
    _initialVersion(version),
    _negotiatedVersion(makeString(version))
{
    // No-op
}



ByteWriter&
TypedWriter::build() {
    auto const finalPos = _buffer.position();
    auto const messageSize = finalPos - _pos; // Compute actual message size
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
