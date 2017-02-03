#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.h"

#include "../../../../common/Types.h"

#include "../../result/MessageResult.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

using namespace std;

class AcceptTrustLineMessage : public TrustLinesMessage {
public:
    typedef shared_ptr<AcceptTrustLineMessage> Shared;

public:
    AcceptTrustLineMessage(
        byte* buffer);

    const TrustLineAmount &amount() const;

    const MessageTypeID typeID() const;

    pair<ConstBytesShared, size_t> serialize();

    static const size_t kRequestedBufferSize();

    MessageResult::Shared resultAccepted() const;

    MessageResult::Shared resultConflict() const;

    MessageResult::Shared resultTransactionConflict() const;

    MessageResult::Shared customCodeResult(
        uint16_t code) const;

private:
    void deserialize(
        byte* buffer);

public:
    static const uint16_t kResultCodeAccepted = 200;
    static const uint16_t kResultCodeConflict = 409;
    static const uint16_t kResultCodeTransactionConflict = 500;

private:
    TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
