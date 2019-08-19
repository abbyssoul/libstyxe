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
#ifndef STYXE_REQUESTWRITER_HPP
#define STYXE_REQUESTWRITER_HPP

#include "9p2000.hpp"


namespace styxe {

/**
 * Helper class to write Request messages.
 */
struct RequestWriter {
	struct DataWriter
			: public TypedWriter
	{
		DataWriter(Solace::ByteWriter& buffer, Solace::ByteWriter::size_type pos, MessageHeader head) noexcept
			: TypedWriter{buffer, pos, head}
		{}

		TypedWriter data(Solace::MemoryView data);
	};

	struct PathWriter
			: public TypedWriter
	{
		PathWriter(Solace::ByteWriter& buffer, Solace::ByteWriter::size_type pos, MessageHeader head) noexcept;

		PathWriter& path(Solace::StringView pathSegment);

		TypedWriter done() noexcept { return *this; }

	private:
		Solace::ByteWriter::size_type	_segmentsPos;
		WalkPath::size_type				_nSegments{0};
	};

	struct PathDataWriter
			: public DataWriter
	{
		PathDataWriter(Solace::ByteWriter& buffer, Solace::ByteWriter::size_type pos, MessageHeader head) noexcept;

		PathDataWriter& path(Solace::StringView pathSegment);

	private:
		Solace::ByteWriter::size_type	_segmentsPos;
		WalkPath::size_type				_nSegments{0};
	};


public:

	constexpr RequestWriter(Solace::ByteWriter& dest, Tag tag = 1) noexcept
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
	TypedWriter open(Fid fid, OpenMode mode);
	TypedWriter create(Fid fid,
							Solace::StringView name,
							Solace::uint32 permissions,
							OpenMode mode);
	TypedWriter read(Fid fid, Solace::uint64 offset, size_type count);
	DataWriter write(Fid fid, Solace::uint64 offset);
	TypedWriter clunk(Fid fid);
	TypedWriter remove(Fid fid);
	TypedWriter stat(Fid fid);
	TypedWriter writeStat(Fid fid, Stat const& stat);
	PathWriter walk(Fid fid, Fid nfid);


	/* 9P2000.e extention */
	TypedWriter session(Solace::ArrayView<Solace::byte> key);
	PathWriter shortRead(Fid rootFid);
	PathDataWriter shortWrite(Fid rootFid);

private:
	Tag const               _tag;
	Solace::ByteWriter&     _buffer;
};

}  // end of namespace styxe
#endif  // STYXE_REQUESTWRITER_HPP
