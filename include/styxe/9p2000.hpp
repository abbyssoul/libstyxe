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
#pragma once
#ifndef STYXE_9P2000_HPP
#define STYXE_9P2000_HPP

#include <solace/stringView.hpp>
#include <solace/string.hpp>
#include <solace/byteReader.hpp>
#include <solace/byteWriter.hpp>

#include <solace/result.hpp>
#include <solace/error.hpp>
#include <solace/path.hpp>


#include <variant>


namespace styxe {


/** Error category for protocol specific errors */
extern Solace::AtomValue const kProtocolErrorCatergory;


/**
 * Enum class for protocol error codes.
 */
enum class CannedError : int {
    IllFormedHeader = 0,
    IllFormedHeader_FrameTooShort,
    IllFormedHeader_TooBig,
    UnsupportedMessageType,
    NotEnoughData,
    MoreThenExpectedData,
};

/**
 * Get canned error from the protocol error code.
 * @param errorId Error code of the canned error.
 * @return Error object for the error category
 */
Solace::Error getCannedError(CannedError errorId) noexcept;


/** Network protocol uses fixed width int32 to represent size of data in bytes */
using size_type = Solace::uint32;

/** Network protocol uses fixed width int16 to represent variable datum size */
using var_datum_size_type = Solace::uint16;

/** Type of message Tag */
using Tag = Solace::uint16;

/** Type of file identifiers client uses to identify a ``current file`` on the server*/
using Fid = Solace::uint32;


/** Compile time constants (probably should be removed) */
enum Consts {
    MAX_WELEM = 16
};


/**
 * Maximum frame size that can transmitted by the protocol.
 * @note: server/client can negotiate actual frame size to be less then that.
 */
extern const size_type kMaxMesssageSize;


/**
 *  Flags for the mode field in Topen and Tcreate messages
 */
enum class OpenMode : Solace::byte {
    READ   = 0,     //!< open read-only
    WRITE  = 1,     //!< open write-only
    RDWR   = 2,     //!< open read-write
    EXEC   = 3,     //!< execute (== read but check execute permission)
    TRUNC  = 16,    //!< or'ed in (except for exec), truncate file first
    CEXEC  = 32,    //!< or'ed in, close on exec
    RCLOSE = 64,    //!< or'ed in, remove on close
};


/**
 * Qid's type as encoded into bit vector corresponding to the high 8 bits of the file's mode word.
 * Represents the type of a file (directory, etc.).
 * @see Qid.type
 */
enum class QidType : Solace::byte {
    DIR    = 0x80,  //!< directories
    APPEND = 0x40,  //!< append only files
    EXCL   = 0x20,  //!< exclusive use files
    MOUNT  = 0x10,  //!< mounted channel
    AUTH   = 0x08,  //!< authentication file (afid)
    TMP    = 0x04,  //!< non-backed-up file
    LINK   = 0x02,  //!< bit for symbolic link (Unix, 9P2000.u)
    FILE   = 0x00,  //!< bits for plain file
};


/* bits in Stat.mode */
enum class DirMode : Solace::uint32 {
    DIR         = 0x80000000,	/* mode bit for directories */
    APPEND      = 0x40000000,	/* mode bit for append only files */
    EXCL        = 0x20000000,	/* mode bit for exclusive use files */
    MOUNT       = 0x10000000,	/* mode bit for mounted channel */
    AUTH        = 0x08000000,	/* mode bit for authentication file */
    TMP         = 0x04000000,	/* mode bit for non-backed-up file */

    SYMLINK     = 0x02000000,	/* mode bit for symbolic link (Unix, 9P2000.u) */
    DEVICE      = 0x00800000,	/* mode bit for device file (Unix, 9P2000.u) */
    NAMEDPIPE   = 0x00200000,	/* mode bit for named pipe (Unix, 9P2000.u) */
    SOCKET      = 0x00100000,	/* mode bit for socket (Unix, 9P2000.u) */
    SETUID      = 0x00080000,	/* mode bit for setuid (Unix, 9P2000.u) */
    SETGID      = 0x00040000,	/* mode bit for setgid (Unix, 9P2000.u) */

    READ        = 0x4,		/* mode bit for read permission */
    WRITE       = 0x2,		/* mode bit for write permission */
    EXEC        = 0x1,		/* mode bit for execute permission */
};



/**
 * The qid represents the server's unique identification for the file being accessed:
 * two files on the same server hierarchy are the same if and only if their qids are the same.
 */
struct Qid {
    Solace::byte	type;
    Solace::uint32	version;
    Solace::uint64  path;
};

/**
 * Stat about a file on the server.
 */
struct Stat {
    Solace::uint16      size;       //!< Total byte count of the following data
    Solace::uint16      type;       //!< server type (for kernel use)
    Solace::uint32      dev;        //!< server subtype (for kernel use)
    Qid                 qid;        //!< unique id from server, @see Qid
    Solace::uint32      mode;       //!< permissions and flags
    Solace::uint32      atime;      //!< last read time
    Solace::uint32      mtime;      //!< last write time
    Solace::uint64      length;     //!< length of the file in bytes
    Solace::StringView  name;       //!< file name; must be '/' if the file is the root directory of the server
    Solace::StringView  uid;        //!< owner name
    Solace::StringView  gid;        //!< group name
    Solace::StringView  muid;       //!< name of the user who last modified the file
};


/**
 * 9P message types
 */
enum class MessageType : Solace::byte {
    _beginSupportedMessageCode = 100,
    TVersion = 100,
    RVersion,
    TAuth = 102,
    RAuth,
    TAttach = 104,
    RAttach,
    TError = 106, /* illegal */
    RError,
    TFlush = 108,
    RFlush,
    TWalk = 110,
    RWalk,
    TOpen = 112,
    ROpen,
    TCreate = 114,
    RCreate,
    TRead = 116,
    RRead,
    TWrite = 118,
    RWrite,
    TClunk = 120,
    RClunk,
    TRemove = 122,
    RRemove,
    TStat = 124,
    RStat,
    TWStat = 126,
    RWStat,

    /**
     * 9P2000.e extension
     */
    TSession = 150,
    RSession,
    TSRead = 152,
    RSRead,
    TSWrite = 154,
    RSWrite,

    _endSupportedMessageCode
};


/**
 * Fixed size common message header that all messages start with
 */
struct MessageHeader {
    size_type       messageSize;    //!< Size of the message including size of the header and size field itself
    MessageType     type;           //!< Type of the message. @see MessageType.
    Tag             tag;            //!< Message tag for concurrent messages.

    constexpr size_type payloadSize() const noexcept;
};


/**
 * Get size in bytes of the mandatory protocol message header.
 * @see MessageHeader
 * @return Size in bytes of the mandatory protocol message header.
 */
inline constexpr size_type headerSize() noexcept {
    // Note: don't use sizeof(MessageHeader) due to possible padding
    return  sizeof(MessageHeader::messageSize) +
            sizeof(MessageHeader::type) +
            sizeof(MessageHeader::tag);
}

constexpr size_type MessageHeader::payloadSize() const noexcept {
    // Do we care about negative sizes?
    return messageSize - headerSize();
}


inline
MessageHeader makeHeaderWithPayload(MessageType type, Tag tag, size_type payloadSize) noexcept {
    return { headerSize() + payloadSize, type, tag };
}



/**
 * Request message as decoded from a buffer.
 */
struct Request {

    /**
     * The version request. Must be the first message sent on the connection.
     * It negotiates the protocol version and message size to be used on the connection and
     * initializes the connection for I/O.
     */
    struct Version {
        size_type           msize;      //!< The client suggested maximum message size in bytes.
        Solace::StringView  version;    //!< The version string identifies the level of the protocol.
    };

    /** Messages to establish a connection.
     *
     */
    struct Auth {
        Fid                 afid;       //!< A new fid to be established for authentication.
        Solace::StringView  uname;      //!< User identified by the message.
        Solace::StringView  aname;      //!< file tree to access.
    };

    /**
     * Abort a message
     */
    struct Flush {
        Tag        oldtag;              //!< Tag of the message to abort.
    };

    /**
     * A fresh introduction from a user on the client machine to the server.
     */
    struct Attach {
        Fid                 fid;        //!< Client fid to be use as the root directory of the desired file tree.
        Fid                 afid;       //!< Specifies a fid previously established by an auth message.
        Solace::StringView  uname;      //!< Idnetification of a user. All actions will be performed as this user.
        Solace::StringView  aname;      //!< Selected file-tree to attach to.
    };

    /**
     * A message to causes the server to change the current file
     * associated with a fid to be a file in the directory that is the old current file,
     * or one of its subdirectories.
     */
    struct Walk {
        Fid             fid;            //!< Fid of the directory where to start walk from.
        Fid             newfid;         //!< A client provided new fid representing resulting file.
        Solace::Path    path;           //!< A path to walk from the fid.
    };

    /**
     * The open request asks the file server to check permissions and
     * prepare a fid for I/O with subsequent read and write messages.
     */
    struct Open {
        Fid         fid;    //!< Client provided Fid to represent the newly opened file.
        OpenMode    mode;   //!< The mode determines the type of I/O. @see OpenMode
    };

    /**
     * The create request asks the file server to create a new file with the name supplied,
     * in the directory (dir) represented by fid, and requires write permission in the directory.
     * The owner of the file is the implied user id of the request.
     */
    struct Create {
        Fid                 fid;    //!< Fid of the directory where the file should be created.
        Solace::StringView  name;   //!< Name of the file to be created.
        Solace::uint32      perm;   //!< Permissions to the newly created file.
        OpenMode            mode;   //!< The mode the file will be opened in. @see OpenMode
    };

    /**
     * The read request asks for count bytes of data from the file.
     * The file must be opened for reading.
     */
    struct Read {
        Fid             fid;        //!< The file to read from, which must be opened for reading.
        Solace::uint64  offset;     //!< Starting offset bytes after the beginning of the file to read from.
        Solace::uint32  count;      //!< Number of bytes to read.
    };

    /**
     * The write request asks that count bytes of data be recorded in the file.
     * The file must be opened for writing.
     */
    struct Write {
        Fid                         fid;        //!< The file to write into.
        Solace::uint64              offset;     //!< Starting offset bytes after the beginning of the file.
        Solace::MemoryView data;       //!< A data to be written into the file.
    };

    /**
     * The clunk request informs the file server that the current file is no longer needed by the client.
     */
    struct Clunk {
        Fid        fid;     //!< File to foget about.
    };

    /**
     * The remove request asks the file server both to remove the file represented by fid and
     * to clunk the fid, even if the remove fails.
     */
    struct Remove {
        Fid        fid;     //!< File to remove.
    };

    /**
     * The stat transaction inquires about the file identified by fid.
     */
    struct StatRequest {
        Fid        fid;     //!< File to enquire about.
    };

    /**
     * A request to update file stat fields.
     */
    struct WStat {
        Fid         fid;    //!< Fid of the file to update stats on.
        Stat        stat;   //!< New stats to update file info to.
    };


};


/// 9P2000 protocol Erlang extension new messages
struct Request_9P2000E {
    /**
     * A request to re-establish a session.
     */
    struct Session {
        Solace::byte key[8];    //!< A key of the previously established session.
    };

    /**
     * A request to read entire file contents.
     */
    struct SRead {
        Fid             fid;    //!< Fid of the root directory to walk the path from.
        Solace::Path    path;   //!< A path to the file to be read.
    };

    /**
     * A request to overwrite file contents.
     */
    struct SWrite {
        Fid                         fid;    //!< Fid of the root directory to walk the path from.
        Solace::Path                path;   //!< A path to the file to be read.
        Solace::MemoryView data;   //!< A data to be written into the file.
    };
};



struct Response {

    struct Version {
        size_type           msize;
        Solace::StringView  version;
    };

    struct Auth {
        Qid  qid;
    };

    struct Attach {
        Qid  qid;
    };

    struct Error {
        Solace::StringView  ename;
    };

    struct Flush {};

    struct Walk {
        var_datum_size_type nqids;
        Qid     qids[MAX_WELEM];
    };

    struct Open {
        Qid  qid;
        size_type iounit;
    };

    struct Create {
        Qid  qid;
        size_type iounit;
    };

    struct Read {
        Solace::MutableMemoryView data;
    };

    struct Write {
        size_type       count;
    };

    struct Clunk {};

    struct Remove {};

    struct Stat {
        var_datum_size_type dummySize;
        ::styxe::Stat data;
    };

    struct WStat {};
};


/// 9P2000 protocol Erlang extension new messages
struct Response_9P2000E {
    struct Session {};
};



struct TypedWriter {

    TypedWriter(Solace::ByteWriter& buffer, Solace::ByteWriter::size_type pos, MessageHeader head)
        : _buffer{buffer}
        , _pos{pos}
        , _header{head}
    {}

    constexpr Solace::ByteWriter& buffer() noexcept { return _buffer; }
    Solace::ByteWriter& build();

    constexpr Tag tag() const noexcept { return _header.tag; }
    constexpr MessageType type() const noexcept { return _header.type; }
    constexpr size_type payloadSize() const noexcept { return _header.payloadSize(); }

private:
    Solace::ByteWriter&     _buffer;
    Solace::ByteWriter::size_type _pos;

    MessageHeader           _header;
};


/**
 * Helper class to build Request messages.
 */
class RequestBuilder {
public:

    RequestBuilder(Solace::ByteWriter& dest, Tag tag = 1) noexcept
        : _tag{tag}
        , _buffer{dest}
    {}

    /**
     * Create a version request.
     * @param version Suggested protocol version.
     * @param maxMessageSize Suggest maximum size of the protocol message, including mandatory message header.
     * @return Ref to this for fluent interface.
     */
    TypedWriter version(Solace::StringView version,
                        size_type maxMessageSize = kMaxMesssageSize) const;
    TypedWriter auth(Fid afid, Solace::StringView userName, Solace::StringView attachName) const;
    TypedWriter flush(Tag oldTransation) const;
    TypedWriter attach(Fid fid, Fid afid,
                            Solace::StringView userName, Solace::StringView attachName);
    TypedWriter walk(Fid fid, Fid nfid, Solace::Path const& path);
    TypedWriter open(Fid fid, OpenMode mode);
    TypedWriter create(Fid fid,
                            Solace::StringView name,
                            Solace::uint32 permissions,
                            OpenMode mode);
    TypedWriter read(Fid fid, Solace::uint64 offset, size_type count);
    TypedWriter write(Fid fid, Solace::uint64 offset, Solace::MemoryView data);
    TypedWriter clunk(Fid fid);
    TypedWriter remove(Fid fid);
    TypedWriter stat(Fid fid);
    TypedWriter writeStat(Fid fid, Stat const& stat);

    /* 9P2000.e extention */
    TypedWriter session(Solace::MemoryView key);
    TypedWriter shortRead(Fid rootFid, Solace::Path const& path);
    TypedWriter shortWrite(Fid rootFid, Solace::Path const& path, Solace::MemoryView data);

private:
    Tag const               _tag;
    Solace::ByteWriter&     _buffer;
};



/**
 * Helper class to build response messages.
 */
class ResponseBuilder {
public:

    constexpr ResponseBuilder(Solace::ByteWriter& dest, Tag tag) noexcept
        : _tag{tag}
        , _buffer{dest}
    {}

    TypedWriter version(Solace::StringView version, size_type maxMessageSize = kMaxMesssageSize);
    TypedWriter auth(Qid qid);
    TypedWriter error(Solace::StringView message);
    TypedWriter error(Solace::Error const& err) {
        return error(err.toString().view());
    }

    TypedWriter flush();
    TypedWriter attach(Qid qid);
    TypedWriter walk(Solace::ArrayView<Qid> const& qids);
    TypedWriter open(Qid qid, size_type iounit);
    TypedWriter create(Qid qid, size_type iounit);
    TypedWriter read(Solace::MemoryView data);
    TypedWriter write(size_type iounit);
    TypedWriter clunk();
    TypedWriter remove();
    TypedWriter stat(Stat const& value);
    TypedWriter wstat();

    /* 9P2000.e extention */
    TypedWriter session();
    TypedWriter shortRead(Solace::MemoryView data);
    TypedWriter shortWrite(size_type iounit);

private:
    Tag const               _tag;
    Solace::ByteWriter&     _buffer;
};



using RequestMessage = std::variant<
                        Request::Version,
                        Request::Auth,
                        Request::Flush,
                        Request::Attach,
                        Request::Walk,
                        Request::Open,
                        Request::Create,
                        Request::Read,
                        Request::Write,
                        Request::Clunk,
                        Request::Remove,
                        Request::StatRequest,
                        Request::WStat,
                        Request_9P2000E::Session,
                        Request_9P2000E::SRead,
                        Request_9P2000E::SWrite
                        >;

using ResponseMessage = std::variant<
                            Response::Version,
                            Response::Auth,
                            Response::Attach,
                            Response::Error,
                            Response::Flush,
                            Response::Walk,
                            Response::Open,
                            Response::Create,
                            Response::Read,
                            Response::Write,
                            Response::Clunk,
                            Response::Remove,
                            Response::Stat,
                            Response::WStat,
                            Response_9P2000E::Session
                            >;


/**
 * An implementation of 9P2000 protocol.
 * The protocol is state-full as version, supported extentions and messages size are negotiated.
 * Thus this info must be preserved during communication. Instance of this class serves this purpose as well as
 * helps with message parsing.
 *
 * @note The implementation of the protocol does not allocate memory for any operation.
 * Message parser acts on an instanc of the user provided Solace::ReadBuffer and any message data such as
 * name string or data read from a file is actually a pointer to the underlying ReadBuffer storage.
 * Thus it is user's responsibility to manage lifetime of that buffer.
 * (That is not technically correct as current implementation does allocate memory when dealing with Solace::Path
 * objects as there is no currently availiable PathView version)
 *
 * In order to create 9P2000 messages please @see RequestBuilder.
 */
class Parser {
public:

    /** String representing version of protocol. */
    static const Solace::StringLiteral PROTOCOL_VERSION;

    /** String const for unknow version. */
    static const Solace::StringLiteral UNKNOWN_PROTOCOL_VERSION;


    /** Special value of a message tag representing 'no tag'. */
    static const Tag NO_TAG;

    /** Special value of a message FID representing 'no Fid'. */
    static const Fid NOFID;

public:

    /// Default destructor
    ~Parser() noexcept = default;

    Parser(Parser const& ) = delete;
    Parser& operator= (Parser const& ) = delete;

    /**
     * Construct a new instance of the protocol.
     * Usually one would create an instance per connection as protocol stores state per estanblished session.
     * @param maxMassageSize Maximum message size in bytes.
     * This is advertized by the protocol during version/size negotiation.
     * @param version Supported protocol version. This is advertized by the protocol during version/size negotiation.
     */
    Parser(size_type maxMassageSize = kMaxMesssageSize,
           Solace::StringView version = PROTOCOL_VERSION) noexcept
        : _maxMassageSize(maxMassageSize)
        , _maxNegotiatedMessageSize(maxMassageSize)
        , _initialVersion(version)
        , _negotiatedVersion(makeString(version))
    {
    }

    /**
     * Get maximum message size supported by the protocol instance.
     * @return Maximum message size in bytes.
     */
    constexpr size_type maxPossibleMessageSize() const noexcept {
        return _maxMassageSize;
    }

    /**
     * Get negotiated message size to be used in an established session.
     * @return Negotiated message size in bytes.
     */
    constexpr size_type maxNegotiatedMessageSize() const noexcept {
        return _maxNegotiatedMessageSize;
    }

    /**
     * Set negotiated message size.
     * @param newMessageSize Size of the message in bytes. This is maximum size of the message that will be communicated
     * @return Actually set message size which may be less then requested if requested message size was more then max.
     */
    size_type maxNegotiatedMessageSize(size_type newMessageSize);

    /**
     * Get negotiated protocol version effective for the estanblished session.
     * @return Negotiated version string.
     */
    constexpr Solace::String const& getNegotiatedVersion() const noexcept {
        return _negotiatedVersion;
    }

    /**
     * Set negotiated protocol version.
     * @param version A new negotited protocol version.
     */
    void setNegotiatedVersion(Solace::String&& version) noexcept {
        _negotiatedVersion = Solace::mv(version);
    }

    /**
     * Parse 9P message header from a byte byffer.
     * @param buffer Byte buffer to read message header from.
     * @return Resulting message header if parsed successfully or an error otherwise.
     */
    Solace::Result<MessageHeader, Solace::Error>
    parseMessageHeader(Solace::ByteReader& buffer) const;

    /**
     * Parse 9P Response type message from a byte byffer.
     * This is the primiry method used by a client to parse response from the server.
     *
     * @param header Message header.
     * @param data Byte buffer to read message content from.
     * @return Resulting message if parsed successfully or an error otherwise.
     */
    Solace::Result<ResponseMessage, Solace::Error>
    parseResponse(MessageHeader const& header, Solace::ByteReader& data) const;

    /**
     * Parse 9P Request type message from a byte byffer.
     * This is the primiry method used by a server implementation to parse requests from a client.

     * @param header Message header.
     * @param data Byte buffer to read message content from.
     * @return Resulting message if parsed successfully or an error otherwise.
     */
    Solace::Result<RequestMessage, Solace::Error>
    parseRequest(MessageHeader const& header, Solace::ByteReader& data) const;

private:

    size_type const         _maxMassageSize;                /// Initial value of the maximum message size in bytes.
    size_type               _maxNegotiatedMessageSize;      /// Negotiated value of the maximum message size in bytes.

    Solace::StringView const    _initialVersion;            /// Initial value of the used protocol version.
    Solace::String              _negotiatedVersion;         /// Negotiated value of the protocol version.
};


inline
bool operator == (Qid const& lhs, Qid const& rhs) noexcept {
    return (lhs.path == rhs.path &&
            lhs.version == rhs.version &&
            lhs.type == rhs.type);
}


inline
bool operator == (Stat const& lhs, Stat const& rhs) noexcept {
    return (lhs.atime == rhs.atime &&
            lhs.dev == rhs.dev &&
            lhs.gid == rhs.gid &&
            lhs.length == rhs.length &&
            lhs.mode == rhs.mode &&
            lhs.mtime == rhs.mtime &&
            lhs.name == rhs.name &&
            lhs.qid == rhs.qid &&
            lhs.size == rhs.size &&
            lhs.type == rhs.type &&
            lhs.uid == rhs.uid);
}


/**
 * @brief A helper class that allows to build responce content for DIR `read` request.
 * @see Protocol::Request::Read
 *
 * This is a helper class design to help server implementors to build DIR read responce.
 * May responsibility is of this class is to keep track of offset and count received from the request.
 * The implementation only measures how much data is would have encoded untill it reaches 'offset' value.
 * Only after that data provided to encode will be actually encoded
 * untill count bytest has been written into the destination buffer.
 *
 * @example
 * \code{.cpp}
 * ...
    DirListingWriter encoder{dest, count, offset};
    for (auto const& dirEntry : entries) {
        if (!encoder.encode(mapEntryStats(dirEntry))) {
            break;
        }
    }
    ...
 * \endcode
 *
 */
struct DirListingWriter {

    static var_datum_size_type sizeStat(Stat const& stat);

    /**
     * @brief Create an instance of Dir listing writer that encodes no more then 'inCount' bytes after the offset.
     * @param inCount Maximum number of bytes that can be written into dest.
     * @param inOffset Number of bytes to skip.
     * @param dest Output buffer where resuling data is written.
     */
    constexpr DirListingWriter(Solace::ByteWriter& dest, Solace::uint32 inCount, Solace::uint64 inOffset) noexcept
        : offset{inOffset}
        , count{inCount}
        , _dest{dest}
    {}


    /**
     * @brief Encode directory entry into response message
     * @param stat Directory entry stat.
     * @return True if more entries can be encoded.
     */
    bool encode(Stat const& stat);

    constexpr auto bytesTraversed() const noexcept { return _bytesTraversed; }
    constexpr auto bytesEncoded() const noexcept { return _bytesEncoded; }

private:
    Solace::uint64 _bytesTraversed{0};
    Solace::uint64 const offset;
    Solace::uint32 const count;
    Solace::uint32 _bytesEncoded{0};

    Solace::ByteWriter& _dest;
};


}  // end of namespace styxe
#endif  // STYXE_9P2000_HPP
