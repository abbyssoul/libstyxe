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

extern const Solace::uint32 kNonUNAME;

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


struct DirEntry {
	Qid qid;
	Solace::uint64 offset;
	Solace::byte type;
	Solace::StringView name;
};


//inline
//size_type protocolSize(DirEntry const& value) noexcept {
//	return protocolSize(value.qid)
//			+ protocolSize(value.offset)
//			+ protocolSize(value.type)
//			+ protocolSize(value.name);
//}


/// 9P2000.L requests
struct Request {

	/// Get file system information request
	struct StatFS {
		Fid fid;
	};

	/// Open a file
	struct Open /*: public ::styxe::Request::Open*/ {
		Fid				fid;     //!< Client provided Fid to represent the newly opened file.
		Solace::uint32	flags;   //!< File open-flags
	};


	/// Create regular file
	struct Create /*: public ::styxe::Request::Create*/ {
		Fid                 fid;    //!< Fid of the directory where the file should be created.
		Solace::StringView  name;   //!< Name of the file to be created.
		Solace::uint32      flags;   //!< Permissions to the newly created file.
		Solace::uint32      mode;   //!< The mode the file will be opened in.
		Solace::uint32      gid;
	};

	/// Create symbolic link response
	struct Symlink {
		Fid                 fid;    //!< Fid of the directory where the file should be created.
		Solace::StringView  name;   //!< Name of the file to be created.
		Solace::StringView  symtgt;
		Solace::uint32      gid;
	};

	/// Create a device node
	struct MkNode {
		Fid                 dfid;    //!< Fid of the directory where the file should be created.
		Solace::StringView  name;   //!< Name of the file to be created.
		Solace::uint32      mode;   //!< The mode the file will be opened in.
		Solace::uint32      major;
		Solace::uint32      minor;
		Solace::uint32      gid;
	};

	/// Rename a file
	struct Rename {
		Fid fid;
		Fid dfid;
		Solace::StringView name;
	};

	/// Read value of symbolic link
	struct ReadLink {
		Fid fid;
	};

	/// Get file attributes
	struct GetAttr {
		Fid fid;
		Solace::uint64 request_mask;
	};

	/// Set file attributes
	struct SetAttr {
		Fid fid;
		Solace::uint32 valid;		// FIXME 32 / 64?
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
		Fid fid;
		Fid newfid;
		Solace::StringView name;
	};

	/// Prepare to set extended attribute
	struct XAttrCreate {
		Fid fid;
		Solace::StringView name;
		Solace::uint64 attr_size;
		Solace::uint32 flags;
	};

	/// Read a directory
	struct ReadDir {
		Fid fid;
		Solace::uint64 offset;
		Solace::uint32 count;
	};

	/// Flush any cached data to disk
	struct FSync {
		Fid fid;
	};

	/// Acquire or release a POSIX record lock
	struct Lock {
		Fid fid;
		Solace::byte type;  //<! Type of lock: F_RDLCK, F_WRLCK, F_UNLCK
		Solace::uint32 flags;
		Solace::uint64 start;			//!< Starting offset for lock
		Solace::uint64 length;			//!< Number of bytes to lock
		Solace::uint32 proc_id;			//!< PID of process blocking our lock (F_GETLK only)
		Solace::StringView client_id;
	};

	/// Test for the existence of a POSIX record lock
	struct GetLock {
		Fid fid;
		Solace::byte type;
		Solace::uint64 start;
		Solace::uint64 length;
		Solace::uint32 proc_id;
		Solace::StringView client_id;
	};

	/// create hard link
	struct Link {
		Fid dfid;	  //!< Directory Fid to create link at.
		Fid fid;	 //!< link target
		Solace::StringView name;  //!< Name of the target link
	};

	/// Create directory
	struct MkDir {
		Fid dfid;
		Solace::StringView name;
		Solace::uint32 mode;  //!< Linux mkdir(2) mode bits.
		Solace::uint32 gid;	  //!< Effective group ID of the caller.
	};

	/// Rename a file or directory
	struct RenameAt {
		Fid olddirfid;
		Solace::StringView oldname;

		Fid newdirfid;
		Solace::StringView newname;
	};

	/// Unlink a file or directory
	struct UnlinkAt {
		Fid dfid;
		Solace::StringView name;
		Solace::uint32 flags;
	};

};


/// 9P2000.L responses
struct Response {
	/// Error resoponse from a server
	struct LError {
		Solace::uint32		ecode;  /// Error code
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
	struct Open : public ::styxe::Response::Open {
	};

	/// Create file response
	struct Create : public ::styxe::Response::Create {
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
		Solace::StringView target;
	};

	/// Get file attributes response
	struct GetAttr {
		Qid qid;
		Solace::uint64 valid;
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
		Solace::uint64 data_version;	//!< reserved for future use
	};

	/// Set file attributes response
	struct SetAttr { };

	/// prepare to read/list extended attributes response
	struct XAttrWalk {
		Solace::uint64 size;
	};

	/// Prepare to set extended attribute response
	struct XAttrCreate { };

	/// Read a directory response
	struct ReadDir {
		Solace::uint32 count;
		void* data; // FIXME: Var-sized data
	};

	/// Flush any cached data to disk
	struct FSync { };

	/// Acquire or release a POSIX record lock response
	struct Lock {
		Solace::byte status;
	};

	/// Test for the existence of a POSIX record lock response
	struct GetLock {
		Solace::byte type;
		Solace::uint64 start;
		Solace::uint64 length;
		Solace::uint32 proc_id;
		Solace::StringView client_id;
	};

	/// create hard link response
	struct Link {};

	/// Create directory
	struct MkDir {
		Qid qid;
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

}  // end of namespace _9P2000L



RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::StatFS const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::Open const& dest);
RequestWriter& operator<< (RequestWriter& writer, _9P2000L::Request::Create const& dest);
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
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::Open const& dest);
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000L::Response::Create const& dest);
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
operator>> (Solace::ByteReader& data, _9P2000L::Request::Open& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Request::Create& dest);
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
operator>> (Solace::ByteReader& data, _9P2000L::Response::Open& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000L::Response::Create& dest);
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



inline constexpr Solace::byte asByte(_9P2000L::MessageType type) noexcept {
	return static_cast<Solace::byte>(type);
}

inline auto messageCode(_9P2000L::Request::StatFS const& ) noexcept { return asByte(_9P2000L::MessageType::Tstatfs); }
inline auto messageCode(_9P2000L::Request::Open const& ) noexcept { return asByte(_9P2000L::MessageType::Tlopen); }
inline auto messageCode(_9P2000L::Request::Create const& ) noexcept { return asByte(_9P2000L::MessageType::Tlcreate); }
inline auto messageCode(_9P2000L::Request::Symlink const& ) noexcept { return asByte(_9P2000L::MessageType::Tsymlink); }
inline auto messageCode(_9P2000L::Request::MkNode const& ) noexcept { return asByte(_9P2000L::MessageType::Tmknod); }
inline auto messageCode(_9P2000L::Request::Rename const& ) noexcept { return asByte(_9P2000L::MessageType::Trename); }
inline auto messageCode(_9P2000L::Request::ReadLink const& ) noexcept { return asByte(_9P2000L::MessageType::Treadlink); }
inline auto messageCode(_9P2000L::Request::GetAttr const& ) noexcept { return asByte(_9P2000L::MessageType::Tgetattr); }
inline auto messageCode(_9P2000L::Request::SetAttr const& ) noexcept { return asByte(_9P2000L::MessageType::Tsetattr); }
inline auto messageCode(_9P2000L::Request::XAttrWalk const& ) noexcept { return asByte(_9P2000L::MessageType::Txattrwalk); }
inline auto messageCode(_9P2000L::Request::XAttrCreate const& ) noexcept { return asByte(_9P2000L::MessageType::Txattrcreate); }
inline auto messageCode(_9P2000L::Request::ReadDir const& ) noexcept { return asByte(_9P2000L::MessageType::Treaddir); }
inline auto messageCode(_9P2000L::Request::FSync const& ) noexcept { return asByte(_9P2000L::MessageType::Tfsync ); }
inline auto messageCode(_9P2000L::Request::Lock const& ) noexcept { return asByte(_9P2000L::MessageType::Tlock); }
inline auto messageCode(_9P2000L::Request::GetLock const& ) noexcept { return asByte(_9P2000L::MessageType::Tgetlock); }
inline auto messageCode(_9P2000L::Request::Link const& ) noexcept { return asByte(_9P2000L::MessageType::Tlink); }
inline auto messageCode(_9P2000L::Request::MkDir const& ) noexcept { return asByte(_9P2000L::MessageType::Tmkdir); }
inline auto messageCode(_9P2000L::Request::RenameAt const& ) noexcept { return asByte(_9P2000L::MessageType::Trenameat); }
inline auto messageCode(_9P2000L::Request::UnlinkAt const& ) noexcept { return asByte(_9P2000L::MessageType::Tunlinkat); }


inline auto messageCode(_9P2000L::Response::LError const& ) noexcept { return asByte(_9P2000L::MessageType::Rlerror); }
inline auto messageCode(_9P2000L::Response::StatFS const& ) noexcept { return asByte(_9P2000L::MessageType::Rstatfs); }
inline auto messageCode(_9P2000L::Response::Open const& ) noexcept { return asByte(_9P2000L::MessageType::Rlopen); }
inline auto messageCode(_9P2000L::Response::Create const& ) noexcept { return asByte(_9P2000L::MessageType::Rlcreate); }
inline auto messageCode(_9P2000L::Response::Symlink const& ) noexcept { return asByte(_9P2000L::MessageType::Rsymlink); }
inline auto messageCode(_9P2000L::Response::MkNode const& ) noexcept { return asByte(_9P2000L::MessageType::Rmknod); }
inline auto messageCode(_9P2000L::Response::Rename const& ) noexcept { return asByte(_9P2000L::MessageType::Rrename); }
inline auto messageCode(_9P2000L::Response::ReadLink const& ) noexcept { return asByte(_9P2000L::MessageType::Rreadlink); }
inline auto messageCode(_9P2000L::Response::GetAttr const& ) noexcept { return asByte(_9P2000L::MessageType::Rgetattr); }
inline auto messageCode(_9P2000L::Response::SetAttr const& ) noexcept { return asByte(_9P2000L::MessageType::Rsetattr); }
inline auto messageCode(_9P2000L::Response::XAttrWalk const& ) noexcept { return asByte(_9P2000L::MessageType::Rxattrwalk); }
inline auto messageCode(_9P2000L::Response::XAttrCreate const& ) noexcept { return asByte(_9P2000L::MessageType::Rxattrcreate); }
inline auto messageCode(_9P2000L::Response::ReadDir const& ) noexcept { return asByte(_9P2000L::MessageType::Rreaddir); }
inline auto messageCode(_9P2000L::Response::FSync const& ) noexcept { return asByte(_9P2000L::MessageType::Rfsync ); }
inline auto messageCode(_9P2000L::Response::Lock const& ) noexcept { return asByte(_9P2000L::MessageType::Rlock); }
inline auto messageCode(_9P2000L::Response::GetLock const& ) noexcept { return asByte(_9P2000L::MessageType::Rgetlock); }
inline auto messageCode(_9P2000L::Response::Link const& ) noexcept { return asByte(_9P2000L::MessageType::Rlink); }
inline auto messageCode(_9P2000L::Response::MkDir const& ) noexcept { return asByte(_9P2000L::MessageType::Rmkdir); }
inline auto messageCode(_9P2000L::Response::RenameAt const& ) noexcept { return asByte(_9P2000L::MessageType::Rrenameat); }
inline auto messageCode(_9P2000L::Response::UnlinkAt const& ) noexcept { return asByte(_9P2000L::MessageType::Runlinkat); }


}  // end of namespace styxe
#endif  // STYXE_9P2000L_HPP
