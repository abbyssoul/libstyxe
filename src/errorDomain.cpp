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

#include "styxe/errorDomain.hpp"


using namespace Solace;
using namespace styxe;


AtomValue const
styxe::kProtocolErrorCatergory = atom("styxe");


#define CANNE(id, msg) \
	Error(kProtocolErrorCatergory, static_cast<uint16>(id), msg)


static Error const kCannedErrors[] = {
	CANNE(CannedError::UnsupportedProtocolVersion, "Unsupported protocol version"),
	CANNE(CannedError::UnsupportedMessageType, "Ill-formed message: Unsupported message type"),
	CANNE(CannedError::IllFormedHeader, "Ill-formed message header. Not enough data to read a header"),
	CANNE(CannedError::IllFormedHeader_FrameTooShort, "Ill-formed message: Declared frame size less than header"),
	CANNE(CannedError::IllFormedHeader_TooBig, "Ill-formed message: Declared frame size greater than negotiated one"),

	CANNE(CannedError::NotEnoughData, "Ill-formed message: Declared frame size larger than message data received"),
	CANNE(CannedError::MoreThenExpectedData, "Ill-formed message: Declared frame size less than message data received"),
};


Error
styxe::getCannedError(CannedError errorId) noexcept {
	return kCannedErrors[static_cast<int>(errorId)];
}
