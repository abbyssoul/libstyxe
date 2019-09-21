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
 * Helper class to build and write Request messages.
 */
struct RequestWriter {

	/// Message writer with data payload.
	struct DataWriter
			: public TypedWriter
	{
		/**
		 * @brief Construct a new DataWriter. @see TypedWriter for more details.
		 * @param buffer A byte stream to write the resulting message to.
		 * @param pos A position in the stream where the message header has been written.
		 * @param header Message header.
		 */
		DataWriter(Solace::ByteWriter& buffer, Solace::ByteWriter::size_type pos, MessageHeader header) noexcept
			: TypedWriter{buffer, pos, header}
		{}

		/**
		 * @brief Write data segement into the message
		 * @param data Data to be written as a message payload
		 * @return TypedWriter to continue message building process
		 */
		TypedWriter data(Solace::MemoryView data);
	};

	/// Message writer for messages that include repeated path segments.
	struct PathWriter
			: public TypedWriter
	{
		/**
		 * @brief Construct a new PathWriter. @see TypedWriter for more details.
		 * @param buffer A byte stream to write the resulting message to.
		 * @param pos A position in the stream where the message header has been written.
		 * @param header Message header.
		 */
		PathWriter(Solace::ByteWriter& buffer, Solace::ByteWriter::size_type pos, MessageHeader header) noexcept;

		/**
		 * @brief Write path segment into the current message.
		 * @param pathSegment Path segment to write next in the message being built.
		 * @return A reference to this for fluent interface.
		 */
		PathWriter& path(Solace::StringView pathSegment);

		/**
		 * @brief Finilize message builder
		 * @return Typed message writer
		 */
		TypedWriter done() noexcept { return *this; }

	private:
		Solace::ByteWriter::size_type	_segmentsPos;   //!< A position in the output stream where path segments start.
		WalkPath::size_type				_nSegments{0};  //!< Number of path segments written
	};

	/// Message writer for messages that include both repeated path segments and data payload.
	struct PathDataWriter
			: public DataWriter
	{
		/**
		 * @brief Construct a new PathDataWriter. @see TypedWriter for more details.
		 * @param buffer A byte stream to write the resulting message to.
		 * @param pos A position in the stream where the message header has been written.
		 * @param header Message header.
		 */
		PathDataWriter(Solace::ByteWriter& buffer, Solace::ByteWriter::size_type pos, MessageHeader header) noexcept;

		/**
		 * @brief Write path segment into the current message.
		 * @param pathSegment Path segment to write next in the message being built.
		 * @return A reference to this for fluent interface.
		 */
		PathDataWriter& path(Solace::StringView pathSegment);

	private:
		Solace::ByteWriter::size_type	_segmentsPos;   //!< A position in the output stream where path segments start.
		WalkPath::size_type				_nSegments{0};  //!< Number of path segments written
	};


public:

	/**
	 * @brief Construct a new RequestWriter.
	 * @param dest A byte writer stream where data to be written.
	 * @param tag Tag of the message being created.
	 */
	constexpr RequestWriter(Solace::ByteWriter& dest, Tag tag = 1) noexcept
		: _buffer{dest}
		, _tag{tag}
	{}

	/**
	 * Create a version request.
	 * @param version Suggested protocol version.
	 * @param maxMessageSize Suggest maximum size of the protocol message, including mandatory message header.
	 * @return Ref to this for fluent interface.
	 */
	TypedWriter version(Solace::StringView version,
						size_type maxMessageSize = kMaxMesssageSize);
	/**
	 * @brief Create Auth request.
	 * @param afid User provided fid to be used for auth operations.
	 * @param userName User name to authenticate as.
	 * @param attachName Name of the filesystem to attach / authenticate to.
	 * @return Message builder.
	 */
	TypedWriter auth(Fid afid, Solace::StringView userName, Solace::StringView attachName);

	/**
	 * @brief Create a Flush request.
	 * @param oldTransation ID of the transaction to flush.
	 * @return Message builder.
	 */
	TypedWriter flush(Tag oldTransation);

	/**
	 * @brief Create Attach request
	 * @param fid User provided fid to be assosiated with attached filesystem.
	 * @param afid Authentication fid used during auth phase.
	 * @param userName User name the use used to authenticate with.
	 * @param attachName Name of the attachment / fs a user has authenticated to.
	 * @return Message builder.
	 */
	TypedWriter attach(Fid fid, Fid afid,
					   Solace::StringView userName, Solace::StringView attachName);
	/**
	 * @brief Create Open file request.
	 * @param fid User provided fid to be assosiated with the opened file.
	 * @param mode File open mode. @see OpenMode for details.
	 * @return Message builder.
	 */
	TypedWriter open(Fid fid, OpenMode mode);

	/** Create file Create request.
	 * @param fid User provided fid assosiated with the directory where file to be created.
	 * @param name Name of the file to create.
	 * @param permissions Permissions of the newly created file.
	 * @param mode File open mode. @see OpenMode for details.
	 * @return Message builder.
	 */
	TypedWriter create(Fid fid,
					   Solace::StringView name,
					   Solace::uint32 permissions,
					   OpenMode mode);

	/**
	 * @brief Create Read request
	 * @param fid User provided fid assosiated with a file to read from.
	 * @param offset Offset from the start of the file to read from.
	 * @param count Number of bytes to read from the file.
	 * @return Message builder.
	 */
	TypedWriter read(Fid fid, Solace::uint64 offset, size_type count);

	/**
	 * @brief Create Write request.
	 * @param fid User provided fid assosiated with a file to write to.
	 * @param offset Offset from the start of the file to read from.
	 * @return Message builder.
	 */
	DataWriter write(Fid fid, Solace::uint64 offset);

	/**
	 * @brief Create Clunk request.
	 * @param fid User provided fid to be forgotten by the server.
	 * @return Message builder.
	 */
	TypedWriter clunk(Fid fid);

	/**
	 * @brief Create Remove file request.
	 * @param fid User provided fid assosiated with a file to be removed.
	 * @return Message builder.
	 */
	TypedWriter remove(Fid fid);

	/**
	 * @brief Create stat request.
	 * @param fid User provided fid assosiated with a file to query stats for.
	 * @return Message builder.
	 */
	TypedWriter stat(Fid fid);

	/**
	 * @brief Cretea WriteStat request.
	 * @param fid User provided fid assosiated with a file to write stats for.
	 * @param stat File stats to be written.
	 * @return Message builder.
	 */
	TypedWriter writeStat(Fid fid, Stat const& stat);

	/**
	 * @brief Create Walk request.
	 * @param fid User provided fid assosiated with a file to start the walk from.
	 * @param nfid User provided fid to be assosiated with the file resulting from the walk.
	 * @return Message builder.
	 */
	PathWriter walk(Fid fid, Fid nfid);

	/* 9P2000.e extention */
	/**
	 * @brief Create Session request.
	 * @param key 8-bytes key of the session to restore.
	 * @return Message builder.
	 */
	TypedWriter session(Solace::ArrayView<Solace::byte> key);

	/**
	 * @brief Create ShortRead request.
	 * @param rootFid User provided fid assosiated with a file to read from.
	 * @return Message builder.
	 */
	PathWriter shortRead(Fid rootFid);

	/**
	 * @brief Create shortWrite request.
	 * @param rootFid User provided fid assosiated with a file to write to.
	 * @return Message builder.
	 */
	PathDataWriter shortWrite(Fid rootFid);

private:
	/// Byte writer where all data goes
	Solace::ByteWriter&     _buffer;

	/// Message tag
	Tag const               _tag;
};

}  // end of namespace styxe
#endif  // STYXE_REQUESTWRITER_HPP
