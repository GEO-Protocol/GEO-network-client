/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_ROUTINGTABLEREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_ROUTINGTABLEREQUESTMESSAGE_H

#include "../SenderMessage.h"


class RoutingTableRequestMessage :
    public SenderMessage {

public:
    typedef shared_ptr<RoutingTableRequestMessage> Shared;

public:
    using SenderMessage::SenderMessage;

    const MessageType typeID() const;
};

#endif //GEO_NETWORK_CLIENT_ROUTINGTABLEREQUESTMESSAGE_H
