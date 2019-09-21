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
#ifndef STYXE_RESPONSEWRITER_HPP
#define STYXE_RESPONSEWRITER_HPP
#include "9p2000.hpp"


namespace styxe {

/**
 * Helper class to build response messages.
 */
class ResponseWriter {
public:

	/**
	 * @brief Construct a new ResponseWriter.
	 * @param dest A byte writer stream where data to be written.
	 * @param tag Tag of the message being created.
	 */
	constexpr ResponseWriter(Solace::ByteWriter& dest, Tag tag) noexcept
		: _buffer{dest}
		, _tag{tag}
	{}

	/**
	 * @brief Create Version response
	 * @param version Protocol version that server can support.
	 * @param maxMessageSize Maximum message size in bytes that a server can support.
	 * @return Message builder.
	 */
	TypedWriter version(Solace::StringView version, size_type maxMessageSize = kMaxMesssageSize);

	/**
	 * @brief Create Auth response
	 * @param qid Qid of the file to be used for authentication.
	 * @return Message builder.
	 */
	TypedWriter auth(Qid qid);

	/**
	 * @brief Create error respose.
	 * @param message Error message to communicate back to the client.
	 * @return Message builder.
	 */
	TypedWriter error(Solace::StringView message);

	/**
	 * @brief Create error response from a Solace::Error type.
	 * @param err System error type to communicate back to the client.
	 * @return Message builder.
	 */
	TypedWriter error(Solace::Error const& err) {
		return error(err.toString().view());
	}

	/**
	 * @brief Create Flush response.
	 * @return Message builder.
	 */
	TypedWriter flush();

	/**
	 * @brief Create attach response.
	 * @param qid Qid of the filesystem a client attached to.
	 * @return Message builder.
	 */
	TypedWriter attach(Qid qid);

	/**
	 * @brief Create walk response.
	 * @param qids An array of qids that a server walked through.
	 * @return Message builder.
	 */
	TypedWriter walk(Solace::ArrayView<Qid> qids);

	/**
	 * @brief Create open response.
	 * @param qid Qid of the opened file.
	 * @param iounit Hint for the optimal read/write size.
	 * @return Message builder.
	 */
	TypedWriter open(Qid qid, size_type iounit);

	/**
	 * @brief Create Create response.
	 * @param qid Qid of the created file.
	 * @param iounit Hint for the optimal read/write size.
	 * @return Message builder.
	 */
	TypedWriter create(Qid qid, size_type iounit);

	/**
	 * @brief Create Read file respose.
	 * @param data Data read from the file to be sent back to the client.
	 * @return Message builder.
	 */
	TypedWriter read(Solace::MemoryView data);

	/**
	 * @brief Create Write file response.
	 * @param iounit Number of bytes written.
	 * @return Message builder.
	 */
	TypedWriter write(size_type iounit);

	/**
	 * @brief Create Clunk response.
	 * @return Message builder.
	 */
	TypedWriter clunk();

	/**
	 * @brief Create Remove response.
	 * @return Message builder.
	 */
	TypedWriter remove();

	/**
	 * @brief Create stat response.
	 * @param value Stat data read about the file.
	 * @return Message builder.
	 */
	TypedWriter stat(Stat const& value);

	/**
	 * @brief Create Write Stats.
	 * @return Message builder.
	 */
	TypedWriter wstat();


	/* 9P2000.e extention */

	/**
	 * @brief Create Session restore response.
	 * @return Message builder.
	 */
	TypedWriter session();

	/**
	 * @brief Create ShortRead respose.
	 * @param data Data read from the file to be sent back to the client.
	 * @return Message builder.
	 */
	TypedWriter shortRead(Solace::MemoryView data);

	/**
	 * @brief Create ShortWrite respose.
	 * @param iounit Number of bytes actually written.
	 * @return Message builder.
	 */
	TypedWriter shortWrite(size_type iounit);

private:
	/// Byte writer where all data goes
	Solace::ByteWriter&     _buffer;

	/// Message tag
	Tag const               _tag;
};


}  // end of namespace styxe
#endif  // STYXE_RESPONSEWRITER_HPP
