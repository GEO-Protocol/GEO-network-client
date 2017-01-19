#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H

#include "../../../common/Types.h"

#include "../Message.h"
#include "../result/MessageResult.h"

#include "../../../trust_lines/TrustLine.h"

#include <string>
#include <vector>
#include <memory>

using namespace std;

class AcceptTrustLineMessage : public Message {
public:
    typedef shared_ptr<AcceptTrustLineMessage> Shared;

public:
    AcceptTrustLineMessage(
        byte* buffer);

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

    const MessageTypeID typeID() const;

    TrustLineAmount amount() const;

    MessageResult::Shared resultAccepted() const;

    MessageResult::Shared resultConflict() const;

    MessageResult::Shared resultTransactionConflict() const;

    MessageResult::Shared customCodeResult(
        uint16_t code) const;

public:
    static const uint16_t kResultCodeAccepted = 200;
    static const uint16_t kResultCodeConflict = 409;
    static const uint16_t kResultCodeTransactionConflict = 500;

private:
    const size_t kTrustLineAmountSize = 32;

    TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
