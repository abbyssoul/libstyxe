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

#include "styxe/9p2000L.hpp"
#include "styxe/decoder.hpp"

#include "parse_helper.hpp"
#include "write_helper.hpp"


using namespace Solace;
using namespace styxe;


const StringLiteral _9P2000L::kProtocolVersion{"9P2000.L"};


StringView
styxe::_9P2000L::messageTypeToString(byte type) noexcept {

#define TYPE_STR(msg) case _9P2000L::MessageType::msg: return StringLiteral{"#msg"}

	auto mType = static_cast<_9P2000L::MessageType>(type);
	switch (mType) {
	TYPE_STR(Tlerror);
	TYPE_STR(Rlerror);
	TYPE_STR(Tstatfs);
	TYPE_STR(Rstatfs);
	TYPE_STR(Tlopen);
	TYPE_STR(Rlopen);
	TYPE_STR(Tlcreate);
	TYPE_STR(Rlcreate);
	TYPE_STR(Tsymlink);
	TYPE_STR(Rsymlink);
	TYPE_STR(Tmknod);
	TYPE_STR(Rmknod);
	TYPE_STR(Trename);
	TYPE_STR(Rrename);
	TYPE_STR(Treadlink);
	TYPE_STR(Rreadlink);
	TYPE_STR(Tgetattr);
	TYPE_STR(Rgetattr);
	TYPE_STR(Tsetattr);
	TYPE_STR(Rsetattr);
	TYPE_STR(Txattrwalk);
	TYPE_STR(Rxattrwalk);
	TYPE_STR(Txattrcreate);
	TYPE_STR(Rxattrcreate);
	TYPE_STR(Treaddir);
	TYPE_STR(Rreaddir);
	TYPE_STR(Tfsync);
	TYPE_STR(Rfsync);
	TYPE_STR(Tlock);
	TYPE_STR(Rlock);
	TYPE_STR(Tgetlock);
	TYPE_STR(Rgetlock);
	TYPE_STR(Tlink);
	TYPE_STR(Rlink);
	TYPE_STR(Tmkdir);
	TYPE_STR(Rmkdir);
	TYPE_STR(Trenameat);
	TYPE_STR(Rrenameat);
	TYPE_STR(Tunlinkat);
	TYPE_STR(Runlinkat);
	}

	return styxe::messageTypeToString(type);
}


RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::StatFS const& message){
	return encode(writer, message, message.fid);
}


RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::Open const& message) {
	return encode(writer, message, message.fid, message.flags);
}


RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::Create const& message) {
	return encode(writer, message,
				  message.fid,
				  message.name,
				  message.flags,
				  message.mode,
				  message.gid);

}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::Symlink const& message) {
	return encode(writer, message,
				  message.fid,
				  message.name,
				  message.symtgt,
				  message.gid);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::MkNode const& message){
	return encode(writer, message,
			message.dfid,
			message.name,
			message.mode,
			message.major,
			message.minor,
			message.gid);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::Rename const& message){
	return encode(writer, message,
			message.fid,
			message.dfid,
			message.name);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::ReadLink const& message){
	return encode(writer, message,
			message.fid);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::GetAttr const& message){
	return encode(writer, message,
			message.fid,
			message.request_mask);

}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::SetAttr const& message){
	return encode(writer, message,
			message.fid,
			message.valid,
			message.mode,
			message.uid,
			message.gid,
			message.size,
			message.atime_sec, message.atime_nsec,
			message.mtime_sec, message.mtime_nsec);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::XAttrWalk const& message){
	return encode(writer, message,
			message.fid,
			message.newfid,
			message.name);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::XAttrCreate const& message){
	return encode(writer, message,
			message.fid,
			message.name,
			message.attr_size,
			message.flags);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::ReadDir const& message){
	return encode(writer, message,
			message.fid,
			message.offset,
			message.count);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::FSync const& message){
	return encode(writer, message,
			message.fid);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::Lock const& message){
	return encode(writer, message,
			message.fid,
			message.type,
			message.flags,
			message.start,
			message.length,
			message.proc_id,
			message.client_id);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::GetLock const& message) {
	return encode(writer, message,
			message.fid,
			message.type,
			message.start,
			message.length,
			message.proc_id,
			message.client_id);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::Link const& message) {
	return encode(writer, message,
			message.dfid,
			message.fid,
			message.name);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::MkDir const& message) {
	return encode(writer, message,
			message.dfid,
			message.name,
			message.mode,
			message.gid);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::RenameAt const& message){
	return encode(writer, message, message.olddirfid, message.oldname, message.newdirfid, message.newname);
}

RequestWriter& styxe::operator<< (RequestWriter& writer, _9P2000L::Request::UnlinkAt const& message) {
	return encode(writer, message, message.dfid, message.name, message.flags);
}

//----------------------------------------------------------------------------------------------------------------------

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::LError const& message){
	return encode(writer, message, message.ecode);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::StatFS const& message) {
	return encode(writer, message,
				  message.type,
				  message.bsize,
				  message.blocks,
				  message.bfree,
				  message.bavail,
				  message.files,
				  message.ffree,
				  message.fsid,
				  message.namelen);
}


ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::Open const& message){
	return encode(writer, message, message.qid, message.iounit);
}


ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::Create const& message){
	return encode(writer, message, message.qid, message.iounit);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::Symlink const& message){
	return encode(writer, message, message.qid);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::MkNode const& message){
	return encode(writer, message, message.qid);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::Rename const&  message) {
	return encode(writer, message);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::ReadLink const& message){
	return encode(writer, message, message.target);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::GetAttr const& m){
	return encode(writer, m,
				  m.valid,
				  m.qid,
				  m.mode,
				  m.uid,
				  m.gid,
				  m.nlink,
				  m.rdev,
				  m.size,
				  m.blksize,
				  m.blocks,
				  m.atime_sec, m.atime_nsec,
				  m.mtime_sec, m.mtime_nsec,
				  m.ctime_sec, m.ctime_nsec,
				  m.btime_sec, m.btime_nsec,
				  m.gen, m.data_version);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::SetAttr const& message) {
	return encode(writer, message);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::XAttrWalk const& message) {
	return encode(writer, message, message.size);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::XAttrCreate const& message) {
	return encode(writer, message);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::ReadDir const& message) {
	return encode(writer, message, message.data);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::FSync const& message) {
	return encode(writer, message);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::Lock const& message) {
	return encode(writer, message, message.status);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::GetLock const& message) {
	return encode(writer, message, message.type, message.start, message.length, message.proc_id, message.client_id);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::Link const& message) {
	return encode(writer, message);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::MkDir const& message) {
	return encode(writer, message, message.qid);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::RenameAt const& message) {
	return encode(writer, message);
}

ResponseWriter& styxe::operator<< (ResponseWriter& writer, _9P2000L::Response::UnlinkAt const& message) {
	return encode(writer, message);
}



Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::StatFS& dest) {
	return decode(data, dest.fid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::Open& dest) {
	return decode(data, dest.fid, dest.flags);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::Create& dest) {
	return decode(data, dest.fid, dest.name, dest.flags, dest.mode, dest.gid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::Symlink& dest) {
	return decode(data, dest.fid, dest.name, dest.symtgt, dest.gid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::MkNode& dest) {
	return decode(data, dest.dfid, dest.name, dest.mode, dest.major, dest.minor, dest.gid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::Rename& dest) {
	return decode(data, dest.fid, dest.dfid, dest.name);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::ReadLink& dest) {
	return decode(data, dest.fid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::GetAttr& dest) {
	return decode(data, dest.fid, dest.request_mask);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::SetAttr& dest) {
	return decode(data, dest.fid,
				  dest.valid,
				  dest.mode,
				  dest.uid,
				  dest.gid,
				  dest.size,
				  dest.atime_sec, dest.atime_nsec,
				  dest.mtime_sec, dest.mtime_nsec);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::XAttrWalk& dest) {
	return decode(data, dest.fid, dest.newfid, dest.name);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::XAttrCreate& dest) {
	return decode(data, dest.fid, dest.name, dest.attr_size, dest.flags);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::ReadDir& dest) {
	return decode(data, dest.fid, dest.offset, dest.count);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::FSync& dest) {
	return decode(data, dest.fid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::Lock& dest) {
	return decode(data, dest.fid, dest.type, dest.flags, dest.start, dest.length, dest.proc_id, dest.client_id);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::GetLock& dest) {
	return decode(data, dest.fid, dest.type, dest.start, dest.length, dest.proc_id, dest.client_id);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::Link& dest) {
	return decode(data, dest.dfid, dest.fid, dest.name);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::MkDir& dest) {
	return decode(data, dest.dfid, dest.name, dest.mode, dest.gid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::RenameAt& dest) {
	return decode(data, dest.olddirfid, dest.oldname, dest.newdirfid, dest.newname);
}
Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Request::UnlinkAt& dest) {
	return decode(data, dest.dfid, dest.name, dest.flags);
}


//----------------------------------------------------------------------------------------------------------------------

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::LError& dest) {
	return decode(data, dest.ecode);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::StatFS& dest) {
	return decode(data,
				  dest.type,
				  dest.bsize,
				  dest.blocks,
				  dest.bfree,
				  dest.bavail,
				  dest.files,
				  dest.ffree,
				  dest.fsid,
				  dest.namelen);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::Open& dest) {
	return decode(data, dest.qid, dest.iounit);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::Create& dest) {
	return decode(data, dest.qid, dest.iounit);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::Symlink& dest) {
	return decode(data, dest.qid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::MkNode& dest) {
	return decode(data, dest.qid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::Rename&) {
	return decode(data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::ReadLink& dest) {
	return decode(data, dest.target);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::GetAttr& dest) {
	return decode(data,
				  dest.valid,
				  dest.qid,
				  dest.mode,
				  dest.uid,
				  dest.gid,
				  dest.nlink,
				  dest.rdev,
				  dest.size,
				  dest.blksize,
				  dest.blocks,
				  dest.atime_sec, dest.atime_nsec,
				  dest.mtime_sec, dest.mtime_nsec,
				  dest.ctime_sec, dest.ctime_nsec,
				  dest.btime_sec, dest.btime_nsec,
				  dest.gen, dest.data_version);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::SetAttr&) {
	return decode(data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::XAttrWalk& dest) {
	return decode(data, dest.size);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::XAttrCreate&) {
	return decode(data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::ReadDir& dest) {
	return decode(data, dest.data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::FSync&) {
	return decode(data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::Lock& dest) {
	return decode(data, dest.status);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::GetLock& dest) {
	return decode(data, dest.type, dest.start, dest.length, dest.proc_id, dest.client_id);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::Link&) {
	return decode(data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::MkDir& dest) {
	return decode(data, dest.qid);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::RenameAt& ) {
	return decode(data);
}

Result<ByteReader&, Error>
styxe::operator>> (ByteReader& data, _9P2000L::Response::UnlinkAt&) {
	return decode(data);
}


Result<void, Error>
_9P2000L::DirEntryReader::Iterator::read() {
	Decoder decoder{_reader};
	auto readResult = decoder >> _value;
	if(!readResult) {
		return readResult.moveError();
	}

	return Ok();
}
