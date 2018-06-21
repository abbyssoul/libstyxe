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

#include <solace/assert.hpp>
#include <solace/exception.hpp>



using namespace Solace;
using namespace styxe;



const P9Protocol::size_type P9Protocol::MAX_MESSAGE_SIZE = 8*1024;      // 8k should be enough for everyone, am I right?
const StringLiteral         P9Protocol::PROTOCOL_VERSION = "9P2000.e";  // By default we want to talk via 9P2000.e proc
const StringLiteral         P9Protocol::UNKNOWN_PROTOCOL_VERSION = "unknown";
const P9Protocol::Tag       P9Protocol::NO_TAG = static_cast<P9Protocol::Tag>(~0);
const P9Protocol::Fid       P9Protocol::NOFID = static_cast<P9Protocol::Fid>(~0);





Result<P9Protocol::Response, Error>
parseNoDataResponse(const P9Protocol::MessageHeader& header, ReadBuffer& SOLACE_UNUSED(data)) {
    P9Protocol::Response fcall(header.type, header.tag);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Response, Error>
parseErrorResponse(const P9Protocol::MessageHeader& SOLACE_UNUSED(header), ReadBuffer& data) {
    StringView errorMessage;

    P9Protocol::Decoder(data)
            .read(&errorMessage);

    return Err(Error(errorMessage));
}


Result<P9Protocol::Response, Error>
parseVersionResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    P9Protocol::Decoder(data)
            .read(&fcall.version.msize)
            .read(&fcall.version.version);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Response, Error>
parseAuthResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    P9Protocol::Decoder(data)
            .read(&fcall.auth.qid);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Response, Error>
parseAttachResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    P9Protocol::Decoder(data)
            .read(&fcall.attach.qid);

    return Ok(std::move(fcall));
}



Result<P9Protocol::Response, Error>
parseOpenResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    P9Protocol::Decoder(data)
            .read(&fcall.open.qid)
            .read(&fcall.open.iounit);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Response, Error>
parseCreateResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    P9Protocol::Decoder(data)
            .read(&fcall.create.qid)
            .read(&fcall.open.iounit);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Response, Error>
parseReadResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    P9Protocol::Decoder(data)
            .read(&fcall.read.data);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Response, Error>
parseWriteResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    P9Protocol::Decoder(data)
            .read(&fcall.write.count);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Response, Error>
parseStatResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    uint16 dummySize;
    P9Protocol::Decoder(data)
            .read(&dummySize)
            .read(&fcall.stat);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Response, Error>
parseWalkResponse(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Response fcall(header.type, header.tag);

    P9Protocol::Decoder decoder(data);

    // FIXME: Non-sense!
    decoder.read(&fcall.walk.nqids);
    for (decltype(fcall.walk.nqids) i = 0; i < fcall.walk.nqids; ++i) {
        decoder.read(&fcall.walk.qids[i]);
    }

    return Ok(std::move(fcall));
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Request parser
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Result<P9Protocol::Request, Error>
parseVersionRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asVersion();
    P9Protocol::Decoder(data)
            .read(&msg.msize)
            .read(&msg.version);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseAuthRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asAuth();
    P9Protocol::Decoder(data)
            .read(&msg.afid)
            .read(&msg.uname)
            .read(&msg.aname);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseFlushRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asFlush();
    P9Protocol::Decoder(data)
            .read(&msg.oldtag);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseAttachRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asAttach();
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&msg.afid)
            .read(&msg.uname)
            .read(&msg.aname);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseWalkRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asWalk();
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&msg.newfid)
            .read(&msg.path);

    return Ok(std::move(fcall));
}



Result<P9Protocol::Request, Error>
parseOpenRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asOpen();
    byte openMode;
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&openMode);

    msg.mode = static_cast<P9Protocol::OpenMode>(openMode);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseCreateRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asCreate();
    byte openMode;
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&msg.name)
            .read(&msg.perm)
            .read(&openMode);

    msg.mode = static_cast<P9Protocol::OpenMode>(openMode);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseReadRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asRead();
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&msg.offset)
            .read(&msg.count);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseWriteRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asWrite();
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&msg.offset)
            .read(&msg.data);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseClunkRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asClunk();
    P9Protocol::Decoder(data)
            .read(&msg.fid);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseRemoveRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asRemove();
    P9Protocol::Decoder(data)
            .read(&msg.fid);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseStatRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asStat();
    P9Protocol::Decoder(data)
            .read(&msg.fid);

    return Ok(std::move(fcall));
}


Result<P9Protocol::Request, Error>
parseWStatRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asWstat();
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&msg.stat);

    return Ok(std::move(fcall));
}



Result<P9Protocol::Request, Error>
parseSessionRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asSession();
//    P9Protocol::Decoder(data);
//            .read(&fcall.session.key);

    msg.key = data.viewRemaining().slice(0, 8);
    data.advance(8);

    return Ok(std::move(fcall));
}

Result<P9Protocol::Request, Error>
parseShortReadRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asShortRead();
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&msg.path);

    return Ok(std::move(fcall));
}

Result<P9Protocol::Request, Error>
parseShortWriteRequest(const P9Protocol::MessageHeader& header, ReadBuffer& data) {
    P9Protocol::Request fcall(header.type, header.tag);

    auto& msg = fcall.asShortWrite();
    P9Protocol::Decoder(data)
            .read(&msg.fid)
            .read(&msg.path)
            .read(&msg.data);

    return Ok(std::move(fcall));
}



Result<P9Protocol::MessageHeader, Error>
P9Protocol::parseMessageHeader(ReadBuffer& buffer) const {
//    Solace::assertIndexInRange(buffer.viewWritten().size(), 0, headerSize());
    const auto mandatoryHeaderSize = headerSize();
    const auto dataAvailliable = buffer.remaining();
    if (dataAvailliable < mandatoryHeaderSize)
        return Err(Error("Ill-formed message header. Not enough data to read a header"));

    MessageHeader header;
    buffer.readLE(header.size);

    // Sanity checks:
    // It is a serious error if server responded with the message of a size bigger than negotiated one.
    if (header.size < headerSize())
        return Err(Error("Ill-formed message: Declared frame size less than header"));
    if (header.size > maxNegotiatedMessageSize())
        return Err(Error("Ill-formed message: Declared frame size greater than negotiated message size"));

    // Read message type:
    byte messageBytecode;
    buffer.readLE(messageBytecode);
    // don't want any funny messages.
    header.type = static_cast<MessageType>(messageBytecode);
    if (header.type < MessageType::_beginSupportedMessageCode ||
        header.type >= MessageType::_endSupportedMessageCode)
        return Err(Error("Ill-formed message: Unsupported message type"));

    // Read message tag. Tags are provided by the client and can not be checked by the message parser.
    // Unless we are provided with the expected tag...
    buffer.readLE(header.tag);

    return Ok(header);
}


Result<P9Protocol::Response, Error>
P9Protocol::parseResponse(const MessageHeader& header, ReadBuffer& data) const {
    const auto expectedData = header.size - headerSize();

    // Message data sanity check
    // Make sure we have been given enough data to read a message as requested in the message size.
    if (expectedData > data.remaining())
        return Err(Error("Ill-formed message: Declared frame size larger than message data received"));

    // Make sure there is no extra data in the buffer.
    if (expectedData < data.remaining())
        return Err(Error("Ill-formed message: Declared frame size less than message data received"));

    switch (header.type) {
    case MessageType::RError:   return parseErrorResponse(header,   data);
    case MessageType::RVersion: return parseVersionResponse(header, data);
    case MessageType::RAuth:    return parseAuthResponse(header,    data);
    case MessageType::RAttach:  return parseAttachResponse(header,  data);
    case MessageType::RWalk:    return parseWalkResponse(header,    data);
    case MessageType::ROpen:    return parseOpenResponse(header,    data);
    case MessageType::RCreate:  return parseCreateResponse(header,  data);
    case MessageType::RSRead:  // Note: RRead is re-used here for RSRead
    case MessageType::RRead:    return parseReadResponse(header,    data);
    case MessageType::RSWrite:  // Note: RWrite is re-used here for RSWrite
    case MessageType::RWrite:   return parseWriteResponse(header,   data);
    case MessageType::RStat:    return parseStatResponse(header,    data);

    // Responses with no data use common procedure:
    case MessageType::RFlush:
    case MessageType::RClunk:
    case MessageType::RRemove:
    case MessageType::RWStat:
    case MessageType::RSession:
        return parseNoDataResponse(header,   data);

    default:
        return Err(Error("Failed to parse responce message: Unsupported message type"));
    }
}

Result<P9Protocol::Request, Solace::Error>
P9Protocol::parseRequest(const MessageHeader& header, ReadBuffer& data) const {
    // Just paranoid about huge messages exciding frame size getting through.
    if (header.size > maxNegotiatedMessageSize())
        return Err(Error("Ill-formed message: Declared frame size greater than negotiated message size"));

    const auto expectedData = header.size - headerSize();

    // Message data sanity check
    // Make sure we have been given enough data to read a message as requested in the message size.
    if (expectedData > data.remaining())
        return Err(Error("Ill-formed message: Declared frame size larger than message data received"));

    // Make sure there is no extra unexpected data in the buffer.
    if (expectedData < data.remaining())
        return Err(Error("Ill-formed message: Declared frame size less than message data received"));

    switch (header.type) {
    case MessageType::TVersion: return parseVersionRequest(header,      data);
    case MessageType::TAuth:    return parseAuthRequest(header,         data);
    case MessageType::TFlush:   return parseFlushRequest(header,        data);
    case MessageType::TAttach:  return parseAttachRequest(header,       data);
    case MessageType::TWalk:    return parseWalkRequest(header,         data);
    case MessageType::TOpen:    return parseOpenRequest(header,         data);
    case MessageType::TCreate:  return parseCreateRequest(header,       data);
    case MessageType::TRead:    return parseReadRequest(header,         data);
    case MessageType::TWrite:   return parseWriteRequest(header,        data);
    case MessageType::TClunk:   return parseClunkRequest(header,        data);
    case MessageType::TRemove:  return parseRemoveRequest(header,       data);
    case MessageType::TStat:    return parseStatRequest(header,         data);
    case MessageType::TWStat:   return parseWStatRequest(header,        data);
    /* 9P2000.e extension messages */
    case MessageType::TSession: return parseSessionRequest(header,      data);
    case MessageType::TSRead:   return parseShortReadRequest(header,    data);
    case MessageType::TSWrite:  return parseShortWriteRequest(header,   data);

    default:
        return Err(Error("Failed to parse responce message: Unsupported message type"));
    }
}

P9Protocol::size_type P9Protocol::maxNegotiatedMessageSize(size_type newMessageSize) {
    Solace::assertIndexInRange(newMessageSize, 0, maxPossibleMessageSize() + 1);
    _maxNegotiatedMessageSize = std::min(newMessageSize, maxPossibleMessageSize());

    return _maxNegotiatedMessageSize;
}


P9Protocol::P9Protocol(size_type maxMassageSize, const Solace::StringView& version) :
    _maxMassageSize(maxMassageSize),
    _maxNegotiatedMessageSize(maxMassageSize),
    _initialVersion(version),
    _negotiatedVersion(version)
{
    // No-op
}




P9Protocol::Request::Request(MessageType msgType, Tag msgTag) :
    _tag(msgTag),
    _type(msgType)
{
    switch (_type) {
    case MessageType::TVersion: new (&version)  Version;        return;
    case MessageType::TAuth:    new (&auth)     Auth;           return;
    case MessageType::TFlush:   new (&flush)    Flush;          return;
    case MessageType::TAttach:  new (&attach)   Attach;         return;
    case MessageType::TWalk:    new (&walk)     Walk;           return;
    case MessageType::TOpen:    new (&open)     Open;           return;
    case MessageType::TCreate:  new (&create)   Create;         return;
    case MessageType::TRead:    new (&read)     Read;           return;
    case MessageType::TWrite:   new (&write)    Write;          return;
    case MessageType::TClunk:   new (&clunk)    Clunk;          return;
    case MessageType::TRemove:  new (&remove)   Remove;         return;
    case MessageType::TStat:    new (&stat)     StatRequest;    return;
    case MessageType::TWStat:   new (&wstat)    WStat;          return;

    /* 9P2000.e extention */
    case MessageType::TSession: new (&session)      Session;    return;
    case MessageType::TSRead:   new (&shortRead)    SRead;      return;
    case MessageType::TSWrite:  new (&shortWrite)   SWrite;     return;

    default:
        Solace::raise<IOException>("Unexpected message type");
        break;
    }
}

P9Protocol::Request::Request(Request&& rhs) :
    _tag(std::move(rhs._tag)),
    _type(std::move(rhs._type))
{
    switch (_type) {
    case MessageType::TVersion: new (&version)  Version(std::move(rhs.version));    return;
    case MessageType::TAuth:    new (&auth)     Auth(std::move(rhs.auth));       return;
    case MessageType::TFlush:   new (&flush)    Flush(std::move(rhs.flush));      return;
    case MessageType::TAttach:  new (&attach)   Attach(std::move(rhs.attach));     return;
    case MessageType::TWalk:    new (&walk)     Walk(std::move(rhs.walk));       return;
    case MessageType::TOpen:    new (&open)     Open(std::move(rhs.open));       return;
    case MessageType::TCreate:  new (&create)   Create(std::move(rhs.create));     return;
    case MessageType::TRead:    new (&read)     Read(std::move(rhs.read));       return;
    case MessageType::TWrite:   new (&write)    Write(std::move(rhs.write));      return;
    case MessageType::TClunk:   new (&clunk)    Clunk(std::move(rhs.clunk));      return;
    case MessageType::TRemove:  new (&remove)   Remove(std::move(rhs.remove));     return;
    case MessageType::TStat:    new (&stat)     StatRequest(std::move(rhs.stat));       return;
    case MessageType::TWStat:   new (&wstat)    WStat(std::move(rhs.wstat));      return;

    /* 9P2000.e extention */
    case MessageType::TSession: new (&session)      Session(std::move(rhs.session));   return;
    case MessageType::TSRead:   new (&shortRead)    SRead(std::move(rhs.shortRead));  return;
    case MessageType::TSWrite:  new (&shortWrite)   SWrite(std::move(rhs.shortWrite)); return;

    default:
        Solace::raise<IOException>("Unexpected message type");
        break;
    }
}

P9Protocol::Request::~Request() {
    switch (_type) {
    case MessageType::TVersion: (&version)->~Version();     break;
    case MessageType::TAuth:    (&auth)->~Auth();           break;
    case MessageType::TFlush:   (&flush)->~Flush();         break;
    case MessageType::TAttach:  (&attach)->~Attach();       break;
    case MessageType::TWalk:    (&walk)->~Walk();           break;
    case MessageType::TOpen:    (&open)->~Open();           break;
    case MessageType::TCreate:  (&create)->~Create();       break;
    case MessageType::TRead:    (&read)->~Read();           break;
    case MessageType::TWrite:   (&write)->~Write();         break;
    case MessageType::TClunk:   (&clunk)->~Clunk();         break;
    case MessageType::TRemove:  (&remove)->~Remove();       break;
    case MessageType::TStat:    (&stat)->~StatRequest();    break;
    case MessageType::TWStat:   (&wstat)->~WStat();         break;

    /* 9P2000.e extention */
    case MessageType::TSession: (&session)->~Session();     break;
    case MessageType::TSRead:   (&shortRead)->~SRead();     break;
    case MessageType::TSWrite:  (&shortWrite)->~SWrite();   break;

    default:
        Solace::raise<IOException>("Unexpected message type");
        break;
    }
}


P9Protocol::Request::Version&
P9Protocol::Request::asVersion() {
    if (_type != MessageType::TVersion)
        Solace::raise<IOException>("Incorrect message type");

    return version;
}

P9Protocol::Request::Auth&
P9Protocol::Request::asAuth(){
    if (_type != MessageType::TAuth)
        Solace::raise<IOException>("Incorrect message type");

    return auth;
}

P9Protocol::Request::Flush&
P9Protocol::Request::asFlush(){
    if (_type != MessageType::TFlush)
        Solace::raise<IOException>("Incorrect message type");

    return flush;
}

P9Protocol::Request::Attach&
P9Protocol::Request::asAttach(){
    if (_type != MessageType::TAttach)
        Solace::raise<IOException>("Incorrect message type");

    return attach;
}

P9Protocol::Request::Walk&
P9Protocol::Request::asWalk(){
    if (_type != MessageType::TWalk)
        Solace::raise<IOException>("Incorrect message type");

    return walk;
}

P9Protocol::Request::Open&
P9Protocol::Request::asOpen(){
    if (_type != MessageType::TOpen)
        Solace::raise<IOException>("Incorrect message type");

    return open;
}

P9Protocol::Request::Create&
P9Protocol::Request::asCreate(){
    if (_type != MessageType::TCreate)
        Solace::raise<IOException>("Incorrect message type");

    return create;
}

P9Protocol::Request::Read&
P9Protocol::Request::asRead(){
    if (_type != MessageType::TRead)
        Solace::raise<IOException>("Incorrect message type");

    return read;
}

P9Protocol::Request::Write&
P9Protocol::Request::asWrite(){
    if (_type != MessageType::TWrite)
        Solace::raise<IOException>("Incorrect message type");

    return write;
}

P9Protocol::Request::Clunk&
P9Protocol::Request::asClunk(){
    if (_type != MessageType::TClunk)
        Solace::raise<IOException>("Incorrect message type");

    return clunk;
}

P9Protocol::Request::Remove&
P9Protocol::Request::asRemove(){
    if (_type != MessageType::TRemove)
        Solace::raise<IOException>("Incorrect message type");

    return remove;
}

P9Protocol::Request::StatRequest&
P9Protocol::Request::asStat(){
    if (_type != MessageType::TStat)
        Solace::raise<IOException>("Incorrect message type");

    return stat;
}

P9Protocol::Request::WStat&
P9Protocol::Request::asWstat(){
    if (_type != MessageType::TWStat)
        Solace::raise<IOException>("Incorrect message type");

    return wstat;
}

P9Protocol::Request::Session&
P9Protocol::Request::asSession(){
    if (_type != MessageType::TSession)
        Solace::raise<IOException>("Incorrect message type");

    return session;
}

P9Protocol::Request::SRead&
P9Protocol::Request::asShortRead(){
    if (_type != MessageType::TSRead)
        Solace::raise<IOException>("Incorrect message type");

    return shortRead;
}

P9Protocol::Request::SWrite&
P9Protocol::Request::asShortWrite(){
    if (_type != MessageType::TSWrite)
        Solace::raise<IOException>("Incorrect message type");

    return shortWrite;
}




P9Protocol::Response::Response(MessageType msgType, Tag msgTag) :
    type(msgType),
    tag(msgTag)
{
    switch (type) {
    case MessageType::RError:   new (&error) Error;     return;
    case MessageType::RVersion: new (&version) Version; return;
    case MessageType::RAuth:    new (&auth) Auth;       return;
    case MessageType::RAttach:  new (&attach) Attach;   return;
    case MessageType::RWalk:    new (&walk) Walk;       return;
    case MessageType::ROpen:    new (&open) Open;       return;
    case MessageType::RCreate:  new (&create) Create;   return;
    case MessageType::RSRead:  // Note: RRead is re-used here for RSRead
    case MessageType::RRead:    new (&read) Read;       return;
    case MessageType::RSWrite:  // Note: RWrite is re-used here for RSWrite
    case MessageType::RWrite:   new (&write) Write;     return;
    case MessageType::RStat:    new (&stat) Stat;       return;
    case MessageType::RClunk:
    case MessageType::RRemove:
    case MessageType::RFlush:
    case MessageType::RWStat:
    case MessageType::RSession:
        break;

    default:
        Solace::raise<IOException>("Unexpected message type");
        break;
    }
}

P9Protocol::Response::Response(Response&& rhs) :
    type(rhs.type),
    tag(rhs.tag)
{
    switch (type) {
    case MessageType::RError:   new (&error) Error(std::move(rhs.error)); return;
    case MessageType::RVersion: new (&version) Version(std::move(rhs.version)); return;
    case MessageType::RAuth:    new (&auth) Auth(std::move(rhs.auth)); return;
    case MessageType::RAttach:  new (&attach) Attach(std::move(rhs.attach)); return;
    case MessageType::RWalk:    new (&walk) Walk(std::move(rhs.walk)); return;
    case MessageType::ROpen:    new (&open) Open(std::move(rhs.open)); return;
    case MessageType::RCreate:  new (&create) Create(std::move(rhs.create)); return;
    case MessageType::RSRead:  // Note: RRead is re-used here for RSRead
    case MessageType::RRead:    new (&read) Read(std::move(rhs.read)); return;
    case MessageType::RSWrite:  // Note: RWrite is re-used here for RSWrite
    case MessageType::RWrite:   new (&write) Write(std::move(rhs.write)); return;
    case MessageType::RStat:    new (&stat) Stat(std::move(rhs.stat)); return;
    case MessageType::RClunk:
    case MessageType::RRemove:
    case MessageType::RFlush:
    case MessageType::RWStat:
    case MessageType::RSession:
        break;

    default:
        Solace::raise<IOException>("Unexpected message type");
        break;
    }
}


P9Protocol::Response::~Response() {
    switch (type) {
    case MessageType::RError:   (&error)->~Error();     break;
    case MessageType::RVersion: (&version)->~Version(); break;
    case MessageType::RAuth:    (&auth)->~Auth();       break;
    case MessageType::RAttach:  (&attach)->~Attach();   break;
    case MessageType::RWalk:    (&walk)->~Walk();       break;
    case MessageType::ROpen:    (&open)->~Open();       break;
    case MessageType::RCreate:  (&create)->~Create();   break;
    case MessageType::RSRead:  // Note: RRead is re-used here for RSRead
    case MessageType::RRead:    (&read)->~Read();       break;
    case MessageType::RSWrite:  // Note: RWrite is re-used here for RSWrite
    case MessageType::RWrite:   (&write)->~Write();     break;
    case MessageType::RStat:    (&stat)->~Stat();       break;
    case MessageType::RClunk:
    case MessageType::RRemove:
    case MessageType::RFlush:
    case MessageType::RWStat:
    case MessageType::RSession:
    default:
        break;
    }
}
