/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H

#include "../base/transaction/DestinationMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


class SetIncomingTrustLineMessage:
    public DestinationMessage {

public:
    typedef shared_ptr<SetIncomingTrustLineMessage> Shared;

public:
    SetIncomingTrustLineMessage(
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationMessage,
        const TrustLineAmount &amount)
        noexcept;

    SetIncomingTrustLineMessage(
        BytesShared buffer)
        noexcept;

    const MessageType typeID() const
        noexcept;

    const TrustLineAmount& amount() const
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

protected:
    TrustLineAmount mAmount;
};


#endif //GEO_NETWORK_CLIENT_SETTRUSTLINEMESSAGE_H
