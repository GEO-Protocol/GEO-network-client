/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H
#define GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

#include "../../../../common/multiprecision/MultiprecisionUtils.h"

class RequestCycleMessage : public TransactionMessage {

public:
    RequestCycleMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const TrustLineAmount &amount);

    RequestCycleMessage(
        BytesShared buffer);

    const TrustLineAmount& amount() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

    const size_t kOffsetToInheritedBytes() const
    noexcept;

protected:
    TrustLineAmount mAmount;
};


#endif //GEO_NETWORK_CLIENT_REQUESTCYCLEMESSAGE_H
