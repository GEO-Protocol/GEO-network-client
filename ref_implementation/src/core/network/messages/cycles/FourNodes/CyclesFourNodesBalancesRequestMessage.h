/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <set>


class CyclesFourNodesBalancesRequestMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<CyclesFourNodesBalancesRequestMessage> Shared;

public:
    CyclesFourNodesBalancesRequestMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const NodeUUID &creditorNeighbor,
        const NodeUUID &debtorNeighbor);

    CyclesFourNodesBalancesRequestMessage(
        BytesShared buffer);

    virtual pair<BytesShared, size_t> serializeToBytes()const
        throw(bad_alloc);

    const MessageType typeID() const;

    const NodeUUID debtor() const;

    const NodeUUID creditor() const;

protected:
    NodeUUID mDebtorUUID;
    NodeUUID mCreditorUUID;
};

#endif //GEO_NETWORK_CLIENT_FOURNODESBALANCESREQUESTMESSAGE_H
