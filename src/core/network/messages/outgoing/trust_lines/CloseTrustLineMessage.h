#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H

#include "../../TrustLinesMessage.h"

#include "../../../../common/Types.h"

#include "../../../../common/NodeUUID.h"
#include "../../../../transactions/TransactionUUID.h"

#include <memory>
#include <utility>
#include <stdint.h>
#include <malloc.h>

class CloseTrustLineMessage: public TrustLinesMessage {

public:
    CloseTrustLineMessage(
        NodeUUID &sender,
        TransactionUUID &transactionUUID,
        NodeUUID &contractorUUID
    );

    const MessageTypeID typeID() const;

    pair<ConstBytesShared, size_t> serialize();

private:
    void deserialize(
        byte* buffer);

private:
    NodeUUID mContractorUUID;

};


#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINEMESSAGE_H
