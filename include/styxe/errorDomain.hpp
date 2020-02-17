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
#ifndef STYXE_ERRORDOMAIN_HPP
#define STYXE_ERRORDOMAIN_HPP

#include <solace/atom.hpp>
#include <solace/error.hpp>


namespace styxe {

/** Error type used to represent runtime error by the library */
using Error = Solace::Error;

/// Result type shortrhand
template<typename T>
using Result = Solace::Result<T, Error>;


/** Error category for protocol specific errors */
extern Solace::AtomValue const kProtocolErrorCatergory;

/**
 * Enum class for protocol error codes.
 */
enum class CannedError : int {
	UnsupportedProtocolVersion = 0,
	UnsupportedMessageType,
	IllFormedHeader,
	IllFormedHeader_FrameTooShort,
	IllFormedHeader_TooBig,
	NotEnoughData,
	MoreThenExpectedData,
};

/**
 * Get canned error from the protocol error code.
 * @param errorId Error code of the canned error.
 * @return Error object for the error category
 */
Error
getCannedError(CannedError errorId) noexcept;


}  // end of namespace styxe
#endif  // STYXE_ERRORDOMAIN_HPP
