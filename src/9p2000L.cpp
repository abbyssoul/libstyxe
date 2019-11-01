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

#include "styxe/9p2000L.hpp"
#include "styxe/decoder.hpp"


using namespace Solace;
using namespace styxe;


const StringLiteral _9P2000L::kProtocolVersion{"9P2000.L"};


StringView
styxe::_9P2000L::messageTypeToString(byte type) noexcept {

#define TYPE_STR(msg) case _9P2000L::MessageType::msg: return StringLiteral{"#msg"}

	auto mType = static_cast<_9P2000L::MessageType>(type);
	switch (mType) {
	TYPE_STR(Tlerror);
	TYPE_STR(Rlerror);
	TYPE_STR(Tstatfs);
	TYPE_STR(Rstatfs);
	TYPE_STR(Tlopen);
	TYPE_STR(Rlopen);
	TYPE_STR(Tlcreate);
	TYPE_STR(Rlcreate);
	TYPE_STR(Tsymlink);
	TYPE_STR(Rsymlink);
	TYPE_STR(Tmknod);
	TYPE_STR(Rmknod);
	TYPE_STR(Trename);
	TYPE_STR(Rrename);
	TYPE_STR(Treadlink);
	TYPE_STR(Rreadlink);
	TYPE_STR(Tgetattr);
	TYPE_STR(Rgetattr);
	TYPE_STR(Tsetattr);
	TYPE_STR(Rsetattr);
	TYPE_STR(Txattrwalk);
	TYPE_STR(Rxattrwalk);
	TYPE_STR(Txattrcreate);
	TYPE_STR(Rxattrcreate);
	TYPE_STR(Treaddir);
	TYPE_STR(Rreaddir);
	TYPE_STR(Tfsync);
	TYPE_STR(Rfsync);
	TYPE_STR(Tlock);
	TYPE_STR(Rlock);
	TYPE_STR(Tgetlock);
	TYPE_STR(Rgetlock);
	TYPE_STR(Tlink);
	TYPE_STR(Rlink);
	TYPE_STR(Tmkdir);
	TYPE_STR(Rmkdir);
	TYPE_STR(Trenameat);
	TYPE_STR(Rrenameat);
	TYPE_STR(Tunlinkat);
	TYPE_STR(Runlinkat);
	}

	return styxe::messageTypeToString(type);
}

