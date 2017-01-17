#ifndef GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H

#include "../../../common/Types.h"

#include "../Message.h"
#include "../result/MessageResult.h"

#include <memory>

using namespace std;

class RejectTrustLineMessage : public Message {

public:
    typedef shared_ptr<RejectTrustLineMessage> Shared;

public:
    RejectTrustLineMessage(
      NodeUUID sender,
      TransactionUUID transactionUUID,
      uint16_t resultCode
    );

    pair<ConstBytesShared, size_t> serialize();

    void deserialize(
        byte* buffer);

    const MessageTypeID typeID() const;

    uint16_t resultCode() const;

private:
    uint16_t mResultCode;
};


#endif //GEO_NETWORK_CLIENT_REJECTTRUSTLINEMESSAGE_H
