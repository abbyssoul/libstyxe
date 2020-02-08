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


/**
 * Base class for 9p message writers.
 */
struct MessageWriterBase {

	/**
	 * @brief Construct a new ResponseWriter.
	 * @param dest A byte writer stream where data to be written.
	 * @param messageTag Tag of the message being created.
	 */
	constexpr MessageWriterBase(Solace::ByteWriter& dest, Tag messageTag) noexcept
		: _encoder{dest}
		, _pos{dest.position()}
		, _header{headerSize(), 0, messageTag}
	{}


	/// Update message header with the actual number of bytes written so far.
	void updateMessageSize();


	/** Get underlying data encoder
	* @return Encoder
	*/
   constexpr Encoder& encoder() noexcept { return _encoder; }

   /**
	*  Get formed message header
	* @return Copy of the message header.
	*/
   constexpr MessageHeader header() const noexcept { return _header; }

   /**
	* Set message type and write newly formed message header to the output stream.
	* @param typeCode Message type byte-code. @see MessageHeader::type
	* @return styxe::Encoder to write payload data to.
	*/
   Encoder& messageType(Solace::byte typeCode) {
	   return messageType(typeCode, _header.tag);
   }

   template<typename MsgType>
   Encoder& messageTypeOf() {
	   return messageType(messageCodeOf<MsgType>());
   }

   /**
	* Set message type and write newly formed message header to the output stream.
	* @param type Message type byte-code. @see MessageHeader::type
	* @param tag Message tag. @see MessageHeader::tag
	* @return styxe::Encoder to write payload data to.
	*/
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
	 * @param messageTag Tag of the message being created.
	 */
	constexpr MessageWriter(Solace::ByteWriter& dest, Tag messageTag = kNoTag) noexcept
		: MessageWriterBase{dest, messageTag}
	{}

	/**
	 * IO manipulator helper function. Accept io manipulator in a function form and apply it.
	 * @return Resulting reference to a writer after application of the manipulator passed in.
	 */
	MessageWriter& operator<< (MessageWriter& (*pf)(MessageWriter&)) {
		return pf(*this);
	}

	/**
	 * IO manipulator helper function. Accept io manipulator in a function form and apply it.
	 */
	void operator<< (void (*pf)(MessageWriter&)) {
		pf(*this);
	}

};


struct ResponseTag {};
struct RequestTag {};

using RequestWriter = MessageWriter<RequestTag>;
using ResponseWriter = MessageWriter<ResponseTag>;



/// Message writer partial for messages that include trailing data segment.s
struct PartialDataWriter {

	/**
	 * Construct a new DataWriter.
	 * @param writer A byte stream to write the resulting message to.
	 */
	explicit PartialDataWriter(MessageWriterBase& writer) noexcept
		: _writer{writer}  // Store a writer object
		, _segmentsPos{writer.encoder().buffer().position()}  // Save postion in the output stream
	{
		_writer.encoder() << Solace::MemoryView{};  // Write empty data segment (to be overwritten later)
		_writer.updateMessageSize();
	}

	MessageWriterBase& update(size_type dataSize);

	/**
	 * Write data field to the output writer.
	 * @param value Data buffer to write
	 * @return Ref to request original request writer.
	 */
	MessageWriterBase& data(Solace::MemoryView value);

	Solace::MutableMemoryView viewRemainder();

private:
	MessageWriterBase&						_writer;
	Solace::ByteWriter::size_type const		_segmentsPos;   //!< A position in the output stream where path segments start.
	size_type								_dataSize{0};   //!< Total size of data written so far
};



inline
PartialDataWriter&& operator<< (PartialDataWriter&& writer, Solace::MemoryView segment) {
	writer.data(segment);
	return Solace::mv(writer);
}

inline
PartialDataWriter& operator<< (PartialDataWriter& writer, Solace::MemoryView segment) {
	writer.data(segment);
	return writer;
}


/**
 * Message writer specialization for messages that include repeated path segments.
 */
struct PartialPathWriter {
	/**
	 * Construct a new PathWriter.
	 * @param writer A byte stream to write the resulting message to.
	 */
	explicit PartialPathWriter(RequestWriter& writer) noexcept
		: _writer{writer}
		, _segmentsPos{writer.encoder().buffer().position()}
	{
		_writer.encoder() << _nSegments;
		_writer.updateMessageSize();
	}

	/**
	 * Get a reference to the underlying writer object
	 * @return reference to the underlying writer object
	 */
	constexpr RequestWriter& writer() noexcept { return  _writer; }

	/**
	 * Write path segment of a path.
	 * @param value A string representation of a path segment to be written.
	 */
	void segment(Solace::StringView value);

protected:
	RequestWriter&					_writer;  //!< Ref to the underlying writer object the data written to.

private:
	Solace::ByteWriter::size_type const	_segmentsPos;   //!< A position in the output stream where path segments start.
	WalkPath::size_type					_nSegments{0};  //!< Number of path segments written
};


PartialPathWriter&& operator<< (PartialPathWriter&& writer, Solace::StringView segment);

/**
 *  Message writer specialization to create partial messages that include repeated path segments followed by data.
 */
struct PathDataWriter final : public PartialPathWriter {
	/**
	 * Construct a new PathDataWriter.
	 * @param writer A byte stream to write the resulting message to.
	 */
	explicit PathDataWriter(RequestWriter& writer) noexcept
		: PartialPathWriter{writer}
	{}

	/**
	 * Write data field to the output writer.
	 * @param value Data buffer to write
	 * @return Ref to request original request writer.
	 */
	RequestWriter& data(Solace::MemoryView value);
};


inline
PathDataWriter&& operator<< (PathDataWriter&& writer, Solace::StringView segment) {
	writer.segment(segment);
	return Solace::mv(writer);
}

inline
PathDataWriter& operator<< (PathDataWriter& writer, Solace::StringView segment) {
	writer.segment(segment);
	return writer;
}

inline
RequestWriter& operator<< (PathDataWriter&& writer, Solace::MemoryView segment) {
	return writer.data(segment);
}



/// Message writer partial for messages that include trailing string segments
struct PartialStringWriter {

	/**
	 * Construct a new DataWriter.
	 * @param writer A byte stream to write the resulting message to.
	 */
	explicit PartialStringWriter(MessageWriterBase& writer) noexcept
		: _writer{writer}  // Store a writer object
		, _segmentsPos{writer.encoder().buffer().position()}  // Save postion in the output stream
	{
		_writer.encoder() << Solace::StringView{};  // Write empty data segment (to be overwritten later)
		_writer.updateMessageSize();
	}

	/**
	 * Write data field to the output writer.
	 * @param value Data buffer to write
	 * @return Ref to request original request writer.
	 */
	MessageWriterBase& string(Solace::StringView value);

private:
	MessageWriterBase&						_writer;
	Solace::ByteWriter::size_type const		_segmentsPos;   //!< A position in the output stream where path segments start.
	Solace::StringView::size_type			_dataSize{0};   //!< A position in the output stream where path segments start.
};

inline
PartialStringWriter&& operator<< (PartialStringWriter&& writer, Solace::StringView segment) {
	writer.string(segment);
	return Solace::mv(writer);
}

inline
PartialStringWriter& operator<< (PartialStringWriter& writer, Solace::StringView segment) {
	writer.string(segment);
	return writer;
}


}  // end of namespace styxe
#endif  // STYXE_MESSAGEWRITER_HPP
