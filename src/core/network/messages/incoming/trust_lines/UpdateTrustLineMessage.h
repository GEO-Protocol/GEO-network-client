#ifndef GEO_NETWORK_CLIENT_UPDATETRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_UPDATETRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.h"

#include "../../../../common/Types.h"

#include "../../result/MessageResult.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

using namespace std;

class UpdateTrustLineMessage : public TrustLinesMessage {
public:
    typedef shared_ptr<UpdateTrustLineMessage> Shared;

public:
    UpdateTrustLineMessage(
        byte* buffer);

    const MessageTypeID typeID() const;

    const TrustLineAmount &newAmount() const;

    pair<ConstBytesShared, size_t> serialize();

    static const size_t kRequestedBufferSize();

    MessageResult::Shared resultAccepted() const;

    MessageResult::Shared resultRejected() const;

    MessageResult::Shared resulConflict() const;

    MessageResult::Shared resultTransactionConflict() const;

    MessageResult::Shared customCodeResult(
        uint16_t code) const;

private:
    void deserialize(
        byte* buffer);

public:
    static const uint16_t kResultCodeAccepted = 200;
    static const uint16_t kResultCodeRejected = 401;
    static const uint16_t kResultCodeConflict = 409;
    static const uint16_t kResultCodeTransactionConflict = 500;

private:
    TrustLineAmount mNewTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_UPDATETRUSTLINEMESSAGE_H
