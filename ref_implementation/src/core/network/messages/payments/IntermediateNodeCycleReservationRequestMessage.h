/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H

#include "base/RequestCycleMessage.h"

class IntermediateNodeCycleReservationRequestMessage :
    public RequestCycleMessage {

public:
    typedef shared_ptr<IntermediateNodeCycleReservationRequestMessage> Shared;
    typedef shared_ptr<const IntermediateNodeCycleReservationRequestMessage> ConstShared;

public:
    IntermediateNodeCycleReservationRequestMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID& transactionUUID,
        const TrustLineAmount& amount,
        const NodeUUID& coordinatorUUID,
        SerializedPathLengthSize cucleLength);

    IntermediateNodeCycleReservationRequestMessage(
        BytesShared buffer);

    const NodeUUID& coordinatorUUID() const;

    SerializedPathLengthSize cycleLength() const;

protected:
    const MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    SerializedPathLengthSize mCycleLength;
    NodeUUID mCoordinatorUUID;
};


#endif //GEO_NETWORK_CLIENT_INTERMEDIATENODECYCLERESERVATIONREQUESTMESSAGE_H
