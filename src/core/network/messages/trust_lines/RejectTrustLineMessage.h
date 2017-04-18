#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../result/MessageResult.h"


class RejectTrustLineMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<RejectTrustLineMessage> Shared;

public:
    RejectTrustLineMessage(
        BytesShared buffer)
        noexcept;

    const NodeUUID &contractorUUID() const
        noexcept;

    const MessageType typeID() const
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw(bad_alloc);

    MessageResult::SharedConst resultRejected()
        throw(bad_alloc);

    MessageResult::SharedConst resultRejectDelayed()
        throw(bad_alloc);

    MessageResult::SharedConst resultTransactionConflict() const
        throw(bad_alloc);

public:
    static const uint16_t kResultCodeRejected = 200;
    static const uint16_t kResultCodeRejectDelayed = 202;
    static const uint16_t kResultCodeTransactionConflict = 500;

private:
    const NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H
