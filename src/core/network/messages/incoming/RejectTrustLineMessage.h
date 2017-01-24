#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H

#include "../Message.h"

#include "../../../common/Types.h"

#include "../result/MessageResult.h"

class RejectTrustLineMessage : public Message {
public:
    typedef shared_ptr<RejectTrustLineMessage> Shared;

public:
    RejectTrustLineMessage(
        byte *buffer);

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

    const MessageTypeID typeID() const;

    const NodeUUID &contractorUUID() const;

    MessageResult::Shared resultRejected();

    MessageResult::Shared resultRejectDelayed();

    MessageResult::Shared resultTransactionConflict() const;

public:
    static const uint16_t kResultCodeRejected = 200;
    static const uint16_t kResultCodeRejectDelayed = 202;
    static const uint16_t kResultCodeTransactionConflict = 500;

private:
    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H
