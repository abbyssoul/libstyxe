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

#include "styxe/9p2000.hpp"
#include "styxe/print.hpp"

#include <ostream>


namespace styxe {

    std::ostream& operator<< (std::ostream& ostr, MessageType t) {

        switch (t) {
        case MessageType::TVersion: ostr << "TVersion"; break;
        case MessageType::RVersion: ostr << "RVersion"; break;
        case MessageType::TAuth:    ostr << "TAuth"; break;
        case MessageType::RAuth:    ostr << "RAuth"; break;
        case MessageType::TAttach:  ostr << "TAttach"; break;
        case MessageType::RAttach:  ostr << "RAttach"; break;
        case MessageType::TError:   ostr << "TError"; break;
        case MessageType::RError:   ostr << "RError"; break;
        case MessageType::TFlush:   ostr << "TFlush"; break;
        case MessageType::RFlush:   ostr << "RFlush"; break;
        case MessageType::TWalk:    ostr << "TWalk"; break;
        case MessageType::RWalk:    ostr << "RWalk"; break;
        case MessageType::TOpen:    ostr << "TOpen"; break;
        case MessageType::ROpen:    ostr << "ROpen"; break;
        case MessageType::TCreate:  ostr << "TCreate"; break;
        case MessageType::RCreate:  ostr << "RCreate"; break;
        case MessageType::TRead:    ostr << "TRead"; break;
        case MessageType::RRead:    ostr << "RRead"; break;
        case MessageType::TWrite:   ostr << "TWrite"; break;
        case MessageType::RWrite:   ostr << "RWrite"; break;
        case MessageType::TClunk:   ostr << "TClunk"; break;
        case MessageType::RClunk:   ostr << "RClunk"; break;
        case MessageType::TRemove:  ostr << "TRemove"; break;
        case MessageType::RRemove:  ostr << "RRemove"; break;
        case MessageType::TStat:    ostr << "TStat"; break;
        case MessageType::RStat:    ostr << "RStat"; break;
        case MessageType::TWStat:   ostr << "TWStat"; break;
        case MessageType::RWStat:   ostr << "RWStat"; break;

        case MessageType::TSession: ostr << "TSession"; break;
        case MessageType::RSession: ostr << "RSession"; break;
        case MessageType::TSRead:   ostr << "TSRead"; break;
        case MessageType::RSRead:   ostr << "RSRead"; break;
        case MessageType::TSWrite:  ostr << "TSWrite"; break;
        case MessageType::RSWrite:  ostr << "RSWrite"; break;
        default:
            ostr << "[Unknown value '" << static_cast<Solace::byte>(t) << "']";
        }

        return ostr;
    }
}  // end of namespace styxe
