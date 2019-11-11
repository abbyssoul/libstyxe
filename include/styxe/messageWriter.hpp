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
#ifndef STYXE_MESSAGEWRITER_HPP
#define STYXE_MESSAGEWRITER_HPP

#include "encoder.hpp"
#include "9p.hpp"


namespace styxe {

/** Encode a message header into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, MessageHeader value) {
	return encoder << value.messageSize
				   << value.type
				   << value.tag;

}

struct MessageWriterBase {

	/**
	 * @brief Construct a new ResponseWriter.
	 * @param dest A byte writer stream where data to be written.
	 * @param tag Tag of the message being created.
	 */
	constexpr MessageWriterBase(Solace::ByteWriter& dest, Tag messageTag) noexcept
		: _encoder{dest}
		, _pos{dest.position()}
		, _header{headerSize(), 0, messageTag}
	{}


	void updateMessageSize();


	/** Get underlying data encoder
	* @return Encoder
	*/
   constexpr Encoder& encoder() noexcept { return _encoder; }

   constexpr MessageHeader header() const noexcept { return _header; }

   Encoder& messageType(Solace::byte type) {
	   return messageType(type, _header.tag);
   }

   Encoder& messageType(Solace::byte type, Tag tag) {
	   _header.type = type;

	   return _encoder << MessageHeader{_header.messageSize, _header.type, tag};
   }

private:
   /** Finalize the message build.
   * @return ByteWriter stream
   */
   Solace::ByteWriter& build();

	/// Data encoder used to write data out
	Encoder							_encoder;

	/// Current position in the output stream where the message header starts
	Solace::ByteWriter::size_type	_pos;

	/// Message header
	MessageHeader					_header;
};

/**
* Helper type used to represent a message being built.
*/
template<typename MessageTag>
struct MessageWriter : public MessageWriterBase {

	/**
	 * @brief Construct a new MessageWriter.
	 * @param dest A byte writer stream where data to be written.
	 * @param tag Tag of the message being created.
	 */
	constexpr MessageWriter(Solace::ByteWriter& dest, Tag messageTag = kNoTag) noexcept
		: MessageWriterBase{dest, messageTag}
	{}

	MessageWriter& operator<< (MessageWriter& (*pf)(MessageWriter&)) {
		return pf(*this);
	}

	void operator<< (void (*pf)(MessageWriter&)) {
		pf(*this);
	}

};


struct ResponseTag {};
struct RequestTag {};

using RequestWriter = MessageWriter<RequestTag>;
using ResponseWriter = MessageWriter<ResponseTag>;


/// Message writer for messages that include repeated path segments.
struct PathWriter {
	/**
	 * Construct a new PathWriter.
	 * @param writer A byte stream to write the resulting message to.
	 */
	PathWriter(RequestWriter& writer) noexcept
		: _writer{writer}
		, _segmentsPos{writer.encoder().buffer().position()}
	{
		_writer.encoder() << _nSegments;
	}

	constexpr RequestWriter& writer() noexcept { return  _writer; }

	void segment(Solace::StringView value);

protected:
	RequestWriter&					_writer;

private:
	Solace::ByteWriter::size_type const	_segmentsPos;   //!< A position in the output stream where path segments start.
	WalkPath::size_type					_nSegments{0};  //!< Number of path segments written
};


PathWriter&& operator<< (PathWriter&& writer, Solace::StringView segment);



/// Message writer partial for messages that include trailing data segment.s
struct DataWriter {

	/**
	 * Construct a new DataWriter.
	 * @param writer A byte stream to write the resulting message to.
	 */
	DataWriter(RequestWriter& writer) noexcept
		: _writer{writer}
		, _segmentsPos{writer.encoder().buffer().position()}
	{
		_writer.encoder() << Solace::MemoryView{};
	}

	RequestWriter& data(Solace::MemoryView value);

private:
	RequestWriter&						_writer;
	Solace::ByteWriter::size_type const _segmentsPos;   //!< A position in the output stream where path segments start.
};


inline
RequestWriter& operator<< (DataWriter& writer, Solace::MemoryView segment) {
	return writer.data(segment);
}


struct PathDataWriter: public PathWriter {
	/**
	 * Construct a new PathDataWriter.
	 * @param writer A byte stream to write the resulting message to.
	 */
	PathDataWriter(RequestWriter& writer) noexcept
		: PathWriter{writer}
	{}

	RequestWriter& data(Solace::MemoryView value);
};


inline
PathDataWriter&& operator<< (PathDataWriter&& writer, Solace::StringView segment) {
	writer.segment(segment);
	return Solace::mv(writer);
}

inline
RequestWriter& operator<< (PathDataWriter&& writer, Solace::MemoryView segment) {
	return writer.data(segment);
}


}  // end of namespace styxe
#endif  // STYXE_MESSAGEWRITER_HPP
