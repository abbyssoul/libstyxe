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

#include "styxe/messageWriter.hpp"
#include "styxe/encoder.hpp"

#include "styxe/9p2000.hpp"
#include "styxe/9p2000e.hpp"

using namespace Solace;
using namespace styxe;


ByteWriter&
MessageWriter::build() {
	auto const finalPos = _encoder.buffer().position();
	auto const messageSize = finalPos - _pos;  // Re-compute actual message size
	auto& buffer = _encoder.buffer();
	buffer.position(_pos);  // Reset to the start position

	_header.messageSize = narrow_cast<size_type>(messageSize);
	_encoder << _header;

	if (_header.type == static_cast<byte>(MessageType::RRead) ||
		_header.type == static_cast<byte>(_9P2000E::MessageType::RShortRead)) {
		auto const dataSize = narrow_cast<size_type>(finalPos - buffer.position() - sizeof(size_type));
		_encoder << dataSize;
	}

	buffer.position(finalPos);
	return buffer.flip();
}
