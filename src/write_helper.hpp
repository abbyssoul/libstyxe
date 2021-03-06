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
#ifndef STYXE_INTERNAL_WRITE_HELPER_HPP
#define STYXE_INTERNAL_WRITE_HELPER_HPP

#include "styxe/messageWriter.hpp"


namespace styxe {

template<typename MsgType, typename...Args>
RequestWriter&
encode(RequestWriter& writer, MsgType const&, Args&& ...args) {
	(writer.messageTypeOf<MsgType>()
			<< ... << args);
	writer.updateMessageSize();

	return writer;
}


template<typename MsgType, typename...Args>
ResponseWriter&
encode(ResponseWriter& writer, MsgType const&, Args&& ...args) {
	(writer.messageTypeOf<MsgType>()
			<< ... << args);
	writer.updateMessageSize();

	return writer;
}


}  // namespace styxe
#endif  // STYXE_INTERNAL_WRITE_HELPER_HPP
