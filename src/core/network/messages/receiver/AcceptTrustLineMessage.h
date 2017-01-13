#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H

#include "../../../common/Types.h"

#include "../Message.h"

#include "../../../transactions/TransactionUUID.h"
#include "../../../trust_lines/TrustLine.h"

#include <string>
#include <vector>

using namespace std;

class AcceptTrustLineMessage : public Message {

public:
    AcceptTrustLineMessage(
        byte* buffer);

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

    const MessageTypeID typeID() const;

private:
    const size_t kTrustLineAmountSize = 32;

    TrustLineAmount mTrustLineAmount;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
