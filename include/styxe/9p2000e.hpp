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
#ifndef STYXE_9P2000E_HPP
#define STYXE_9P2000E_HPP

#include "styxe/9p2000.hpp"


namespace styxe {
namespace _9P2000E {

/// Protocol version literal
extern const Solace::StringLiteral kProtocolVersion;


enum class MessageType : Solace::byte {
	/**
	 * 9P2000.e extension
	 */
	TSession = 150,
	RSession,
	TShortRead = 152,
	RShortRead,
	TShortWrite = 154,
	RShortWrite,
};


/// 9P2000 protocol Erlang extension new messages
struct Request : public styxe::Request {

	/// A request to re-establish a session.
	struct Session {
		Solace::byte key[8];    //!< A key of the previously established session.
	};

	/// A request to read entire file contents.
	struct ShortRead {
		Fid             fid;    //!< Fid of the root directory to walk the path from.
		WalkPath		path;   //!< A path to the file to be read.
	};

	/// A request to overwrite file contents.
	struct ShortWrite {
		Fid			        fid;    //!< Fid of the root directory to walk the path from.
		WalkPath			path;   //!< A path to the file to be read.
		Solace::MemoryView	data;   //!< A data to be written into the file.
	};
};


/// 9P2000 protocol Erlang extension new messages
struct Response : public styxe::Response {
	/// Session re-establishment response
	struct Session {};

	/// Read resopose
	struct ShortRead {
		/// View in to the response buffer where raw read data is.
		Solace::MemoryView data;
	};

	/// Write response
	struct ShortWrite {
		size_type  count;  //!< Number of bytes written
	};
};


/**
 * Get a string representation of the message name given the op-code.
 * @param messageType Message op-code to convert to a string.
 * @return A string representation of a given message code.
 */
Solace::StringView
messageTypeToString(Solace::byte type) noexcept;


}  // end of namespace _9P2000E


inline constexpr Solace::byte asByte(_9P2000E::MessageType type) noexcept {
	return static_cast<Solace::byte>(type);
}

inline
auto messageCode(_9P2000E::Request::Session const&) noexcept { return asByte(_9P2000E::MessageType::TSession); }

inline
auto messageCode(_9P2000E::Request::ShortRead const&) noexcept { return asByte(_9P2000E::MessageType::TShortRead); }

inline
auto messageCode(_9P2000E::Request::ShortWrite const&) noexcept { return asByte(_9P2000E::MessageType::TShortWrite); }

inline
auto messageCode(_9P2000E::Response::Session const&) noexcept { return asByte(_9P2000E::MessageType::RSession); }

inline
auto messageCode(_9P2000E::Response::ShortRead const&) noexcept { return asByte(_9P2000E::MessageType::RShortRead); }

inline
auto messageCode(_9P2000E::Response::ShortWrite const&) noexcept { return asByte(_9P2000E::MessageType::RShortWrite); }


Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000E::Request::Session& dest);

Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000E::Request::ShortRead& dest);

Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000E::Request::ShortWrite& dest);

Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000E::Response::Session& dest);

Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000E::Response::ShortRead& dest);

Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000E::Response::ShortWrite& dest);


/** Create Session request. */
RequestWriter& operator<< (RequestWriter& writer, _9P2000E::Request::Session const& request);

/** Create ShortRead request. */
RequestWriter& operator<< (RequestWriter& writer, _9P2000E::Request::ShortRead const& request);

/** Create short Write request. */
RequestWriter& operator<< (RequestWriter& writer, _9P2000E::Request::ShortWrite const& request);

/** Create Session restore response. */
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000E::Response::Session const& response);

/** Create ShortRead response. */
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000E::Response::ShortRead const& response);

/** Create ShortWriteresponse. */
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000E::Response::ShortWrite const& response);


}  // end of namespace styxe
#endif  // STYXE_9P2000E_HPP
