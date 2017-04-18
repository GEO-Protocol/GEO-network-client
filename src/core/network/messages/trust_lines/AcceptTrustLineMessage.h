#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H

#include "base/BaseTrustLineMessage.h"
#include "../result/MessageResult.h"
#include "../../../common/multiprecision/MultiprecisionUtils.h"


// ToDo: [review: hsc] merge with open trust line
class AcceptTrustLineMessage:
    public BaseTrustLineMessage {

public:
    typedef shared_ptr<AcceptTrustLineMessage> Shared;

public:
    AcceptTrustLineMessage(
        BytesShared buffer);

    const TrustLineAmount &amount() const;

    pair<BytesShared, size_t> serializeToBytes();

    MessageResult::SharedConst resultAccepted() const;

    MessageResult::SharedConst resultConflict() const;

    MessageResult::SharedConst resultTransactionConflict() const;

    const MessageType typeID() const;

public:
    static const uint16_t kResultCodeAccepted = 200;
    static const uint16_t kResultCodeConflict = 409;
    static const uint16_t kResultCodeTransactionConflict = 500;

private:
    TrustLineAmount mTrustLineAmount;
};
#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
