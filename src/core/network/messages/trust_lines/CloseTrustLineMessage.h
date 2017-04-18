#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"


// ToDo: WARN! There is no deserialization method! Is this message work correct?
class CloseTrustLineMessage:
    public TransactionMessage {

public:
    CloseTrustLineMessage(
        const NodeUUID &sender,
        const TransactionUUID &transactionUUID,
        const NodeUUID &contractorUUID)
        noexcept;

    const MessageType typeID() const
        noexcept;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

protected:
    const NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
