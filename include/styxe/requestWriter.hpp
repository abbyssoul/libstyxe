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
		: _encoder{dest}
		, _tag{tag}
	{}


private:
	/// Data encoder used to write data out
	Encoder					_encoder;

	/// Message tag
	Tag const               _tag;
};

}  // end of namespace styxe
#endif  // STYXE_REQUESTWRITER_HPP
