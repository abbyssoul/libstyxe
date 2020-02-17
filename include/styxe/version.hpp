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
#ifndef STYXE_VERSION_HPP
#define STYXE_VERSION_HPP

#include <solace/version.hpp>


#define STYXE_VERSION_MAJOR 0
#define STYXE_VERSION_MINOR 7
#define STYXE_VERSION_BUILD 9


namespace styxe {

/**
 * Get compiled version of the library.
 * @note This is not the protocol version, but the version of the library itself.
 * @return Build-version of the library.
 */
Solace::Version const& getVersion() noexcept;

}  // end of namespace styxe
#endif  // STYXE_VERSION_HPP
