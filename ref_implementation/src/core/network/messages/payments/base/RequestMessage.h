/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef REQUESTMESSAGE_H
#define REQUESTMESSAGE_H


#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"


class RequestMessage:
    public TransactionMessage {

public:
    RequestMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PathID &pathID,
        const TrustLineAmount &amount);

    RequestMessage(
        BytesShared buffer);

    const TrustLineAmount& amount() const;

    const PathID& pathID() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    const size_t kOffsetToInheritedBytes() const
        noexcept;

protected:
    TrustLineAmount mAmount;
    PathID mPathID;
};

#endif // REQUESTMESSAGE_H
