/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H
#define INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H


#include "base/ResponseMessage.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


class IntermediateNodeReservationResponseMessage:
    public ResponseMessage {

public:
    typedef shared_ptr<IntermediateNodeReservationResponseMessage> Shared;
    typedef shared_ptr<const IntermediateNodeReservationResponseMessage> ConstShared;

public:
    // TODO: Amount may be used as flag for approved/rejected
    // (true if amount > 0)
    IntermediateNodeReservationResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const PathID &pathID,
        const OperationState state,
        const TrustLineAmount &reservedAmount=0);

    IntermediateNodeReservationResponseMessage(
        BytesShared buffer);

    const TrustLineAmount& amountReserved() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    const MessageType typeID() const;

protected:
    TrustLineAmount mAmountReserved;
};

#endif // INTERMEDIATENODERESERVATIONRESPONSEMESSAGE_H
