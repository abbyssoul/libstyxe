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
#include "testHarnes.hpp"
#include <random>

using namespace Solace;

styxe::Qid randomQid(styxe::QidType type) noexcept {
	std::random_device rd;
	std::default_random_engine randomGen{rd()};
	std::linear_congruential_engine<uint32, 48271, 0, 2147483647> randomGen2{rd()};

	return {
		randomGen(),
		randomGen2(),
		static_cast<byte>(type)
	};
}
