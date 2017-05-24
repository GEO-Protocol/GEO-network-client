#ifndef GEO_NETWORK_CLIENT_UPDATETRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_UPDATETRUSTLINEMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../result/MessageResult.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


class UpdateTrustLineMessage:
    public TransactionMessage {

public:
    typedef shared_ptr<UpdateTrustLineMessage> Shared;

public:
    UpdateTrustLineMessage(
        BytesShared buffer)
        noexcept;

    const TrustLineAmount &newAmount() const
        noexcept;

    virtual pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

    const MessageType typeID() const
        noexcept;

public:
    static const uint16_t kResultCodeAccepted = 200;
    static const uint16_t kResultCodeRejected = 401;
    static const uint16_t kResultCodeTrustLineAbsent = 404;
    static const uint16_t kResultCodeConflict = 409;

protected:
    TrustLineAmount mNewTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_UPDATETRUSTLINEMESSAGE_H
