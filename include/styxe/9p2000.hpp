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

#include "decoder.hpp"

#include "errorDomain.hpp"
#include "9p.hpp"
#include "messageWriter.hpp"


#include <solace/arrayView.hpp>
#include <solace/string.hpp>   // Error.toString() is partial without it
#include <solace/result.hpp>

#include <variant>


namespace styxe {


/// Protocol version literal
extern const Solace::StringLiteral kProtocolVersion;

/**
 * Minimum frame size that can used by the protocol.
 * @note: server/client should negotiate actual frame size that is larger then that.
 */
extern const size_type kMinMessageSize;

/**
 * Maximum frame size that can transmitted by the protocol.
 * @note: server/client can negotiate actual frame size to be less then that.
 */
extern const size_type kMaxMessageSize;


/**
 *  Flags for the mode field in TOpen and TCreate messages
 */
struct OpenMode {
	static const Solace::byte READ   = 0;     //!< open read-only
	static const Solace::byte WRITE  = 1;     //!< open write-only
	static const Solace::byte RDWR   = 2;     //!< open read-write
	static const Solace::byte EXEC   = 3;     //!< execute (== read but check execute permission)
	static const Solace::byte TRUNC  = 16;    //!< or'ed in (except for exec), truncate file first
	static const Solace::byte CEXEC  = 32;    //!< or'ed in, close on exec
	static const Solace::byte RCLOSE = 64;    //!< or'ed in, remove on close

	constexpr OpenMode() noexcept
		: mode{0}
	{}

	/**
	 * @brief Construct a new OpenMode from a byte
	 * @param m Byte representing open mode.
	 */
	constexpr OpenMode(Solace::byte m) noexcept
		: mode{m}
	{}

	/**
	 * @brief operator=
	 * @param rhs A flag-storage to assign from
	 * @return Reference to this
	 */
	OpenMode& operator= (Solace::byte rhs) noexcept {
		mode = rhs;
		return *this;
	}

	Solace::byte	mode;  //!< A storage for the file openning flags.
};


inline
bool operator== (OpenMode lhs, OpenMode rhs) noexcept { return (lhs.mode == rhs.mode); }
inline
bool operator!= (OpenMode lhs, OpenMode rhs) noexcept { return (lhs.mode != rhs.mode); }
inline
bool operator== (OpenMode lhs, Solace::byte rhs) noexcept { return (lhs.mode == rhs); }
inline
bool operator== (Solace::byte lhs, OpenMode rhs) noexcept { return (lhs == rhs.mode); }



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


/**
 * Metadata record about about a file on the server.
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


inline
bool operator== (Stat const& lhs, Stat const& rhs) noexcept {
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

inline
bool operator!= (Stat const& lhs, Stat const& rhs) noexcept {
	return !(lhs == rhs);
}



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

	_endSupportedMessageCode = RWStat
};


/**
 * Request message as decoded from a buffer.
 */
struct Request {

	struct Partial {
		/**
		 * Parial Walk message. @see Request::Walk for message details.
		 */
		struct Walk {
			Fid             fid;            //!< Fid of the directory where to start walk from.
			Fid             newfid;         //!< A client provided new fid representing resulting file.
		};

		struct Write {
			Fid					fid;        //!< The file to write into.
			Solace::uint64		offset;     //!< Starting offset bytes after the beginning of the file.
		};
	};


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
	 * associated with a fid to be a file in the directory that is identified by following a given path.
	 */
	struct Walk : public Partial::Walk {
		WalkPath		path;           //!< A path to walk from the fid.
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
		Fid					fid;        //!< The file to read from, which must be opened for reading.
		Solace::uint64		offset;     //!< Starting offset bytes after the beginning of the file to read from.
		Solace::uint32		count;      //!< Number of bytes to read.
	};

	/**
	 * The write request asks that count bytes of data be recorded in the file.
	 * The file must be opened for writing.
	 */
	struct Write: public Partial::Write {
		Solace::MemoryView	data;       //!< A data to be written into the file.
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
	struct Stat {
		Fid        fid;     //!< File to enquire about.
	};

	/**
	 * A request to update file stat fields.
	 */
	struct WStat {
		Fid					fid;    //!< Fid of the file to update stats on.
		::styxe::Stat		stat;   //!< New stats to update file info to.
	};

};


/**
* Response message as decoded from a buffer.
 */
struct Response {

	struct Partial {
		struct Read {};
		struct Error {};
	};

	/// Version response
	struct Version {
		/// Maximum message size that server accepts and can receive
		size_type           msize;

		/// Maximum protocol version accepted by the server
		Solace::StringView  version;
	};

	/// Authentication response
	struct Auth {
		Qid  qid;  //!< Qid of the auth file selected one was requrested
	};

	/// Attach requrest
	struct Attach {
		Qid  qid;  //!< Qid of the attached FS
	};

	/// Error resoponse from a server
	struct Error {
		/// Error description
		Solace::StringView  ename;
	};

	/// Flush response
	struct Flush {};

	/// Walk response
	struct Walk {
		var_datum_size_type nqids;  //!< Number of qids returned
		Qid qids[16];               //!< QIDs of the directories walked
	};

	/// Open file response
	struct Open {
		Qid				qid;			//!< Qid of the opened file

		/// Hint for the number of bytes to read in a single operation
		size_type	iounit;
	};

	/// Create file response
	struct Create {
		Qid  qid;					//!< Qid of the created file
		size_type iounit;		//!< Hint numbed of bytes FS may read in a single op
	};

	/// Read resopose
	struct Read : public Partial::Read {
		Solace::MemoryView data;  /// View in to the response buffer where raw read data is.
	};

	/// Write response
	struct Write {
		size_type  count;  //!< Number of bytes written
	};

	/// Clunk response
	struct Clunk {};

	/// Remove file response
	struct Remove {};

	/// Stat response
	struct Stat {
		var_datum_size_type dummySize;  //!< Dummy variable size of data.
		::styxe::Stat data;				//!< File stat data
	};

	/// Write stats resopose
	struct WStat {};

};



size_type protocolSize(Qid const& value) noexcept;
size_type protocolSize(Stat const& value) noexcept;


/**
 * @brief A helper class that allows to build response content for DIR `read` request.
 * @see Protocol::Request::Read
 *
 * This is a helper class design to help server implementors to build DIR read response.
 * May responsibility is of this class is to keep track of offset and count received from the request.
 * The implementation only measures how much data is would have encoded untill it reaches 'offset' value.
 * Only after that data provided to encode will be actually encoded
 * untill count bytest has been written into the destination buffer.
 *
 * \code{.cpp}
...
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

	/**
	 * Estimate the number of bytes required to serialize a give stat instance.
	 * @param stat Stat struct to estimate the size requirements for.
	 * @return Number of bytes requried for the serialized stat struct.
	 */
	template<typename StatType>
	static var_datum_size_type sizeStat(StatType const& stat) {
		return Solace::narrow_cast<var_datum_size_type>(protocolSize(stat) - sizeof(stat.size));
	}

	/**
	 * @brief Create an instance of Dir listing writer that encodes no more then 'maxBytes' bytes after the offset.
	 * @param writer Output stream where resuling data is written.
	 * @param maxBytes Maximum number of bytes that can be written into dest.
	 * @param offset Number of bytes to skip.
	 */
	DirListingWriter(ResponseWriter& writer, Solace::uint32 maxBytes, Solace::uint64 offset = 0) noexcept;

	/**
	 * @brief Encode directory entry into response message
	 * @param stat Directory entry stat.
	 * @return True if more entries can be encoded.
	 */
	bool encode(Stat const& stat);

	/// Update response writer with the current payload size.
	void updateDataSize();

	/** Get number of bytes travesed.
	 * @return Number of bytes seen so far.
	 */
	constexpr auto bytesTraversed() const noexcept { return _bytesTraversed; }

	/** Get number of bytes travesed.
	 * @return Number of bytes seen so far.
	 */
	constexpr auto bytesEncoded() const noexcept { return _bytesEncoded; }

private:
	Solace::ByteWriter::size_type	_dataPosition;

	/// Number of bytes traversed so far.
	Solace::uint64			_bytesTraversed{0};
	/// Number of bytes to skip before starting to write data.
	Solace::uint64 const	_offset;
	/// Max number of bytes to write.
	Solace::uint32 const	_maxBytes;
	/// Number of bytes written.
	Solace::uint32			_bytesEncoded{0};
	/// Writer to write data to.
	ResponseWriter&			_writer;
};


/** Encode a file Qid into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, Qid value) {
	return encoder << value.type
				   << value.version
				   << value.path;

}

/** Encode a file stats into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, Stat const& value) {
	return encoder << value.size
				   << value.type
				   << value.dev
				   << value.qid
				   << value.mode
				   << value.atime
				   << value.mtime
				   << value.length
				   << value.name
				   << value.uid
				   << value.gid
				   << value.muid;
}


/** Encode a list of qids into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, Solace::ArrayView<Qid> value) {
	// Encode variable datum size first:
	encoder << Solace::narrow_cast<var_datum_size_type>(value.size());

	// Datum
	for (auto const& qid : value) {
		encoder << qid;
	}

	return encoder;
}

inline
Encoder& operator<< (Encoder& encoder, WalkPath const& path) {
	// Encode variable datum size first:
	encoder << path.size();
	// Datum
	for (auto segment : path) {
		encoder << segment;
	}

	return encoder;
}


/** Decode a file Qid from the stream.
 * @param decoder A data stream to read a value from.
 * @param dest An address where to store decoded value.
 * @return Ref to the decoder or Error if operation has failed.
 */
inline
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Qid& dest) {
	return decoder >> dest.type
				   >> dest.version
				   >> dest.path;
}


/** Decode a Stat struct from the stream.
 * @param decoder A data stream to read a value from.
 * @param dest An address where to store decoded value.
 * @return Ref to the decoder or Error if operation has failed.
 */
inline
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, Stat& dest) {
	return decoder >> dest.size
				   >> dest.type
				   >> dest.dev
				   >> dest.qid
				   >> dest.mode
				   >> dest.atime
				   >> dest.mtime
				   >> dest.length
				   >> dest.name
				   >> dest.uid
				   >> dest.gid
				   >> dest.muid;
}


/**
 * @brief Create Version response
 * @param version Protocol version that server can support.
 * @param maxMessageSize Maximum message size in bytes that a server can support.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Version const& response);

/**
 * @brief Create Auth response
 * @param qid Qid of the file to be used for authentication.
 * @return Typed Message builder for fluent interface.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Auth const& response);

/**
 * @brief Create error response.
 * @param message Error message to communicate back to the client.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Error const& response);


/**
 * @brief Create Flush response.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Flush const& response);

/**
 * @brief Create attach response.
 * @param qid Qid of the filesystem a client attached to.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Attach const& response);

/**
 * @brief Create walk response.
 * @param qids An array of qids that a server walked through.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Walk const& response);

/**
 * @brief Create open response.
 * @param qid Qid of the opened file.
 * @param iounit Hint for the optimal read/write size.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Open const& response);

/**
 * @brief Create Create response.
 * @param qid Qid of the created file.
 * @param iounit Hint for the optimal read/write size.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Create const& response);

/**
 * @brief Create Read file response.
 * @param data Data read from the file to be sent back to the client.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Read const& response);

/**
 * @brief Create Write file response.
 * @param iounit Number of bytes written.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Write const& response);

/**
 * @brief Create Clunk response.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Clunk const& response);

/**
 * @brief Create Remove response.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Remove const& response);

/**
 * @brief Create stat response.
 * @param value Stat data read about the file.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::Stat const& response);

/**
 * @brief Create Write Stats.
 * @return Message builder.
 */
ResponseWriter& operator<< (ResponseWriter& writer, Response::WStat const& response);


/**
 * Create a version request.
 * @param version Suggested protocol version.
 * @param maxMessageSize Suggest maximum size of the protocol message, including mandatory message header.
 * @return Ref to this for fluent interface.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Version const& request);

/**
 * @brief Create Auth request.
 * @param afid User provided fid to be used for auth operations.
 * @param userName User name to authenticate as.
 * @param attachName Name of the filesystem to attach / authenticate to.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Auth const& request);

/**
 * @brief Create a Flush request.
 * @param oldTransation ID of the transaction to flush.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Flush const& request);

/**
 * @brief Create Attach request
 * @param fid User provided fid to be assosiated with attached filesystem.
 * @param afid Authentication fid used during auth phase.
 * @param userName User name the use used to authenticate with.
 * @param attachName Name of the attachment / fs a user has authenticated to.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Attach const& request);

/**
 * @brief Create Open file request.
 * @param fid User provided fid to be assosiated with the opened file.
 * @param mode File open mode. @see OpenMode for details.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Open const& request);

/** Create file Create request.
 * @param fid User provided fid assosiated with the directory where file to be created.
 * @param name Name of the file to create.
 * @param permissions Permissions of the newly created file.
 * @param mode File open mode. @see OpenMode for details.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Create const& request);

/**
 * @brief Create Read request
 * @param fid User provided fid assosiated with a file to read from.
 * @param offset Offset from the start of the file to read from.
 * @param count Number of bytes to read from the file.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Read const& request);

/**
 * @brief Create Write request.
 * @param fid User provided fid assosiated with a file to write to.
 * @param offset Offset from the start of the file to read from.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Write const& request);


/**
 * @brief Create Clunk request.
 * @param fid User provided fid to be forgotten by the server.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Clunk const& request);

/**
 * @brief Create Remove file request.
 * @param fid User provided fid assosiated with a file to be removed.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Remove const& response);

/**
 * @brief Create stat request.
 * @param fid User provided fid assosiated with a file to query stats for.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Stat const& request);

/**
 * @brief Cretea WriteStat request.
 * @param fid User provided fid assosiated with a file to write stats for.
 * @param stat File stats to be written.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::WStat const& request);

/**
 * @brief Create Walk request.
 * @param fid User provided fid assosiated with a file to start the walk from.
 * @param nfid User provided fid to be assosiated with the file resulting from the walk.
 * @return Message builder.
 */
RequestWriter& operator<< (RequestWriter& writer, Request::Walk const& request);


/**
 * Create partial Walk request.
 * @return partial writer.
 */
PathWriter operator<< (RequestWriter& writer, Request::Partial::Walk const& request);

/**
 * Create partial Write request.
 * @return partial writer.
 */
DataWriter operator<< (RequestWriter& writer, Request::Partial::Write const& request);


/**
 * Create partial Read response.
 * @return Message builder.
 */
DataWriter operator<< (ResponseWriter& writer, Response::Partial::Read const& response);

PartialStringWriter operator<< (ResponseWriter& writer, Response::Partial::Error const& response);


Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Version& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Auth& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Flush& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Attach& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Walk& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Open& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Create& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Read& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Write& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Clunk& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Remove& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::Stat& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Request::WStat& dest);

Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Version& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Auth& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Attach& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Error& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Flush& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Walk& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Open& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Create& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Read& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Write& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Clunk& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Remove& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::Stat& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, Response::WStat& dest);


inline constexpr Solace::byte asByte(MessageType type) noexcept {
	return static_cast<Solace::byte>(type);
}

template <>
constexpr Solace::byte messageCodeOf<Request::Version>() noexcept { return asByte(MessageType::TVersion); }
template <>
constexpr Solace::byte messageCodeOf<Request::Auth>() noexcept { return asByte(MessageType::TAuth); }
template <>
constexpr Solace::byte messageCodeOf<Request::Flush>() noexcept { return asByte(MessageType::TFlush); }
template <>
constexpr Solace::byte messageCodeOf<Request::Attach>() noexcept { return asByte(MessageType::TAttach); }
template <>
constexpr Solace::byte messageCodeOf<Request::Walk>() noexcept { return asByte(MessageType::TWalk); }
template <>
constexpr Solace::byte messageCodeOf<Request::Open>() noexcept { return asByte(MessageType::TOpen); }
template <>
constexpr Solace::byte messageCodeOf<Request::Create>() noexcept { return asByte(MessageType::TCreate); }
template <>
constexpr Solace::byte messageCodeOf<Request::Read>() noexcept { return asByte(MessageType::TRead); }
template <>
constexpr Solace::byte messageCodeOf<Request::Write>() noexcept { return asByte(MessageType::TWrite); }
template <>
constexpr Solace::byte messageCodeOf<Request::Clunk>() noexcept { return asByte(MessageType::TClunk); }
template <>
constexpr Solace::byte messageCodeOf<Request::Remove>() noexcept { return asByte(MessageType::TRemove); }
template <>
constexpr Solace::byte messageCodeOf<Request::Stat>() noexcept { return asByte(MessageType::TStat); }
template <>
constexpr Solace::byte messageCodeOf<Request::WStat>() noexcept { return asByte(MessageType::TWStat); }

template <>
constexpr Solace::byte messageCodeOf<Response::Version>() noexcept { return asByte(MessageType::RVersion); }
template <>
constexpr Solace::byte messageCodeOf<Response::Auth>() noexcept { return asByte(MessageType::RAuth); }
template <>
constexpr Solace::byte messageCodeOf<Response::Attach>() noexcept { return asByte(MessageType::RAttach); }
template <>
constexpr Solace::byte messageCodeOf<Response::Error>() noexcept { return asByte(MessageType::RError); }
template <>
constexpr Solace::byte messageCodeOf<Response::Flush>() noexcept { return asByte(MessageType::RFlush); }
template <>
constexpr Solace::byte messageCodeOf<Response::Walk>() noexcept { return asByte(MessageType::RWalk); }
template <>
constexpr Solace::byte messageCodeOf<Response::Open>() noexcept { return asByte(MessageType::ROpen); }
template <>
constexpr Solace::byte messageCodeOf<Response::Create>() noexcept { return asByte(MessageType::RCreate); }
template <>
constexpr Solace::byte messageCodeOf<Response::Read>() noexcept { return asByte(MessageType::RRead); }
template <>
constexpr Solace::byte messageCodeOf<Response::Write>() noexcept { return asByte(MessageType::RWrite); }
template <>
constexpr Solace::byte messageCodeOf<Response::Clunk>() noexcept { return asByte(MessageType::RClunk); }
template <>
constexpr Solace::byte messageCodeOf<Response::Remove>() noexcept { return asByte(MessageType::RRemove); }
template <>
constexpr Solace::byte messageCodeOf<Response::Stat>() noexcept { return asByte(MessageType::RStat); }
template <>
constexpr Solace::byte messageCodeOf<Response::WStat>() noexcept { return asByte(MessageType::RWStat); }


/**
 * Get a string representation of the message name given the op-code.
 * @param messageType Message op-code to convert to a string.
 * @return A string representation of a given message code.
 */
Solace::StringView
messageTypeToString(Solace::byte type) noexcept;

}  // end of namespace styxe
#endif  // STYXE_9P2000_HPP
