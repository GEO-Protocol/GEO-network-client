/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef COORDINATORRESERVATIONREQUESTMESSAGE_H
#define COORDINATORRESERVATIONREQUESTMESSAGE_H

#include "base/RequestMessageWithReservations.h"

class CoordinatorReservationRequestMessage:
    public RequestMessageWithReservations {

public:
    typedef shared_ptr<CoordinatorReservationRequestMessage> Shared;
    typedef shared_ptr<const CoordinatorReservationRequestMessage> ConstShared;

public:
    CoordinatorReservationRequestMessage(
        const NodeUUID& senderUUID,
        const TransactionUUID& transactionUUID,
        const vector<pair<PathID, ConstSharedTrustLineAmount>> &finalAmountsConfig,
        const NodeUUID& nextNodeInThePath);

    CoordinatorReservationRequestMessage(
        BytesShared buffer);

    const NodeUUID& nextNodeInPath() const;

    const Message::MessageType typeID() const;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

protected:
     NodeUUID mNextPathNode;
};

#endif // COORDINATORRESERVATIONREQUESTMESSAGE_H
