/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_TTLPROLONGATIONRESPONSEMESSAGE_H
#define GEO_NETWORK_CLIENT_TTLPROLONGATIONRESPONSEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"

class TTLProlongationResponseMessage : public TransactionMessage {

public:
    typedef shared_ptr<TTLProlongationResponseMessage> Shared;
    typedef shared_ptr<const TTLProlongationResponseMessage> ConstShared;

public:
    enum OperationState {
        Continue = 1,
        Finish = 2,
    };

public:
    TTLProlongationResponseMessage(
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        const OperationState state);

    TTLProlongationResponseMessage(
        BytesShared buffer);

    const Message::MessageType typeID() const;

    const OperationState state() const;

protected:
    typedef byte SerializedOperationState;

    virtual pair<BytesShared, size_t> serializeToBytes() const
    throw(bad_alloc);

protected:
    OperationState mState;
};


#endif //GEO_NETWORK_CLIENT_TTLPROLONGATIONRESPONSEMESSAGE_H
