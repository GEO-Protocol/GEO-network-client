/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_RESPONSECYCLEMESSAGE_H
#define GEO_NETWORK_CLIENT_RESPONSECYCLEMESSAGE_H

#include "../../base/transaction/TransactionMessage.h"

class ResponseCycleMessage : public TransactionMessage {

public:
    enum OperationState {
        Accepted = 1,
        Rejected = 2,
        RejectedBecauseReservations = 3,
        NextNodeInaccessible = 4
    };

public:
    ResponseCycleMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    ResponseCycleMessage(
        BytesShared buffer);

    const OperationState state() const;

protected:
    typedef byte SerializedOperationState;
    const size_t kOffsetToInheritedBytes() const
    noexcept;

    pair<BytesShared, size_t> serializeToBytes() const
    throw (bad_alloc);

private:
    OperationState mState;
};


#endif //GEO_NETWORK_CLIENT_RESPONSECYCLEMESSAGE_H
