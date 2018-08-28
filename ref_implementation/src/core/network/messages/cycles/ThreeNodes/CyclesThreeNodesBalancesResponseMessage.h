/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
#define GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"


class CyclesThreeNodesBalancesResponseMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<CyclesThreeNodesBalancesResponseMessage> Shared;

public:
    CyclesThreeNodesBalancesResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        SerializedRecordsCount neighborsUUUIDCount);

    CyclesThreeNodesBalancesResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        vector<NodeUUID> &neighborsUUUID);

    CyclesThreeNodesBalancesResponseMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

    void addNeighborUUIDAndBalance(NodeUUID neighborUUID)
        throw (bad_alloc);

    const MessageType typeID() const
        noexcept;

    vector<NodeUUID> NeighborsAndBalances()
        noexcept;

protected:
    vector<NodeUUID> mNeighborsUUUID;
};

#endif //GEO_NETWORK_CLIENT_BALANCESRESPONCEMESSAGE_H
