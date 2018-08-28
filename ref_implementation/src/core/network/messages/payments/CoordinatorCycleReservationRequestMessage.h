/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONREQUESTMESSAGE_H
#define GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONREQUESTMESSAGE_H

#include "base/RequestCycleMessage.h"

class CoordinatorCycleReservationRequestMessage :
    public RequestCycleMessage{

public:
    typedef shared_ptr<CoordinatorCycleReservationRequestMessage> Shared;
    typedef shared_ptr<const CoordinatorCycleReservationRequestMessage> ConstShared;

public:
    CoordinatorCycleReservationRequestMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID& transactionUUID,
        const TrustLineAmount& amount,
        const NodeUUID& nextNodeInThePath);

    CoordinatorCycleReservationRequestMessage(
        BytesShared buffer);

    const NodeUUID& nextNodeInPath() const;

    const Message::MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    NodeUUID mNextPathNode;
};


#endif //GEO_NETWORK_CLIENT_COORDINATORCYCLERESERVATIONREQUESTMESSAGE_H
