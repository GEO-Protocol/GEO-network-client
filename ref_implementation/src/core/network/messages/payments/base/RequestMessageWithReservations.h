/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_REQUESTMESSAGEWITHRESERVATIONS_H
#define GEO_NETWORK_CLIENT_REQUESTMESSAGEWITHRESERVATIONS_H

#include "../../base/transaction/TransactionMessage.h"
#include "../../../../common/multiprecision/MultiprecisionUtils.h"

#include <vector>

class RequestMessageWithReservations : public TransactionMessage {

public:
    typedef shared_ptr<RequestMessageWithReservations> Shared;

public:
    RequestMessageWithReservations(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig);

    RequestMessageWithReservations(
        BytesShared buffer);

    const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfiguration() const;

protected:
    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

    const size_t kOffsetToInheritedBytes() const
    noexcept;

private:
    vector<pair<PathID, ConstSharedTrustLineAmount>> mFinalAmountsConfiguration;
};


#endif //GEO_NETWORK_CLIENT_REQUESTMESSAGEWITHRESERVATIONS_H
