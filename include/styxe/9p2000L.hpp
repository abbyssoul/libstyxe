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

extern const Solace::uint32 kNONUNAME;

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


struct StatFS {
	Solace::uint32	f_type;     /* type of file system (see below) */
	Solace::uint32	f_bsize;    /* optimal transfer block size */
	Solace::uint64	f_blocks;   /* total data blocks in file system */
	Solace::uint64	f_bfree;    /* free blocks in fs */
	Solace::uint64	f_bavail;   /* free blocks avail to non-superuser */
	Solace::uint64	f_files;    /* total file nodes in file system */
	Solace::uint64	f_ffree;    /* free file nodes in fs */
	Solace::uint64	f_fsid;     /* file system id */
	Solace::uint32	f_namelen;  /* maximum length of filenames */
};


struct Stat {
	dev_t     st_dev;     /* ID of device containing file */
	ino_t     st_ino;     /* inode number */
	mode_t    st_mode;    /* protection */
	nlink_t   st_nlink;   /* number of hard links */
	uid_t     st_uid;     /* user ID of owner */
	gid_t     st_gid;     /* group ID of owner */
	dev_t     st_rdev;    /* device ID (if special file) */
	off_t     st_size;    /* total size, in bytes */
	blksize_t st_blksize; /* blocksize for file system I/O */
	blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
	time_t    st_atime;   /* time of last access */
	time_t    st_mtime;   /* time of last modification */
	time_t    st_ctime;   /* time of last status change */
};


struct DirEntry {
	Qid qid;
	Solace::uint64 offset;
	Solace::byte type;
	Solace::StringView name;
};

struct flock {
	Solace::uint16 l_type;  /* Type of lock: F_RDLCK, F_WRLCK, F_UNLCK */
	Solace::uint16 l_whence;/* How to interpret l_start: SEEK_SET, SEEK_CUR, SEEK_END */
	off_t l_start; /* Starting offset for lock */
	off_t l_len;   /* Number of bytes to lock */
	pid_t l_pid;   /* PID of process blocking our lock (F_GETLK only) */
};

/**
 * Get a string representation of the message name given the op-code.
 * @param messageType Message op-code to convert to a string.
 * @return A string representation of a given message code.
 */
Solace::StringView
messageTypeToString(Solace::byte type) noexcept;

}  // end of namespace _9P2000L


}  // end of namespace styxe
#endif  // STYXE_9P2000L_HPP
