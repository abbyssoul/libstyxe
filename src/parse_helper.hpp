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
#ifndef STYXE_INTERNAL_PARSE_HELPER_HPP
#define STYXE_INTERNAL_PARSE_HELPER_HPP

#include "styxe/errorDomain.hpp"
#include "styxe/decoder.hpp"

#include <solace/byteReader.hpp>
#include <solace/result.hpp>

namespace styxe {

template<typename...Args>
Solace::Result<Solace::ByteReader&, Error>
decode(Solace::ByteReader& data, Args&& ...args) {
	Decoder decoder{data};
	auto result = (decoder >> ... >> args);
	if (!result) return result.moveError();

	return Solace::Result<Solace::ByteReader&, Error>{Solace::types::okTag, data};
}

}  // namespace styxe
#endif  // STYXE_INTERNAL_PARSE_HELPER_HPP
