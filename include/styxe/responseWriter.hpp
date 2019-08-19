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

	constexpr ResponseWriter(Solace::ByteWriter& dest, Tag tag) noexcept
		: _tag{tag}
		, _buffer{dest}
	{}

	TypedWriter version(Solace::StringView version, size_type maxMessageSize = kMaxMesssageSize);
	TypedWriter auth(Qid qid);
	TypedWriter error(Solace::StringView message);
	TypedWriter error(Solace::Error const& err) {
		return error(err.toString().view());
	}

	TypedWriter flush();
	TypedWriter attach(Qid qid);
	TypedWriter walk(Solace::ArrayView<Qid> qids);
	TypedWriter open(Qid qid, size_type iounit);
	TypedWriter create(Qid qid, size_type iounit);
	TypedWriter read(Solace::MemoryView data);
	TypedWriter write(size_type iounit);
	TypedWriter clunk();
	TypedWriter remove();
	TypedWriter stat(Stat const& value);
	TypedWriter wstat();

	/* 9P2000.e extention */
	TypedWriter session();
	TypedWriter shortRead(Solace::MemoryView data);
	TypedWriter shortWrite(size_type iounit);

private:
	Tag const               _tag;
	Solace::ByteWriter&     _buffer;
};


}  // end of namespace styxe
#endif  // STYXE_RESPONSEWRITER_HPP
