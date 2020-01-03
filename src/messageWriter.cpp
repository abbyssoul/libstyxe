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


using namespace Solace;
using namespace styxe;


void MessageWriterBase::updateMessageSize() {
	auto const finalPos = _encoder.buffer().position();
	auto const messageSize = finalPos - _pos;  // Re-compute actual message size
	if (finalPos == _pos || _header.messageSize == messageSize) {  // Nothing to do
		return;
	}

	// Update message size filed in the header
	_header.messageSize = narrow_cast<size_type>(messageSize);

	auto& buffer = _encoder.buffer();
	buffer.position(_pos);  // Reset output stream to the start position

	// Write a new header with updated message size
	_encoder << _header;

	// Reset output stream to the current position
	buffer.position(finalPos);
}


ByteWriter&
MessageWriterBase::build() {
	updateMessageSize();

	return _encoder.buffer().flip();
}

void PathWriter::segment(Solace::StringView value) {
	auto& buffer = _writer.encoder().buffer();
	auto const finalPos = buffer.position();
	buffer.position(_segmentsPos);  // Reset output stream to the start position

	_nSegments += 1;
	_writer.encoder() << _nSegments;

	buffer.position(finalPos);  // Reset output stream to the final position
	_writer.encoder() << value;
	_writer.updateMessageSize();
}


MessageWriterBase&
DataWriter::data(MemoryView value) {
	auto& buffer = _writer.encoder().buffer();
	buffer.position(_segmentsPos);  // Reset output stream to the start position
	_writer.encoder() << value;
	_writer.updateMessageSize();

	return _writer;
}


MessageWriterBase&
PartialStringWriter::string(Solace::StringView value) {
	auto& buffer = _writer.encoder().buffer();

	_dataSize += value.size();
	buffer.write(value.view());
	auto const finalPos = buffer.position();

	buffer.position(_segmentsPos);  // Reset output stream to the start position
	_writer.encoder() << _dataSize;
	buffer.position(finalPos);

	_writer.updateMessageSize();

	return _writer;
}



RequestWriter&
PathDataWriter::data(Solace::MemoryView value) {
	_writer.encoder() << value;
	_writer.updateMessageSize();

	return _writer;
}



PathWriter&&
styxe::operator<< (PathWriter&& writer, StringView segment) {
	writer.segment(segment);
	return mv(writer);
}
