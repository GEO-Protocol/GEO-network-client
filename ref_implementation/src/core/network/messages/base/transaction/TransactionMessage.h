/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
#define GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H

#include "../../SenderMessage.h"

#include "../../../../transactions/transactions/base/TransactionUUID.h"


class TransactionMessage:
    public SenderMessage {

public:
    typedef shared_ptr<TransactionMessage> Shared;
    typedef shared_ptr<const TransactionMessage> ConstShared;

public:
    TransactionMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID)
        noexcept;

    TransactionMessage(
        BytesShared buffer)
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const;

    const TransactionUUID &transactionUUID() const
        noexcept;

protected:
    const size_t kOffsetToInheritedBytes() const
        noexcept;

    const bool isTransactionMessage() const
        noexcept;

protected:
    const TransactionUUID mTransactionUUID;
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONMESSAGE_H
