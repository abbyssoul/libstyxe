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
#ifndef STYXE_9P2000U_HPP
#define STYXE_9P2000U_HPP

#include "styxe/9p2000.hpp"


namespace styxe {
namespace _9P2000U {

/// Protocol version literal
extern const Solace::StringLiteral kProtocolVersion;


enum class MessageType : Solace::byte {
	/// 9p2000.u adds no new messages but extand existing
};

struct v9fs_stat : public Stat {
/* 9p2000.u extensions */
	Solace::StringView extension;
	Solace::uint32 n_uid;
	Solace::uint32 n_gid;
	Solace::uint32 n_muid;
};


/** Encode a file stats into the output stream.
 * @param encoder Encoder used to encode the value.
 * @param value Value to encode.
 * @return Ref to the encoder for fluency.
 */
inline
Encoder& operator<< (Encoder& encoder, v9fs_stat const& value) {
	return encoder << static_cast<Stat const&>(value)
				   << value.extension
				   << value.n_uid
				   << value.n_gid
				   << value.n_muid;
}


/** Decode a Stat struct from the stream.
 * @param decoder A data stream to read a value from.
 * @param dest An address where to store decoded value.
 * @return Ref to the decoder or Error if operation has failed.
 */
inline
Solace::Result<Decoder&, Error> operator>> (Decoder& decoder, v9fs_stat& dest) {
	return decoder >> static_cast<Stat&>(dest)
				   >> dest.extension
				   >> dest.n_uid
				   >> dest.n_gid
				   >> dest.n_muid;
}


/// 9P2000.u protocol messages
struct Request {

	/// Messages to establish a connection.
	struct Auth : public ::styxe::Request::Auth {
		Solace::uint32		n_uname;
	};

	/// A fresh introduction from a user on the client machine to the server.
	struct Attach : public ::styxe::Request::Attach {
		Solace::uint32		n_uname;
	};

	/**
	 * The create request asks the file server to create a new file with the name supplied,
	 * in the directory (dir) represented by fid, and requires write permission in the directory.
	 * The owner of the file is the implied user id of the request.
	 */
	struct Create : public ::styxe::Request::Create {
		Solace::StringView  extension;   //!< extension
	};

	/**
	 * A request to update file stat fields.
	 */
	struct WStat {
		Fid				fid;    //!< Fid of the file to update stats on.
		v9fs_stat		stat;   //!< New stats to update file info to.
	};

};


/// 9P2000.u messages
struct Response {
	/// Error resoponse from a server
	struct Error: public ::styxe::Response::Error {
		Solace::uint32		errcode;  /// Error code
	};

	/// Stat response
	struct Stat {
		var_datum_size_type dummySize;  //!< Dummy variable size of data.
		v9fs_stat			data;       //!< File stat data
	};
};


/**
 * Get a string representation of the message name given the op-code.
 * @param messageType Message op-code to convert to a string.
 * @return A string representation of a given message code.
 */
inline
Solace::StringView
messageTypeToString(Solace::byte type) noexcept {
	// Note: 9p2000u does not introduce new messages but extend existing
	return ::styxe::messageTypeToString(type);
}

}  // end of namespace _9P2000U


inline
size_type protocolSize(_9P2000U::v9fs_stat const& value) noexcept {
	return protocolSize(static_cast<Stat const&>(value))
			+ protocolSize(value.extension)
			+ protocolSize(value.n_uid)
			+ protocolSize(value.n_gid)
			+ protocolSize(value.n_muid);
}


/** Serialize Auth request message into a request writer stream.
 * @param writer Bytestream to write the message to.
 * @param message A message to serilalize.
 * @return Ref to writer for fluent interface.
 */
RequestWriter& operator<< (RequestWriter& writer, _9P2000U::Request::Auth const& message);

/** Serialize Attach request message into a request writer stream.
 * @param writer Bytestream to write the message to.
 * @param message A message to serilalize.
 * @return Ref to writer for fluent interface.
 */
RequestWriter& operator<< (RequestWriter& writer, _9P2000U::Request::Attach const& message);

/** Serialize Create request message into a request writer stream.
 * @param writer Bytestream to write the message to.
 * @param message A message to serilalize.
 * @return Ref to writer for fluent interface.
 */
RequestWriter& operator<< (RequestWriter& writer, _9P2000U::Request::Create const& message);

/** Serialize WStat request message into a request writer stream.
 * @param writer Bytestream to write the message to.
 * @param message A message to serilalize.
 * @return Ref to writer for fluent interface.
 */
RequestWriter& operator<< (RequestWriter& writer, _9P2000U::Request::WStat const& message);

/** Serialize Error response message into a request writer stream.
 * @param writer Bytestream to write the message to.
 * @param message A message to serilalize.
 * @return Ref to writer for fluent interface.
 */
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000U::Response::Error const& message);

/** Serialize Stat response message into a request writer stream.
 * @param writer Bytestream to write the message to.
 * @param message A message to serilalize.
 * @return Ref to writer for fluent interface.
 */
ResponseWriter& operator<< (ResponseWriter& writer, _9P2000U::Response::Stat const& message);


Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000U::Request::Auth& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000U::Request::Attach& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000U::Request::Create& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000U::Request::WStat& dest);

Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000U::Response::Error& dest);
Solace::Result<Solace::ByteReader&, Error>
operator>> (Solace::ByteReader& data, _9P2000U::Response::Stat& dest);


template <>
constexpr Solace::byte messageCodeOf<_9P2000U::Request::Auth>() noexcept { return asByte(MessageType::TAuth); }
template <>
constexpr Solace::byte messageCodeOf<_9P2000U::Request::Attach>() noexcept { return asByte(MessageType::TAttach); }
template <>
constexpr Solace::byte messageCodeOf<_9P2000U::Request::Create>() noexcept { return asByte(MessageType::TCreate); }
template <>
constexpr Solace::byte messageCodeOf<_9P2000U::Request::WStat>() noexcept { return asByte(MessageType::TWStat); }

template <>
constexpr Solace::byte messageCodeOf<_9P2000U::Response::Stat>() noexcept { return asByte(MessageType::RStat); }
template <>
constexpr Solace::byte messageCodeOf<_9P2000U::Response::Error>() noexcept { return asByte(MessageType::RError); }


}  // end of namespace styxe
#endif  // STYXE_9P2000U_HPP
