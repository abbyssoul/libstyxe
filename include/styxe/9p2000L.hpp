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
#ifndef STYXE_9P2000L_HPP
#define STYXE_9P2000L_HPP


#include "styxe/9p2000.hpp"


namespace styxe {
namespace _9P2000L {

/// Protocol version literal
extern const Solace::StringLiteral kProtocolVersion;


/**
 * 9P2000.L messages
 */
enum class MessageType : Solace::byte {
	Tlerror = 6,
	Rlerror,
	Tstatfs = 8,
	Rstatfs,
	Tlopen = 12,
	Rlopen,
	Tlcreate = 14,
	Rlcreate,
	Tsymlink = 16,
	Rsymlink,
	Tmknod = 18,
	Rmknod,
	Trename = 20,
	Rrename,
	Treadlink = 22,
	Rreadlink,
	Tgetattr = 24,
	Rgetattr,
	Tsetattr = 26,
	Rsetattr,
	Txattrwalk = 30,
	Rxattrwalk,
	Txattrcreate = 32,
	Rxattrcreate,
	Treaddir = 40,
	Rreaddir,
	Tfsync = 50,
	Rfsync,
	Tlock = 52,
	Rlock,
	Tgetlock = 54,
	Rgetlock,
	Tlink = 60,
	Rlink,
	Tmkdir = 72,
	Rmkdir,
	Trenameat = 74,
	Rrenameat,
	Tunlinkat = 76,
	Runlinkat,
};



/// Server directory entry as returned by ReadDir
struct DirEntry {
	Qid qid;						//!< Qid of the file
	Solace::uint64 offset;			//!< Offset of this entry in the stream
	Solace::byte type;				//!< type of the file
	Solace::StringView name;		//!< File entry name
};


inline
bool operator== (DirEntry const& lhs, DirEntry const& rhs) noexcept {
	return (lhs.qid == rhs.qid && lhs.offset == rhs.offset && lhs.type == rhs.type && lhs.name == rhs.name);
}

inline
bool operator!= (DirEntry const& lhs, DirEntry const& rhs) noexcept {
	return !operator==(lhs, rhs);
}

/// Bitmask values to be used for GetAttr request and response. @see Response::GetAttr
struct AttributesMask {
	static constexpr Solace::uint64 const GET_MODE = 0x00000001ULL;  //!< Bitmask for `mode.
	static constexpr Solace::uint64 const GET_NLINK = 0x00000002ULL;  //!<  Bitmask for `nlinks` attribute.
	static constexpr Solace::uint64 const GET_UID = 0x00000004ULL;  //!< Bitmask for `uid` attribute.
	static constexpr Solace::uint64 const GET_GID = 0x00000008ULL;  //!< Bitmask for `gid` attribute.
	static constexpr Solace::uint64 const GET_RDEV = 0x00000010ULL;  //!< Bitmask for `rdev` attribute.
	static constexpr Solace::uint64 const GET_ATIME = 0x00000020ULL;  //!< Bitmask for `atime` attribute.
	static constexpr Solace::uint64 const GET_MTIME = 0x00000040ULL;  //!< Bitmask for `mtime` attribute.
	static constexpr Solace::uint64 const GET_CTIME = 0x00000080ULL;  //!< Bitmask for `ctime` attribute.
	static constexpr Solace::uint64 const GET_INO = 0x00000100ULL;  //!< Bitmask for `ino` attribute.
	static constexpr Solace::uint64 const GET_SIZE = 0x00000200ULL;  //!< Bitmask for `size` attribute.
	static constexpr Solace::uint64 const GET_BLOCKS = 0x00000400ULL;  //!< Bitmask for `blocks` attribute.

	static constexpr Solace::uint64 const GET_BTIME = 0x00000800ULL;  //!< Bitmask for `btime` attribute.
	static constexpr Solace::uint64 const GET_GEN = 0x00001000ULL;  //!< Bitmask for `gen` attribute.
	static constexpr Solace::uint64 const GET_DATA_VERSION = 0x00002000ULL;  //!< Bitmask for `data_verison` attribute.

	static constexpr Solace::uint64 const GET_BASIC = 0x000007ffULL;  //!< Mask for fields up to BLOCKS
	static constexpr Solace::uint64 const GET_ALL = 0x00003fffULL;  //!< Mask for All fields above
};

/// Bit mask of attributes to be used for SetAttr message request
struct SetAttributesMaks {
	static constexpr Solace::uint32 const MODE = 0x00000001UL;  //!< Bitmask for `mode`. @see Request::SetAttr::mode
	static constexpr Solace::uint32 const UID = 0x00000002UL;   //!< Bitmask for `uid`. @see Request::SetAttr::uid
	static constexpr Solace::uint32 const GID = 0x00000004UL;   //!< Bitmask for `gid`. @see Request::SetAttr::gid
	static constexpr Solace::uint32 const SIZE = 0x00000008UL;  //!< Bitmask for `size`. @see Request::SetAttr::size
	static constexpr Solace::uint32 const ATIME = 0x00000010UL;  //!< Bitmask for `atime`. @see Request::SetAttr::atime
	static constexpr Solace::uint32 const MTIME = 0x00000020UL;  //!< Bitmask for `mtime`. @see Request::SetAttr::mtime
	static constexpr Solace::uint32 const CTIME = 0x00000040UL;  //!< Bitmask for `ctime`. @see Request::SetAttr::ctime
	static constexpr Solace::uint32 const ATIME_SET = 0x00000080UL;  //!< Bitmask for `atime`. Use provided. value.
	static constexpr Solace::uint32 const MTIME_SET = 0x00000100UL;  //!< Bitmask for `mtime`. Use provided. value.
};


/// Type of lock operation for Lock request.
struct LockType {
	static constexpr Solace::byte const ReadLock = 0;		//!< Aquire Read-lock
	static constexpr Solace::byte const WriteLock = 1;		//!< Aquire Write-lock
	static constexpr Solace::byte const Unlock = 2;			//!< Unlock existing lock
};

/// Lock flag bits
struct LockFlags {
	static constexpr Solace::uint32 const BLOCK = 1;		//!< blocking request
	static constexpr Solace::uint32 const RECLAIM = 2;		//!< reserved for future use
};

/// Lock status types:
struct LockStatus {
	static constexpr Solace::byte const SUCCESS = 0;		//!< Lock operation has been successful
	static constexpr Solace::byte const BLOCKED = 1;		//!< Lock operaiton has been blocked
	static constexpr Solace::byte const ERROR = 2;			//!< Lock operation has failed
	static constexpr Solace::byte const GRACE = 3;			//!< ????
};

/// 9P2000.L requests
struct Request {

	/// Get file system information
	struct StatFS {
		Fid fid;  //!< Fid contained be the FS the stats are qeeried for.
	};

	/// Open a file
	struct LOpen {
		Fid				fid;     //!< Client provided Fid to represent the newly opened file.
		Solace::uint32	flags;   //!< File open-flags
	};


	/// Create regular file
	struct LCreate {
		Fid                 fid;		//!< Fid of the directory where the file should be created.
		Solace::StringView  name;		//!< Name of the file to be created.
		Solace::uint32      flags;		//!< Permissions to the newly created file.
		Solace::uint32      mode;		//!< Linux creat(2) mode bits.
		Solace::uint32      gid;		//!< effective gid of the caller.
	};

	/// Create symbolic link response
	struct Symlink {
		Fid                 fid;		//!< Fid of the directory where the file should be created.
		Solace::StringView  name;		//!< Name of the file to be created.
		Solace::StringView  symtgt;		//!< Name of the symlink target
		Solace::uint32      gid;		//!< effective gid of the caller.
	};

	/// Create a device node
	struct MkNode {
		Fid                 dfid;		//!< Fid of the directory where the file should be created.
		Solace::StringView  name;		//!< Name of the file to be created.
		Solace::uint32      mode;		//!< The mode the file will be opened in.
		Solace::uint32      major;		//!< Node's major number
		Solace::uint32      minor;		//!< Node's minor number
		Solace::uint32      gid;		//!< Effective gid of the caller.
	};

	/// Rename a file
	struct Rename {
		Fid fid;						//!< Fid of the file to rename
		Fid dfid;						//!< Fid of the directory where the new file will reside.
		Solace::StringView name;		//!< New name for the file
	};

	/// Read value of symbolic link
	struct ReadLink {
		Fid fid;		//!< Fid of the link file to read value of.
	};

	/// Get file attributes
	struct GetAttr {
		Fid fid;					  //!< Fid of the file system object to get attributes of.
		Solace::uint64 request_mask;  //!< Bitmask indicating which fields are requested. @see AttributesMask
	};

	/// Set file attributes
	struct SetAttr {
		Fid fid;						//!< Fid of the file system object to set attributes for.
		Solace::uint32 valid;			//!< Bit mask of the attributes to set. @see SetAttributesMaks
		Solace::uint32 mode;			//!< protection
		Solace::uint32 uid;				//!< user ID of owner
		Solace::uint32 gid;				//!< group ID of owner
		Solace::uint64 size;			//!< total size, in bytes
		Solace::uint64 atime_sec;		//!< time of last access
		Solace::uint64 atime_nsec;		//!< time of last access - nano-second portion
		Solace::uint64 mtime_sec;		//!< time of last modification
		Solace::uint64 mtime_nsec;		//!< time of last modification - nano-second portion
	};

	/// Prepare to read/list extended attributes
	struct XAttrWalk {
		Fid fid;						//!< Fid of a file system object to read attributes of.
		Fid newfid;						//!< Fid pointing to xattr name used to read the xattr value.
		Solace::StringView name;		//!< Name of the attribute or null to get a list of attributes.
	};

	/// Prepare to set extended attribute
	struct XAttrCreate {
		Fid fid;						//!< Fid pointing to the xattr name to be used to set the xattr value.
		Solace::StringView name;		//!< Name of the attribute.
		Solace::uint64 attr_size;		//!< Data size of the attribute.
		Solace::uint32 flags;			//!< derived from set Linux setxattr.
	};

	/// Read a list of directory entries
	struct ReadDir {
		Fid fid;					//!< Fid of the directory, previously opened with lopen, to read list from
		Solace::uint64 offset;		//!< Offset in to the directory stream. Opaqu value, zero on the first call.
		Solace::uint32 count;		//!< Most number of bytes to be returned in data.
	};

	/// Tell the server to flush any cached data associated with a fid, previously opened with lopen.
	struct FSync {
		Fid fid;  //!< Fid of the file, previously opened with lopen, to flush IO operations for.
	};

	/// Acquire or release a POSIX record lock
	struct Lock {
		Fid fid;						//!< Fid of a filesystem object to aquire / release lock on.
		Solace::byte type;				//!< Type of the lock. @see LockType
		Solace::uint32 flags;			//!< Extra flags. @see LockFlags
		Solace::uint64 start;			//!< Starting offset for lock
		Solace::uint64 length;			//!< Number of bytes to lock
		Solace::uint32 proc_id;			//!< PID of process blocking our lock (F_GETLK only)
		Solace::StringView client_id;   //!< Id uniquely identifying the lock requester
	};

	/// Test for the existence of a POSIX record lock
	struct GetLock {
		Fid fid;							//!< Fid of a filesystem object to test for lock.
		Solace::byte type;					//!< Type of the lock. @see LockType
		Solace::uint64 start;				//!< Starting offset for lock
		Solace::uint64 length;				//!< Number of bytes to lock
		Solace::uint32 proc_id;				//!< PID of process blocking our lock (F_GETLK only)
		Solace::StringView client_id;		//!< Id uniquely identifying the lock requester
	};

	/// create hard link
	struct Link {
		Fid dfid;						//!< Directory Fid to create link at.
		Fid fid;						//!< link target
		Solace::StringView name;		//!< Name of the target link
	};

	/// Create directory
	struct MkDir {
		Fid dfid;						//!< Fid of a directory to create a new directory in.
		Solace::StringView name;		//!< Name of a new directory.
		Solace::uint32 mode;			//!< Linux mkdir(2) mode bits.
		Solace::uint32 gid;				//!< Effective group ID of the caller.
	};

	/// Rename a file or directory
	struct RenameAt {
		Fid olddirfid;					//!< Fid of a directory where file to be renamed, resides
		Solace::StringView oldname;		//!< Old name of the file to be renamed.

		Fid newdirfid;					//!< Fid of a directory where the file should reside after renaming.
		Solace::StringView newname;		//!< New name of the file.
	};

	/// Unlink a file or directory
	struct UnlinkAt {
		Fid dfid;						//!< Fid of a directory to unlink a new from.
		Solace::StringView name;		//!< Name to be unlinked from a directory
		Solace::uint32 flags;			//!< Extra flags
	};

};


/// 9P2000.L responses
struct Response {
	/// Error resoponse from a server
	struct LError {
		Solace::uint32		ecode;  //!< Error code
	};

	/// Get file system information response
	struct StatFS {
		Solace::uint32 type;	 //!< type of file system
		Solace::uint32 bsize;	 //!< optimal transfer block size
		Solace::uint64 blocks;	 //!< total data blocks in file system
		Solace::uint64 bfree;	 //!< free blocks in fs
		Solace::uint64 bavail;	 //!< free blocks avail to non-superuser
		Solace::uint64 files;	 //!< total file nodes in file system
		Solace::uint64 ffree;	 //!< free file nodes in fs
		Solace::uint64 fsid;	 //!< file system id
		Solace::uint32 namelen;  //!< maximum length of filenames
	};

	/// Open file response
	struct LOpen : public ::styxe::Response::Open {
	};

	/// Create file response
	struct LCreate : public ::styxe::Response::Create {
	};

	/// Create symbolic link response
	struct Symlink {
		Qid				qid;			//!< Qid of the new symbolic link file
	};

	/// Create a device node response
	struct MkNode {
		Qid				qid;			//!< Qid of the new symbolic link file
	};

	/// Rename a file response
	struct Rename {};

	/// Read value of symbolic link response
	struct ReadLink {
		Solace::StringView target;		//!< Contents of the symbolic link requested
	};

	/// Get file attributes response
	struct GetAttr {
		Qid qid;						//!< qid of the attributes object
		Solace::uint64 valid;			//!< Bitmask indicating which fields are valid. @see AttributesMask
		Solace::uint32 mode;			//!< protection
		Solace::uint32 uid;				//!< user ID of owner
		Solace::uint32 gid;				//!< group ID of owner
		Solace::uint64 size;			//!< total size, in bytes

		Solace::uint64 atime_sec;		//!< time of last access
		Solace::uint64 atime_nsec;		//!< time of last access - nano-second portion
		Solace::uint64 mtime_sec;		//!< time of last modification
		Solace::uint64 mtime_nsec;		//!< time of last modification - nano-second portion

		Solace::uint64 ctime_sec;		//!< time of last status change
		Solace::uint64 ctime_nsec;		//!< time of last status change - nano-second portion

		Solace::uint64 nlink;			//!< number of hard links
		Solace::uint64 rdev;			//!< device ID (if special file)
		Solace::uint64 blksize;			//!< blocksize for file system I/O
		Solace::uint64 blocks;			//!< number of 512B blocks allocated

		Solace::uint64 btime_sec;		//!< reserved for future use
		Solace::uint64 btime_nsec;		//!< reserved for future use
		Solace::uint64 gen;				//!< reserved for future use
		Solace::uint64 data_version;    //!< reserved for future use
	};

	/// Set file attributes response
	struct SetAttr { };

	/// prepare to read/list extended attributes response
	struct XAttrWalk {
		Solace::uint64 size;  //!< A field of undocumented utility
	};

	/// Prepare to set extended attribute response
	struct XAttrCreate { };

	/// Read a directory response
	struct ReadDir {
		Solace::MemoryView data;  //!< Data buffer encoding directory entries. @see DirEntry
	};

	/// Flush any cached data to disk
	struct FSync { };

	/// Acquire or release a POSIX record lock response
	struct Lock {
		Solace::byte status;  //!< Status code of the lock operation. @see LockStatus for the list of valid codes
	};

	/// Test for the existence of a POSIX record lock response
	struct GetLock {
		Solace::byte type;					//!< Type of the lock. @see LockType
		Solace::uint64 start;				//!< Starting offset for lock
		Solace::uint64 length;				//!< Number of bytes to lock
		Solace::uint32 proc_id;				//!< PID of process blocking our lock (F_GETLK only)
		Solace::StringView client_id;		//!< Id uniquely identifying the lock requester
	};

	/// create hard link response
	struct Link {};

	/// Create directory
	struct MkDir {
		Qid qid;		//!< Qid of the new directory
	};

	/// Rename a file or directory response
	struct RenameAt {};

	/// Unlink a file or directory response
	struct UnlinkAt {};

};



/**
 * Get a string representation of the message name given the op-code.
 * @param messageType Message op-code to convert to a string.
 * @return A string representation of a given message code.
 */
Solace::StringView
messageTypeToString(Solace::byte type) noexcept;

/**
 * DirEntryReader - helper class to read @see DirEntry.
 * @see _9P2000L::Response::ReadDir
 */
struct DirEntryReader {
	using value_type = DirEntry;  //!< Type alias for iterator valu_type.
	using reference = DirEntry&;  //!< Type alias for iterator value reference type.
	using const_reference = DirEntry const&;  //!< Type alias for iterator const_reference type.

	/**
	 * Iterator that reads DirEntry from a byte buffer..
	 */
	struct Iterator {

		/**
		 * Construct a new DirEntry reading interator.
		 * @param buffer Memory buffer to read entries from.
		 * @param offset Offset into the buffer to start reading from.
		 */
		Iterator(Solace::MemoryView buffer, Solace::ByteReader::size_type offset) noexcept
			: _reader{buffer}
			, _offset{offset}
		{
			_reader.position(_offset);
			if (!_reader.hasRemaining())
				return;

			if (!read()) {
				_offset = _reader.limit();
				_reader.position(buffer.size());
			}
		}

		/**
		 * Try to read an entry from the buffer at the current read position.
		 * @return Result of the read attempt: Nothing on success or an error.
		 */
		Solace::Result<void, Error> read();

		reference operator* () { return _value; }
		const_reference operator* () const { return _value; }

		Iterator& operator++ () {
			_offset = _reader.position();
			if (!read()) {
				_offset = _reader.limit();
				_reader.position(_reader.limit());
			}

			return *this;
		}

		Iterator& swap(Iterator& rhs) noexcept {
			std::swap(_reader, rhs._reader);
			std::swap(_offset, rhs._offset);
			std::swap(_value, rhs._value);

			return *this;
		}

		Iterator& operator= (Iterator&& rhs) noexcept {
			return swap(rhs);
		}

		constexpr bool operator!= (Iterator const& other) const noexcept {
			return ((_offset != other._offset) ||
					(_reader.position() != other._reader.position()));
		}

		constexpr bool operator== (Iterator const& other) const noexcept {
			return !(operator!= (other));
		}

	  private:
		Solace::ByteReader				_reader;
		Solace::ByteReader::size_type	_offset;
		DirEntry						_value;
	};

	/**
	 * Construct a new DirEntryReader
	 * @param buffer A buffer to read entries from.
	 */
	constexpr DirEntryReader(Solace::MemoryView buffer) noexcept
		: _buffer{buffer}
	{}

	auto constexpr buffer() noexcept { return _buffer; }

private:

	Solace::MemoryView _buffer;  //!< Memory buffer to read DirEntries from
};

/**
 * Get begin iterator
 * @param reader Reader to get being interator from
 * @return Iterator pointing to the firs element in the sequence, if any.
 */
inline DirEntryReader::Iterator
begin(DirEntryReader& reader) noexcept { return {reader.buffer(), 0}; }

/**
 * Get an end iterator
 * @param reader Reader to get end interator from
 * @return Iterator pointing to the one value after the sequence.
 */
inline DirEntryReader::Iterator
end(DirEntryReader& reader) noexcept { return {reader.buffer(), reader.buffer().size()}; }

}  // end of namespace _9P2000L


inline
Encoder& operator<< (Encoder& encoder, _9P2000L::DirEntry const& value) {
	return encoder << value.qid
				   << value.offset
				   << value.type
				   << value.name;
}

inline
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, _9P2000L::DirEntry& value) {
	return decoder >> value.qid
				   >> value.offset
				   >> value.type
				   >> value.name;
}


RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::StatFS const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::LOpen const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::LCreate const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::Symlink const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::MkNode const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::Rename const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::ReadLink const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::GetAttr const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::SetAttr const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::XAttrWalk const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::XAttrCreate const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::ReadDir const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::FSync const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::Lock const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::GetLock const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::Link const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::MkDir const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::RenameAt const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::UnlinkAt const& dest);

ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::LError const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::StatFS const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::LOpen const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::LCreate const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::Symlink const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::MkNode const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::Rename const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::ReadLink const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::GetAttr const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::SetAttr const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::XAttrWalk const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::XAttrCreate const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::ReadDir const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::FSync const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::Lock const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::GetLock const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::Link const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::MkDir const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::RenameAt const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::UnlinkAt const& dest);



Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::StatFS& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::LOpen& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::LCreate& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::Symlink& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::MkNode& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::Rename& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::ReadLink& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::GetAttr& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::SetAttr& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::XAttrWalk& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::XAttrCreate& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::ReadDir& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::FSync& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::Lock& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::GetLock& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::Link& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::MkDir& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::RenameAt& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::UnlinkAt& dest);

Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::LError& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::StatFS& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::LOpen& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::LCreate& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::Symlink& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::MkNode& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::Rename& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::ReadLink& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::GetAttr& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::SetAttr& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::XAttrWalk& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::XAttrCreate& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::ReadDir& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::FSync& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::Lock& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::GetLock& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::Link& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::MkDir& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::RenameAt& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::UnlinkAt& dest);



inline constexpr auto asByte(_9P2000L::MessageType type) noexcept {
	return static_cast<Solace::byte>(type);
}


template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::StatFS>() noexcept { return asByte(_9P2000L::MessageType::Tstatfs); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::LOpen>() noexcept { return asByte(_9P2000L::MessageType::Tlopen); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::LCreate>() noexcept { return asByte(_9P2000L::MessageType::Tlcreate); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::Symlink>() noexcept { return asByte(_9P2000L::MessageType::Tsymlink); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::MkNode>() noexcept { return asByte(_9P2000L::MessageType::Tmknod); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::Rename>() noexcept { return asByte(_9P2000L::MessageType::Trename); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::ReadLink>() noexcept { return asByte(_9P2000L::MessageType::Treadlink); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::GetAttr>() noexcept { return asByte(_9P2000L::MessageType::Tgetattr); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::SetAttr>() noexcept { return asByte(_9P2000L::MessageType::Tsetattr); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::XAttrWalk>() noexcept { return asByte(_9P2000L::MessageType::Txattrwalk); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::XAttrCreate>() noexcept { return asByte(_9P2000L::MessageType::Txattrcreate); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::ReadDir>() noexcept { return asByte(_9P2000L::MessageType::Treaddir); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::FSync>() noexcept { return asByte(_9P2000L::MessageType::Tfsync ); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::Lock>() noexcept { return asByte(_9P2000L::MessageType::Tlock); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::GetLock>() noexcept { return asByte(_9P2000L::MessageType::Tgetlock); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::Link>() noexcept { return asByte(_9P2000L::MessageType::Tlink); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::MkDir>() noexcept { return asByte(_9P2000L::MessageType::Tmkdir); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::RenameAt>() noexcept { return asByte(_9P2000L::MessageType::Trenameat); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Request::UnlinkAt>() noexcept { return asByte(_9P2000L::MessageType::Tunlinkat); }


template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::LError>() noexcept { return asByte(_9P2000L::MessageType::Rlerror); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::StatFS>() noexcept { return asByte(_9P2000L::MessageType::Rstatfs); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::LOpen>() noexcept { return asByte(_9P2000L::MessageType::Rlopen); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::LCreate>() noexcept { return asByte(_9P2000L::MessageType::Rlcreate); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::Symlink>() noexcept { return asByte(_9P2000L::MessageType::Rsymlink); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::MkNode>() noexcept { return asByte(_9P2000L::MessageType::Rmknod); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::Rename>() noexcept { return asByte(_9P2000L::MessageType::Rrename); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::ReadLink>() noexcept { return asByte(_9P2000L::MessageType::Rreadlink); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::GetAttr>() noexcept { return asByte(_9P2000L::MessageType::Rgetattr); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::SetAttr>() noexcept { return asByte(_9P2000L::MessageType::Rsetattr); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::XAttrWalk>() noexcept { return asByte(_9P2000L::MessageType::Rxattrwalk); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::XAttrCreate>() noexcept { return asByte(_9P2000L::MessageType::Rxattrcreate); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::ReadDir>() noexcept { return asByte(_9P2000L::MessageType::Rreaddir); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::FSync>() noexcept { return asByte(_9P2000L::MessageType::Rfsync ); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::Lock>() noexcept { return asByte(_9P2000L::MessageType::Rlock); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::GetLock>() noexcept { return asByte(_9P2000L::MessageType::Rgetlock); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::Link>() noexcept { return asByte(_9P2000L::MessageType::Rlink); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::MkDir>() noexcept { return asByte(_9P2000L::MessageType::Rmkdir); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::RenameAt>() noexcept { return asByte(_9P2000L::MessageType::Rrenameat); }
template <>
constexpr Solace::byte
messageCodeOf<_9P2000L::Response::UnlinkAt>() noexcept { return asByte(_9P2000L::MessageType::Runlinkat); }


}  // end of namespace styxe
#endif  // STYXE_9P2000L_HPP
