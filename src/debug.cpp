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

    std::ostream& operator<< (std::ostream& ostr, Protocol::MessageType t) {

        switch (t) {
        case Protocol::MessageType::TVersion: ostr << "TVersion"; break;
        case Protocol::MessageType::RVersion: ostr << "RVersion"; break;
        case Protocol::MessageType::TAuth:    ostr << "TAuth"; break;
        case Protocol::MessageType::RAuth:    ostr << "RAuth"; break;
        case Protocol::MessageType::TAttach:  ostr << "TAttach"; break;
        case Protocol::MessageType::RAttach:  ostr << "RAttach"; break;
        case Protocol::MessageType::TError:   ostr << "TError"; break;
        case Protocol::MessageType::RError:   ostr << "RError"; break;
        case Protocol::MessageType::TFlush:   ostr << "TFlush"; break;
        case Protocol::MessageType::RFlush:   ostr << "RFlush"; break;
        case Protocol::MessageType::TWalk:    ostr << "TWalk"; break;
        case Protocol::MessageType::RWalk:    ostr << "RWalk"; break;
        case Protocol::MessageType::TOpen:    ostr << "TOpen"; break;
        case Protocol::MessageType::ROpen:    ostr << "ROpen"; break;
        case Protocol::MessageType::TCreate:  ostr << "TCreate"; break;
        case Protocol::MessageType::RCreate:  ostr << "RCreate"; break;
        case Protocol::MessageType::TRead:    ostr << "TRead"; break;
        case Protocol::MessageType::RRead:    ostr << "RRead"; break;
        case Protocol::MessageType::TWrite:   ostr << "TWrite"; break;
        case Protocol::MessageType::RWrite:   ostr << "RWrite"; break;
        case Protocol::MessageType::TClunk:   ostr << "TClunk"; break;
        case Protocol::MessageType::RClunk:   ostr << "RClunk"; break;
        case Protocol::MessageType::TRemove:  ostr << "TRemove"; break;
        case Protocol::MessageType::RRemove:  ostr << "RRemove"; break;
        case Protocol::MessageType::TStat:    ostr << "TStat"; break;
        case Protocol::MessageType::RStat:    ostr << "RStat"; break;
        case Protocol::MessageType::TWStat:   ostr << "TWStat"; break;
        case Protocol::MessageType::RWStat:   ostr << "RWStat"; break;

        case Protocol::MessageType::TSession: ostr << "TSession"; break;
        case Protocol::MessageType::RSession: ostr << "RSession"; break;
        case Protocol::MessageType::TSRead:   ostr << "TSRead"; break;
        case Protocol::MessageType::RSRead:   ostr << "RSRead"; break;
        case Protocol::MessageType::TSWrite:  ostr << "TSWrite"; break;
        case Protocol::MessageType::RSWrite:  ostr << "RSWrite"; break;
        default:
            ostr << "[Unknown value '" << static_cast<Solace::byte>(t) << "']";
        }

        return ostr;
    }
}  // end of namespace styxe
