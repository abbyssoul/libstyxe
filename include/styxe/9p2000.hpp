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
#include <solace/readBuffer.hpp>
#include <solace/byteBuffer.hpp>

#include <solace/result.hpp>
#include <solace/error.hpp>
#include <solace/path.hpp>


namespace styxe {

/**
 * An implementation of 9P2000 protocol.
 * The protocol is state-full as version, supported extentions and messages size are negotiated.
 * Thus this info must be preserved during communication. Instance of this class serves this purpose as well as
 * helps with message parsing.
 *
 * In order to create 9P2000 messages please @see P9Protocol::RequestBuilder.
 */
class P9Protocol {
public:
    /** Network protocol uses fixed width int32 to represent size of data in bytes */
    using size_type = Solace::uint32;
    /** Type of message Tag */
    using Tag = Solace::uint16;
    /** Type of file identifiers client uses to identify a ``current file'' on the server*/
    using Fid = Solace::uint32;

    /**
     * Maximum frame size that can transmitted by the protocol.
     * @note: server/client can negotiate actual frame size to be less then that.
     */
    static const size_type MAX_MESSAGE_SIZE;

    /**
     * String representing version of protocol.
     */
    static const Solace::StringLiteral PROTOCOL_VERSION;

    /**
     * String const for unknow version.
     */
    static const Solace::StringLiteral UNKNOWN_PROTOCOL_VERSION;


    /**
     * Special value of a message tag representing 'no tag'.
     */
    static const Tag NO_TAG;

    /**
     * Special value of a message FID representing 'no Fid'.
     */
    static const Fid NOFID;

    /** Compile time constants (probably should be removed) */
    enum Consts {
        MAX_WELEM = 16
    };

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
     */
    enum class QidType : Solace::byte {
        DIR    = 0x80,  //!< directories
        APPEND = 0x40,  //!< append only files
        EXCL   = 0x20,  //!< exclusive use files
        MOUNT  = 0x10,  //!< mounted channel
        AUTH   = 0x08,  //!< authentication file (afid)
        TMP    = 0x04,  //!< non-backed-up file
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
     * Helper class to decode data structures from the 9P2000 formatted messages.
     */
    class Decoder {
    public:

        Decoder(Solace::ReadBuffer& src) :
            _src(src)
        {}

        Decoder& read(Solace::uint8* dest);
        Decoder& read(Solace::uint16* dest);
        Decoder& read(Solace::uint32* dest);
        Decoder& read(Solace::uint64* dest);
        Decoder& read(Solace::StringView* dest);
        Decoder& read(Solace::ImmutableMemoryView* data);
        Decoder& read(Solace::Path* path);
        Decoder& read(Qid* qid);
        Decoder& read(Stat* stat);

    private:
        Solace::ReadBuffer& _src;
    };


    /**
     * Helper class to encode data into the protocol message format.
     */
    class Encoder {
    public:

        Encoder(Solace::ByteBuffer& dest) :
            _dest(dest)
        {}

        Encoder& header(MessageType type, Tag tag, size_type payloadSize = 0);
        Encoder& encode(Solace::uint8 value);
        Encoder& encode(Solace::uint16 value);
        Encoder& encode(Solace::uint32 value);
        Encoder& encode(Solace::uint64 value);
        Encoder& encode(const Solace::StringView& str);
        Encoder& encode(const Solace::String& str) = delete;
        Encoder& encode(const P9Protocol::Qid& qid);
        Encoder& encode(const Solace::Array<P9Protocol::Qid>& qids);
        Encoder& encode(const P9Protocol::Stat& stat);
        Encoder& encode(const Solace::ImmutableMemoryView& data);
        Encoder& encode(const Solace::Path& path);

        size_type protocolSize(const Solace::uint8& value);
        size_type protocolSize(const Solace::uint16& value);
        size_type protocolSize(const Solace::uint32& value);
        size_type protocolSize(const Solace::uint64& value);
        size_type protocolSize(const Solace::StringView& str);
        size_type protocolSize(const Solace::String& str) = delete;
        size_type protocolSize(const Solace::Path& path);
        size_type protocolSize(const P9Protocol::Qid&);
        size_type protocolSize(const P9Protocol::Stat& stat);
        size_type protocolSize(const Solace::Array<P9Protocol::Qid>& qids);
        size_type protocolSize(const Solace::ImmutableMemoryView& data);

    private:
        Solace::ByteBuffer& _dest;
    };

    /**
     * Common header that all messages have.
     */
    struct MessageHeader {
        size_type       size;   //!< Size of the message including size of the header and size field itself.
        MessageType     type;   //!< Type of the message. @see MessageType.
        Tag             tag;    //!< Message tag for concurent messages.
    };


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
            Solace::ImmutableMemoryView data;       //!< A data to be written into the file.
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


        /**
         * A request to re-establish a session.
         */
        struct Session {
            Solace::ImmutableMemoryView key;    //!< A key of the previously established session.
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
            Solace::ImmutableMemoryView data;   //!< A data to be written into the file.
        };


        Request(MessageType rtype, Tag tag);
        Request(Request&& rhs);

        ~Request();

        Tag tag() const noexcept { return _tag; }

        MessageType type() const noexcept { return _type; }

        Version&        asVersion();
        Auth&           asAuth();
        Flush&          asFlush();
        Attach&         asAttach();
        Walk&           asWalk();
        Open&           asOpen();
        Create&         asCreate();
        Read&           asRead();
        Write&          asWrite();
        Clunk&          asClunk();
        Remove&         asRemove();
        StatRequest&    asStat();
        WStat&          asWstat();
        Session&        asSession();
        SRead&          asShortRead();
        SWrite&         asShortWrite();

    private:

        Tag             _tag;
        MessageType     _type;
        union {
            Version     version;
            Auth        auth;
            Flush       flush;
            Attach      attach;
            Walk        walk;
            Open        open;
            Create      create;
            Read        read;
            Write       write;
            Clunk       clunk;
            Remove      remove;
            StatRequest stat;
            WStat       wstat;

            /* 9P2000.e extention */
            Session     session;
            SRead       shortRead;
            SWrite      shortWrite;
        };
    };


    /**
     * Helper class to build Request messages.
     */
    class RequestBuilder {
    public:

        RequestBuilder(Solace::ByteBuffer& dest) noexcept :
            _tag(1),
            _payloadSize(0),
            _buffer(dest)
        {}

        Solace::ByteBuffer& buffer() noexcept {
            return _buffer;
        }

        Solace::ByteBuffer& build();

        /**
         * Set response message tag
         * @param value Tag of the response message.
         * @return Ref to this for a fluent interface.
         */
        RequestBuilder& tag(Tag value) noexcept {
            _tag = value;
            return (*this);
        }

        Tag tag() const noexcept { return _tag; }
        MessageType type() const noexcept { return _type; }
        size_type payloadSize() const noexcept { return _payloadSize; }

        /**
         * Create a version request.
         * @param version Suggested protocol version.
         * @param maxMessageSize Suggest maximum size of the protocol message, including mandatory message header.
         * @return Ref to this for fluent interface.
         */
        RequestBuilder& version(const Solace::StringView& version = PROTOCOL_VERSION,
                                size_type maxMessageSize = MAX_MESSAGE_SIZE);
        RequestBuilder& auth(Fid afid, const Solace::StringView& userName, const Solace::StringView& attachName);
        RequestBuilder& flush(Tag oldTransation);
        RequestBuilder& attach(Fid fid, Fid afid,
                                const Solace::StringView& userName, const Solace::StringView& attachName);
        RequestBuilder& walk(Fid fid, Fid nfid, const Solace::Path& path);
        RequestBuilder& open(Fid fid, OpenMode mode);
        RequestBuilder& create(Fid fid,
                                const Solace::StringView& name,
                                Solace::uint32 permissions,
                                OpenMode mode);
        RequestBuilder& read(Fid fid, Solace::uint64 offset, size_type count);
        RequestBuilder& write(Fid fid, Solace::uint64 offset, const Solace::ImmutableMemoryView& data);
        RequestBuilder& clunk(Fid fid);
        RequestBuilder& remove(Fid fid);
        RequestBuilder& stat(Fid fid);
        RequestBuilder& writeStat(Fid fid, const Stat& stat);

        /* 9P2000.e extention */
        RequestBuilder& session(const Solace::ImmutableMemoryView& key);
        RequestBuilder& shortRead(Fid rootFid, const Solace::Path& path);
        RequestBuilder& shortWrite(Fid rootFid, const Solace::Path& path, const Solace::ImmutableMemoryView& data);

    private:
        Tag                     _tag;
        MessageType             _type;
        size_type               _payloadSize;

        Solace::ByteBuffer&     _buffer;
    };


    struct Response {

        struct Write {
            size_type       count;
        };

        struct Version {
            size_type       msize;
            Solace::StringView  version;
        };

        struct Error {
            Solace::StringView  ename;
        };

        struct Auth {
            Qid  qid;
        };

        struct Attach {
            Qid  qid;
        };

        struct Walk {
            Solace::uint16 nqids;
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
            Solace::MemoryView data;
        };


        MessageType type;
        Tag         tag;
        union {
            Write       write;
            Version     version;
            Error       error;
            Auth        auth;
            Attach      attach;
            Walk        walk;
            Open        open;
            Create      create;
            Read        read;
            Stat        stat;
        };

        Response(MessageType rtype, Tag tag);
        Response(Response&& rhs);

        ~Response();
    };

    /**
     * Helper class to build response messages.
     */
    class ResponseBuilder {
    public:

        ResponseBuilder(Solace::ByteBuffer& dest, Tag tag) :
            _tag(tag),
            _type(),
            _payloadSize(0),
            _initialPosition(dest.position()),
            _buffer(dest)
        {}

        Solace::ByteBuffer& buffer() noexcept {
            return _buffer;
        }

        Solace::ByteBuffer& build(bool recalcPayloadSize = false);

        /**
         * Set response message tag
         * @param value Tag of the response message.
         * @return Ref to this for a fluent interface.
         */
        ResponseBuilder& tag(Tag value) noexcept {
            _tag = value;
            return (*this);
        }

        Tag tag() const noexcept { return _tag; }
        MessageType type() const noexcept { return _type; }
        size_type payloadSize() const noexcept { return _payloadSize; }

        ResponseBuilder& updatePayloadSize();
        ResponseBuilder& updatePayloadSize(size_type payloadSize);


        ResponseBuilder& version(const Solace::StringView& version, size_type maxMessageSize = MAX_MESSAGE_SIZE);
        ResponseBuilder& auth(const Qid& qid);
        ResponseBuilder& error(const Solace::StringView& message);
        ResponseBuilder& error(const Solace::Error& err) {
            return error(err.toString());
        }

        ResponseBuilder& flush();
        ResponseBuilder& attach(const Qid& qid);
        ResponseBuilder& walk(const Solace::Array<Qid>& qids);
        ResponseBuilder& open(const Qid& qid, size_type iounit);
        ResponseBuilder& create(const Qid& qid, size_type iounit);
        ResponseBuilder& read(const Solace::ImmutableMemoryView& data);
        ResponseBuilder& write(size_type iounit);
        ResponseBuilder& clunk();
        ResponseBuilder& remove();
        ResponseBuilder& stat(const Stat& value);
        ResponseBuilder& wstat();

        /* 9P2000.e extention */
        ResponseBuilder& session();
        ResponseBuilder& shortRead(const Solace::ImmutableMemoryView& data);
        ResponseBuilder& shortWrite(size_type iounit);

    private:
        Tag                     _tag;
        MessageType             _type;
        size_type               _payloadSize;

        Solace::ByteBuffer::size_type   _initialPosition;
        Solace::ByteBuffer&             _buffer;
    };


    /**
     * Get size in bytes of the mandatory protocol message header.
     * @see MessageHeader
     * @return Size in bytes of the mandatory protocol message header.
     */
    static constexpr size_type headerSize() noexcept {
        // Note: can't use sizeof(MessageHeader) due to padding
        return  sizeof(MessageHeader::size) +
                sizeof(MessageHeader::type) +
                sizeof(MessageHeader::tag);
    }


public:

    /**
     * Construct a new instance of the protocol.
     * Usually one would create an instance per connection as protocol stores state per estanblished session.
     * @param maxMassageSize Maximum message size in bytes.
     * This is advertized by the protocol during version/size negotiation.
     * @param version Supported protocol version. This is advertized by the protocol during version/size negotiation.
     */
    P9Protocol(size_type maxMassageSize = MAX_MESSAGE_SIZE,
               const Solace::StringView& version = PROTOCOL_VERSION);

    /**
     * Get maximum message size supported by the protocol instance.
     * @return Maximum message size in bytes.
     */
    size_type maxPossibleMessageSize() const noexcept {
        return _maxMassageSize;
    }

    /**
     * Get negotiated message size to be used in an established session.
     * @return Negotiated message size in bytes.
     */
    size_type maxNegotiatedMessageSize() const noexcept {
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
    const Solace::String& getNegotiatedVersion() const noexcept {
        return _negotiatedVersion;
    }

    /**
     * Set negotiated protocol version.
     * @param version A new negotited protocol version.
     */
    void setNegotiatedVersion(const Solace::String& version) noexcept {
        _negotiatedVersion = version;
    }

    //---------------------------------------------------------
    // Create protocol requests
    //---------------------------------------------------------

    /**
     * Parse 9P message header from a byte byffer.
     * @param buffer Byte buffer to read message header from.
     * @return Resulting message header if parsed successfully or an error otherwise.
     */
    Solace::Result<MessageHeader, Solace::Error>
    parseMessageHeader(Solace::ReadBuffer& buffer) const;

    /**
     * Parse 9P Response type message from a byte byffer.
     * @param header Message header.
     * @param data Byte buffer to read message content from.
     * @return Resulting message if parsed successfully or an error otherwise.
     */
    Solace::Result<Response, Solace::Error>
    parseResponse(const MessageHeader& header, Solace::ReadBuffer& data) const;

    /**
     * Parse 9P Request type message from a byte byffer.
     * @param header Message header.
     * @param data Byte buffer to read message content from.
     * @return Resulting message if parsed successfully or an error otherwise.
     */
    Solace::Result<Request, Solace::Error>
    parseRequest(const MessageHeader& header, Solace::ReadBuffer& data) const;

private:

    const size_type       _maxMassageSize;
    size_type       _maxNegotiatedMessageSize;

    const Solace::String  _initialVersion;
    Solace::String  _negotiatedVersion;
};


inline
bool operator == (const P9Protocol::Qid& lhs, const P9Protocol::Qid& rhs) {
    return (lhs.path == rhs.path &&
            lhs.version == rhs.version &&
            lhs.type == rhs.type);
}


inline
bool operator == (const P9Protocol::Stat& lhs, const P9Protocol::Stat& rhs) {
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

}  // end of namespace styxe
#endif  // STYXE_9P2000_HPP
