/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_VOTESSTATUSREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_VOTESSTATUSREQUESTMESSAGE_H

#include "../base/transaction/TransactionMessage.h"


class VotesStatusRequestMessage :
    public TransactionMessage {

public:

    using TransactionMessage::TransactionMessage;

public:
    const MessageType typeID() const{
        return Message::Payments_VotesStatusRequest;
    };
};


#endif //GEO_NETWORK_CLIENT_VOTESSTATUSREQUESTMESSAGE_H
