/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_DESTINATIONMESSAGE_H
#define GEO_NETWORK_CLIENT_DESTINATIONMESSAGE_H

#include "TransactionMessage.h"

class DestinationMessage : public TransactionMessage {

public:
    typedef shared_ptr<DestinationMessage> Shared;
    typedef shared_ptr<const DestinationMessage> ConstShared;

public:
    DestinationMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const NodeUUID &destinationUUID)
    noexcept;

    DestinationMessage(
        BytesShared buffer)
    noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

    const NodeUUID &destinationUUID() const
    noexcept;

protected:
    const size_t kOffsetToInheritedBytes() const
    noexcept;

protected:
    NodeUUID mDestinationUUID;

};


#endif //GEO_NETWORK_CLIENT_DESTINATIONMESSAGE_H
