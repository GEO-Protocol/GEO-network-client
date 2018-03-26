/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
#define GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H

#include "base/CyclesBaseFiveOrSixNodesBoundaryMessage.h"
class CyclesSixNodesBoundaryMessage:
        public CyclesBaseFiveOrSixNodesBoundaryMessage {
public:
    typedef shared_ptr<CyclesSixNodesBoundaryMessage> Shared;

public:
    CyclesSixNodesBoundaryMessage(
        vector<NodeUUID> &path,
        vector<NodeUUID> &boundaryNodes) :
            CyclesBaseFiveOrSixNodesBoundaryMessage(
                    path,
                    boundaryNodes) {};

    CyclesSixNodesBoundaryMessage(
        BytesShared buffer):CyclesBaseFiveOrSixNodesBoundaryMessage(buffer){};

    const MessageType typeID() const {
        return Message::MessageType::Cycles_SixNodesBoundary;
    };
};

#endif //GEO_NETWORK_CLIENT_CYCLESIXNODESBOUNDARYMESSAGE_H
