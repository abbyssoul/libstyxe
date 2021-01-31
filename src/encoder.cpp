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

#include "styxe/encoder.hpp"
#include "styxe/9p.hpp"


#include <solace/utils.hpp>  // narrow_cast
#include <limits>



using namespace Solace;
using namespace styxe;



size_type
styxe::protocolSize(uint8 const& value) noexcept {
    return sizeof(value);
}

size_type
styxe::protocolSize(uint16 const& value) noexcept {
    return sizeof(value);
}

size_type
styxe::protocolSize(uint32 const& value) noexcept {
    return sizeof(value);
}

size_type
styxe::protocolSize(uint64 const& value) noexcept {
    return sizeof(value);
}

size_type
styxe::protocolSize(StringView const& value) noexcept {
	return sizeof(var_datum_size_type) +         // Space for string var size
			value.size();             // Space for the actual string bytes
}


size_type
styxe::protocolSize(MemoryView const& value) noexcept {
	constexpr auto const maxSize = static_cast<MemoryView::size_type>(std::numeric_limits<size_type>::max());
	auto const valueSize = value.size();
	assertIndexInRange(valueSize, maxSize, "protocolSize(:MemoryView)");

    return sizeof(size_type) +  // Var number of elements
			narrow_cast<size_type>(valueSize);
}




Encoder&
styxe::operator<< (Encoder& encoder, uint8 value) {
	encoder.buffer().writeLE(value);
	return encoder;
}

Encoder& styxe::operator<< (Encoder& encoder, uint16 value) {
	encoder.buffer().writeLE(value);
	return encoder;
}

Encoder& styxe::operator<< (Encoder& encoder, uint32 value) {
	encoder.buffer().writeLE(value);
	return encoder;
}

Encoder& styxe::operator<< (Encoder& encoder, uint64 value) {
	encoder.buffer().writeLE(value);
	return encoder;
}

Encoder& styxe::operator<< (Encoder& encoder, StringView str) {
	encoder << str.size();
	encoder.buffer().write(str.view());

	return encoder;
}


Encoder& styxe::operator<< (Encoder& encoder, MemoryView data) {
	encoder << narrow_cast<size_type>(data.size());
	encoder.buffer().write(data);

	return encoder;
}
