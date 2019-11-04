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
#ifndef STYXE_TESTHARNES_HPP
#define STYXE_TESTHARNES_HPP

#include "styxe/errorDomain.hpp"
#include "styxe/9p2000.hpp"

#include <solace/output_utils.hpp>

#include <gtest/gtest.h>


styxe::Qid randomQid(styxe::QidType type = styxe::QidType::FILE) noexcept;


struct TestHarnes : public ::testing::Test {

	// cppcheck-suppress unusedFunction
	void SetUp() override {
		_memBuf.view().fill(0xFE);

		_writer.rewind();
	}

	void logFailure(styxe::Error const& e) {
		FAIL() << e.toString();
	}

	Solace::MemoryManager   _memManager {styxe::kMaxMessageSize};
	Solace::MemoryResource  _memBuf{_memManager.allocate(styxe::kMaxMessageSize).unwrap()};
	Solace::ByteWriter      _writer{_memBuf};

};



#endif  // STYXE_TESTHARNES_HPP
