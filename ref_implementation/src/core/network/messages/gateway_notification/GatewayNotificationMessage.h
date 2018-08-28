/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class GatewayNotificationMessage : public TransactionMessage {

public:
    typedef shared_ptr<GatewayNotificationMessage> Shared;

public:
    enum NodeState {
        Common = 1,
        Gateway = 2,
    };

public:
    GatewayNotificationMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const NodeState state);

    GatewayNotificationMessage(
        BytesShared buffer);

    const NodeState nodeState() const;

    const MessageType typeID() const;

protected:
    typedef byte SerializedNodeState;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    NodeState mNodeState;
};


#endif //GEO_NETWORK_CLIENT_GATEWAYNOTIFICATIONMESSAGE_H
