#ifndef GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H

#include "../../../common/Types.h"

#include "../Message.h"
#include "../result/MessageResult.h"

#include "../../../transactions/TransactionUUID.h"
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

    AcceptTrustLineMessage(
        NodeUUID sender,
        TransactionUUID transactionUUID,
        uint16_t journalCode);

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

    const MessageTypeID typeID() const;

    TrustLineAmount amount() const;

    const MessageResult* resultConflict() const;

private:
    const size_t kTrustLineAmountSize = 32;

    TrustLineAmount mTrustLineAmount;
    uint16_t mJournalCode;
};


#endif //GEO_NETWORK_CLIENT_ACCEPTTRUSTLINEMESSAGE_H
